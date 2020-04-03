#include "brtc_cfg.h"

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

struct item_qdisc
{
	char action[MAX_PARA_LEN];
	char block[MAX_PARA_LEN];
	char ifname[IF_NAME_MAX_LEN];
};

struct item_filter
{
	uint8_t vid;
	uint8_t priority;
	struct in_addr src_ip;
	struct in_addr dst_ip;
	uint8_t src_port;
	uint8_t dst_port;

	char action[MAX_PARA_LEN];
	char protocol[MAX_PARA_LEN];
	char parent[MAX_PARA_LEN];
	char filter_type[MAX_PARA_LEN];
	char ifname[IF_NAME_MAX_LEN];
	char action_spec[MAX_ACTION_LEN];
};

struct item_cfg
{
	bool valid;
	bool vidflag;
	uint32_t vid;
	char ifname[IF_NAME_MAX_LEN];
};
static struct item_cfg sitem_conf;

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
		goto out;

	PRINT("nodename:%s type:%d\n", nodename, value->type);

	strval = value->data.string_val;

	conf->vidflag = true;
	if (!strcmp(nodename, "vid")) {
		if (true) {
			conf->vid = value->data.uint32_val;
			conf->vidflag = true;
			printf("\nVALID vid= %d\n", conf->vid);
		}
	} else if (!strcmp(nodename, "protocol")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID protocol = %s\n", strval);
		}
	} else if (!strcmp(nodename, "parent")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID parent = %s\n", strval);
		}
	} else if (!strcmp(nodename, "filtertype")) {
		if (conf->vidflag) {
			//snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID filtertype = %d\n", value->data.uint32_val);
		}
#if 0
	} else if (!strcmp(nodename, "srcip")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID srcip = %s\n", strval);
		}
	} else if (!strcmp(nodename, "dstip")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID dstip = %s\n", strval);
		}
	} else if (!strcmp(nodename, "srcport")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID srcport = %s\n", strval);
		}
	} else if (!strcmp(nodename, "dstport")) {
		if (conf->vidflag) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", strval);
			conf->valid = true;
			printf("\nVALID dstport = %s\n", strval);
		}
#endif
	}

out:
	return rc;
}

static int config_per_item(sr_session_ctx_t *session, char *path,
			bool abort, struct item_cfg *conf)
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

static int sub_config(sr_session_ctx_t *session, const char *path, bool abort)
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
		rc = config_per_item(session, xpath, abort, conf);
		if (rc != SR_ERR_OK)
			break;
	}

	if (conf->valid) {
	//	set_inet_vlan(conf->ifname, conf->vid, true);
		PRINT("set_inet_vlan ifname:%s vid:%d\n", conf->ifname, conf->vid);
	}

	if (rc == SR_ERR_NOT_FOUND)
		rc = SR_ERR_OK;

cleanup:
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
		if (rc)
			goto out;
		rc = sub_config(session, xpath, false);
		break;
	case SR_EV_ENABLED:
		rc = sub_config(session, xpath, false);
		break;
	case SR_EV_DONE:
		break;
	case SR_EV_ABORT:
		rc = sub_config(session, xpath, true);
		break;
	default:
		break;
	}
out:
	return rc;
}
