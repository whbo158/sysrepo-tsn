/**
 * @file cb.c
 * @author shang gao
 * @brief Application to configure CB generation and CB recovery related parameters based on sysrepo datastore.
 *
 * Copyright 2021 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cb.h"

#define ADDR_LEN (sizeof(struct in_addr))

static int decision = 0;

struct item_cfg {
	bool valid;
	char ifname[IF_NAME_MAX_LEN];
	int  his_len;
 	int  ind;
	char port[20];
	char genport[20];
	unsigned int  output_id_list[10];
};
static struct item_cfg sitem_conf;

static int parse_node(sr_session_ctx_t *session, sr_val_t *value,
			struct item_cfg *conf)
{
	int rc = SR_ERR_OK;
	sr_xpath_ctx_t xp_ctx = {0};
	char *nodename = NULL;
	char port_path[100];

	if (!session || !value || !conf)
		return rc;
	sr_xpath_recover(&xp_ctx);
	nodename = sr_xpath_node_name(value->xpath);
	if (!nodename)
		goto ret_tag;

	if (!strcmp(nodename, "history-length")){
		conf->his_len = value->data.uint8_val;
	}
	else if (!strcmp(nodename, "index")){
	        conf->ind = value->data.uint8_val;
	}
	else if (!strcmp(nodename, "port-list")) {
		strcpy(conf->port, value->data.string_val);
	}
	else if (!strcmp(nodename, "output-id-list")){
		conf->output_id_list[value->data.uint8_val] = value->data.uint8_val + 1;
	}
	else if (!strcmp(nodename, "port")){
		strncpy(port_path, value->xpath, 83);
		if (strcmp(port_path, PPATH) == 0){
			strcpy(conf->genport, value->data.string_val);
		}	
	}

ret_tag:
	return rc;
}

static int parse_item(sr_session_ctx_t *session, char *path,
			struct item_cfg *conf)
{
	size_t i;
	size_t count;
	int rc = SR_ERR_OK;
	sr_val_t *values = NULL;
	char err_msg[MSG_MAX_LEN] = {0};

	rc = sr_get_items(session, path, &values, &count);
	if (rc == SR_ERR_NOT_FOUND) {
		/*
		 * If can't find any item, we should check whether this
		 * container was deleted.
		 */
		if (is_del_oper(session, path)) {
			printf("WARN: %s was deleted, disable %s",
			       path, "this Instance.\n");
			goto cleanup;
		} else {
			printf("WARN: %s sr_get_items: %s\n", __func__,
			       sr_strerror(rc));
			return SR_ERR_OK;
		}
	} else if (rc != SR_ERR_OK) {
		snprintf(err_msg, MSG_MAX_LEN,
			 "Get items from %s failed", path);
		sr_set_error(session, err_msg, path);

		printf("ERROR: %s sr_get_items: %s\n", __func__,
		       sr_strerror(rc));
		return rc;
	}

	for (i = 0; i < count; i++) {
		if (values[i].type == SR_LIST_T
		    || values[i].type == SR_CONTAINER_PRESENCE_T)
			continue;
		rc = parse_node(session, &values[i], conf);
	}
	if (conf->ind && conf->his_len && conf->port){
		decision = 1;
	}
	else if (conf->ind && !conf->his_len && conf->genport && conf->output_id_list){
		decision = 2;
        }
	else {
		decision = 0;
		printf("ERROR : invalid file\n");
	}

cleanup:
	sr_free_values(values, count);
	return rc;
}

