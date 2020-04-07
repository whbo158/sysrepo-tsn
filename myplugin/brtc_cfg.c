#include "brtc_cfg.h"

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

struct item_qdisc
{
	char action[MAX_PARA_LEN];
	char block[MAX_PARA_LEN];
	char ifname[MAX_PARA_LEN];
};

struct item_filter
{
	uint16_t vlanid;
	uint16_t priority;
	uint16_t src_port;
	uint16_t dst_port;

	char src_ip[MAX_PARA_LEN];
	char dst_ip[MAX_PARA_LEN];
	char action[MAX_PARA_LEN];
	char protocol[MAX_PARA_LEN];
	char parent[MAX_PARA_LEN];
	char type[MAX_PARA_LEN];
	char ifname[MAX_PARA_LEN];
	char action_spec[MAX_ACTION_LEN];
};

struct item_cfg
{
	bool valid;
	uint32_t vid;
	uint8_t sub_flag;
	struct item_qdisc qdisc;
	struct item_filter filter;
};
static struct item_cfg sitem_conf;

static char stc_cmd[MAX_CMD_LEN];
static char stc_subcmd[MAX_CMD_LEN];

static int set_inet_vlan(char *ifname, int vid, bool addflag)
{
	int ret = 0;
	int sockfd = 0;
	struct vlan_ioctl_args ifr;

	if (!ifname)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.u.VID = vid;

	if (addflag) {
		ifr.cmd = ADD_VLAN_CMD;
		snprintf(ifr.device1, sizeof(ifr.device1), "%s", ifname);
	} else {
		ifr.cmd = DEL_VLAN_CMD;
		snprintf(ifr.device1, sizeof(ifr.device1), "%s.%d", ifname, vid);
	}

	ret = ioctl(sockfd, SIOCSIFVLAN, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -3;
	}

	return 0;
}

static int parse_node(sr_session_ctx_t *session, sr_val_t *value, struct item_cfg *conf)
{
	int rc = SR_ERR_OK;
	sr_xpath_ctx_t xp_ctx = {0};
	char *index = NULL;
	uint8_t u8_val = 0;
	uint32_t u32_val = 0;
	uint64_t u64_val = 0;
	char *strval = NULL;
	char *nodename = NULL;
	char err_msg[MSG_MAX_LEN] = {0};

	if (!session || !value || !conf)
		return rc;

	sr_xpath_recover(&xp_ctx);
	nodename = sr_xpath_node_name(value->xpath);
	if (!nodename)
		goto ret_tag;

	//PRINT("nodename:%s type:%d\n", nodename, value->type);

	strval = value->data.string_val;

	if (!strcmp(nodename, "vid")) {
		if (true) {
			conf->vid = value->data.uint32_val;
		}
	} else if (!strcmp(nodename, "qdisc")) {
		conf->sub_flag = SUB_ITEM_QDISC;
	} else if (!strcmp(nodename, "filter")) {
		conf->sub_flag = SUB_ITEM_FILTER;
	} else if (!strcmp(nodename, "action")) {
		if (conf->sub_flag == SUB_ITEM_QDISC) {
			snprintf(conf->qdisc.action, MAX_PARA_LEN, "%s", strval);
		} else if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.action, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "interface")) {
		if (conf->sub_flag == SUB_ITEM_QDISC) {
			snprintf(conf->qdisc.ifname, MAX_PARA_LEN, "%s", strval);
		} else if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.ifname, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "block")) {
		if (conf->sub_flag == SUB_ITEM_QDISC) {
			snprintf(conf->qdisc.block, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "protocol")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.protocol, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "parent")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.parent, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "filtertype")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.type, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "vlanid")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			conf->filter.vlanid = value->data.uint16_val;
		}
	} else if (!strcmp(nodename, "priority")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			conf->filter.priority = value->data.uint16_val;
		}
	} else if (!strcmp(nodename, "srcip")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.src_ip, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "dstip")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			snprintf(conf->filter.dst_ip, MAX_PARA_LEN, "%s", strval);
		}
	} else if (!strcmp(nodename, "srcport")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			conf->filter.src_port = value->data.uint16_val;
		}
	} else if (!strcmp(nodename, "dstport")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			conf->filter.dst_port = value->data.uint16_val;
		}
	} else if (!strcmp(nodename, "actionspec")) {
		if (conf->sub_flag == SUB_ITEM_FILTER) {
			conf->valid = true;
			snprintf(conf->filter.action_spec, MAX_ACTION_LEN, "%s", strval);
		}
	}

ret_tag:
	return rc;
}

static int config_per_item(sr_session_ctx_t *session, char *path,
			struct item_cfg *conf)
{
	size_t i;
	size_t count;
	int valid = 0;
	int rc = SR_ERR_OK;
	sr_val_t *values = NULL;
	char err_msg[MSG_MAX_LEN] = {0};

	rc = sr_get_items(session, path, 0, &values, &count);
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