static int parse_config(sr_session_ctx_t *session, const char *path)
{
	int rc = SR_ERR_OK;
	sr_change_oper_t oper;
	char *ifname = NULL;
	sr_val_t *value = NULL;
	sr_val_t *old_value = NULL;
	sr_val_t *new_value = NULL;
	sr_change_iter_t *it = NULL;
	sr_xpath_ctx_t xp_ctx = {0};
	char xpath[XPATH_MAX_LEN] = {0};
	char err_msg[MSG_MAX_LEN] = {0};
	char ifname_bak[IF_NAME_MAX_LEN] = {0};
	struct item_cfg *conf = &sitem_conf;

	memset(conf, 0, sizeof(struct item_cfg));
	snprintf(xpath, XPATH_MAX_LEN, "%s//*", path);
	rc = sr_get_changes_iter(session, xpath, &it);

	if (rc != SR_ERR_OK) {
		snprintf(err_msg, MSG_MAX_LEN,
			 "Get changes from %s failed", xpath);
		sr_set_error(session, err_msg, xpath);

		printf("ERROR: %s sr_get_changes_iter: %s\n", __func__,
		       sr_strerror(rc));
		goto cleanup;
	}
	while (SR_ERR_OK == (rc = sr_get_change_next(session, it,
					&oper, &old_value, &new_value))) {
		value = new_value ? new_value : old_value;
		if (!value)
			continue;
                rc = parse_item(session, xpath, conf);
		ifname = sr_xpath_key_value(value->xpath, "interface",
					    "name", &xp_ctx);
		sr_free_val(old_value);
		sr_free_val(new_value);

		if (!ifname)
			continue;

		if (!strcmp(ifname, ifname_bak))
			continue;
		snprintf(ifname_bak, IF_NAME_MAX_LEN, "%s", ifname);

		snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", ifname);
					
		rc = parse_item(session, xpath, conf);
		if (rc != SR_ERR_OK)
			break;
	}

cleanup:
	if (conf->valid)
		if (!conf->ifname || (strlen(conf->ifname) == 0))
			return SR_ERR_INVAL_ARG;
	if (rc == SR_ERR_NOT_FOUND)
		rc = SR_ERR_OK;
	return rc;
}

void cbrec_execute(void)
{
	struct tsn_seq_rec_conf cbrecy;
	struct tsn_seq_rec_conf *recy = &cbrecy;
	struct item_cfg *con = &sitem_conf;

	memset(&cbrecy, 0, sizeof(cbrecy));
	cbrecy.seq_len = 16;
	cbrecy.his_len = con->his_len;
	cbrecy.rtag_pop_en = 1;
	init_tsn_socket();
	tsn_cbrec_set(con->port, con->ind, recy);
	close_tsn_socket();
}

void cbgen_execute(void)
{
	struct tsn_seq_gen_conf cbgenr;
	struct tsn_seq_gen_conf *genr = &cbgenr;
	struct item_cfg *co = &sitem_conf;
	int iport_mask = 0;
	int split_mask = 0;

	memset(&cbgenr, 0, sizeof(cbgenr));
	for (int index = 0; index < 10; index++) {
		if(co->output_id_list[index] != 0){
			co->output_id_list[index] = co->output_id_list[index] - 1;
			co->output_id_list[index] = 0x01<<co->output_id_list[index];
			}
		}
	for (int num = 0; num < 10; num ++){
		split_mask = split_mask + co->output_id_list[num];
	}
	iport_mask = 0x01 << (co->genport[3] - '0');
	cbgenr.seq_len = 16;
	cbgenr.seq_num = 2048;
	cbgenr.iport_mask = iport_mask;
	cbgenr.split_mask = split_mask;
	init_tsn_socket();
	tsn_cbgen_set(co->genport, co->ind, genr);
	close_tsn_socket();
}


int cb_subtree_change_cb(sr_session_ctx_t *session, const char *path,
	sr_notif_event_t event, void *private_ctx)
{
	int rc = SR_ERR_OK;
	char xpath[XPATH_MAX_LEN] = {0};
	snprintf(xpath, XPATH_MAX_LEN, "%s", path);

	switch (event) {
	case SR_EV_VERIFY:
		rc = parse_config(session, xpath);
		break;
	case SR_EV_ENABLED:
		rc = parse_config(session, xpath);
		break;
	case SR_EV_APPLY:
		if (decision == 1){
			cbrec_execute();	
		}
		else if (decision == 2){
			cbgen_execute();
		}
		break;
	case SR_EV_ABORT:
		break;
	default:
		break;
	}

	return rc;
}