	PRINT("CUR COUNT:%ld\n", count);
	for (i = 0; i < count; i++) {
		if (values[i].type == SR_LIST_T
		    || values[i].type == SR_CONTAINER_PRESENCE_T)
			continue;

		if (!parse_node(session, &values[i], conf))
			valid++;
	}

	if (!valid)
		goto cleanup;

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
	char ifname_bak[MAX_VLAN_LEN] = {0};
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

		ifname = sr_xpath_key_value(value->xpath, "bridge",
					    "name", &xp_ctx);
		PRINT("IFNAME:%s\n", ifname);

		sr_free_val(old_value);
		sr_free_val(new_value);

		if (!ifname)
			continue;

		if (!strcmp(ifname, ifname_bak))
			continue;
		snprintf(ifname_bak, MAX_VLAN_LEN, "%s", ifname);

		PRINT("SUBXPATH:%s ifname:%s len:%ld\n", xpath, ifname, strlen(ifname));
		rc = config_per_item(session, xpath, conf);
		if (rc != SR_ERR_OK)
			break;
	}

	if (rc == SR_ERR_NOT_FOUND)
		rc = SR_ERR_OK;

cleanup:
	return rc;
}

static int show_config(struct item_cfg *conf)
{
	if (!conf)
		return -1;

	printf("disc-action = %s\n", conf->qdisc.action);
	printf("qdisc-interface = %s\n", conf->qdisc.ifname);
	printf("disc-block = %s\n", conf->qdisc.block);
	printf("filter-action = %s\n", conf->filter.action);
	printf("filter-interface = %s\n", conf->filter.ifname);
	printf("filter-protocol = %s\n", conf->filter.protocol);
	printf("filter-parent = %s\n", conf->filter.parent);
	printf("filter-type = %s\n", conf->filter.type);
	printf("filter-vlanid = %d\n", conf->filter.vlanid);
	printf("filter-priority = %d\n", conf->filter.priority);
	printf("filter-src_ip = %s\n", conf->filter.src_ip);
	printf("filter-dst_ip = %s\n", conf->filter.dst_ip);
	printf("filter-src_port = %d\n", conf->filter.src_port);
	printf("filter-dst_port = %d\n", conf->filter.dst_port);
	printf("filter-action_spec = %s\n", conf->filter.action_spec);

	return 0;
}

static int set_config(sr_session_ctx_t *session, bool abort)
{
	int rc = SR_ERR_OK;
	struct item_cfg *conf = &sitem_conf;

	if (abort) {
		memset(conf, 0, sizeof(struct item_cfg));
		return rc;
	}

	if (!conf->valid)
		return rc;

	show_config(conf);

	if ((strlen(conf->qdisc.action) == 0) && (strlen(conf->qdisc.ifname) > 0))
		return rc;

	if ((strlen(conf->filter.action) == 0) || (strlen(conf->filter.ifname) == 0))
		return rc;

	snprintf(stc_cmd, MAX_CMD_LEN, "tc qdisc %s dev %s %s\n",
		conf->qdisc.action, conf->qdisc.ifname, conf->qdisc.block);
	printf("qdisc: %s\n", stc_cmd);

	snprintf(stc_cmd, MAX_CMD_LEN, "tc filter %s dev %s skip_sw ",
		conf->filter.action, conf->filter.ifname);

	if (strlen(conf->filter.protocol) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "protocol %s ", conf->filter.protocol);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (strlen(conf->filter.parent) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "parent %s ", conf->filter.parent);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (strlen(conf->filter.type) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "%s ", conf->filter.type);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (conf->filter.vlanid > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "vlan_id %d ", conf->filter.vlanid);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (conf->filter.priority > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "vlan_prio %d ", conf->filter.priority);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (strlen(conf->filter.src_ip) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "src_ip %s ", conf->filter.src_ip);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (strlen(conf->filter.dst_ip) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "dst_ip %s ", conf->filter.dst_ip);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (conf->filter.src_port > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "src_port %d ", conf->filter.src_port);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (conf->filter.dst_port > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "dst_port %d ", conf->filter.dst_port);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	if (strlen(conf->filter.action_spec) > 0) {
		snprintf(stc_subcmd, MAX_CMD_LEN, "action %s ", conf->filter.action_spec);
		strncat(stc_cmd, stc_subcmd, MAX_CMD_LEN - 1 - strlen(stc_cmd));
	}
	printf("filter: %s\n", stc_cmd);

	return rc;
}

int brtc_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
	const char *path, sr_event_t event, uint32_t id, void *private_ctx)
{
	int rc = SR_ERR_OK;
	char xpath[XPATH_MAX_LEN] = {0};

	PRINT("mod:%s path:%s event:%d\n", module_name, path, event);
	snprintf(xpath, XPATH_MAX_LEN, "%s", path);

	switch (event) {
	case SR_EV_CHANGE:
		rc = parse_config(session, xpath);
		break;
	case SR_EV_ENABLED:
		break;
	case SR_EV_DONE:
		rc = set_config(session, false);
		break;
	case SR_EV_ABORT:
		rc = set_config(session, true);
		break;
	default:
		break;
	}

	return rc;
}
