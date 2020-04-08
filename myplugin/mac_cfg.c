#include "mac_cfg.h"

/**
 * @file mac_cfg.c
 * @author hongbo wang (hongbo.wang@nxp.com)
 * @brief Application to configure mac address function based on sysrepo datastore.
 *
 * Copyright 2019-2020 NXP
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

struct item_cfg {
	bool valid;
	char ifname[IF_NAME_MAX_LEN];
	char mac_addr[MAC_ADDR_MAX_LEN];
};
static struct item_cfg sitem_conf;

static int get_inet_cfg(char *ifname, int req, void *buf, int len)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr = {0};
	struct sockaddr_in *sin = NULL;

	if (!ifname || !buf)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

	ret = ioctl(sockfd, req, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d\n", ret);
		return -3;
	}

	if (req == SIOCGIFHWADDR) {
		memcpy(buf, &ifr.ifr_ifru.ifru_hwaddr.sa_data, len);
	} else {
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		memcpy((struct in_addr *)buf, &sin->sin_addr, len);
	}

	return 0;
}

int get_inet_mac(char *ifname, uint8_t *buf, int len)
{
	return get_inet_cfg(ifname, SIOCGIFHWADDR, buf, len);
}

static int set_inet_cfg(char *ifname, int req, void *buf, int len)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr = {0};
	struct sockaddr_in *sin = NULL;

	if (!ifname || !buf)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		PRINT("get interface flag failed! ret:%d\n", ret);
		return -3;
	}

	if (req == SIOCSIFHWADDR) {
		memcpy(&ifr.ifr_ifru.ifru_hwaddr.sa_data, buf, len);
		ifr.ifr_addr.sa_family = ARPHRD_ETHER;
	} else {
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		sin->sin_family = AF_INET;
		memcpy(&sin->sin_addr, (struct in_addr *)buf, len);
	}

	ret = ioctl(sockfd, req, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -4;
	}

	return 0;
}

int set_inet_mac(char *ifname, uint8_t *buf, int len)
{
	return set_inet_cfg(ifname, SIOCSIFHWADDR, buf, len);
}

static int set_inet_updown(char *ifname, bool upflag)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr = {0};
	struct sockaddr_in *sin = NULL;

	if (!ifname)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		PRINT("get interface flag failed! ret:%d\n", ret);
		return -3;
	}

	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	sin->sin_family = AF_INET;

	if (upflag)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -4;
	}

	return 0;
}

int convert_mac_address(char *str, uint8_t *pbuf, int buflen)
{
	int i = 0;
	int ret = 0;
	int len = 0;
	int cnt = 0;
	uint32_t data = 0;
	char *pmac = NULL;
	char buf[32] = {0};

	if (!str || !pbuf)
		return -1;

	snprintf(buf, sizeof(buf), "%s", str);
	pmac = buf;

	len = strlen(buf);
	for (i = 0; i < (len + 1); i++) {
		if ((buf[i] == '-') || (buf[i] == ':') || (buf[i] == '\0')) {
			buf[i] = '\0';
			ret = sscanf(pmac, "%02X", &data);
			if (ret != 1)
			      return -2;

			if (cnt < buflen)
			      pbuf[cnt++] = data & 0xFF;

			pmac = buf + i + 1;
		}
	}
#if 0
	for (i = 0; i < cnt; i++) {
		printf("%d:%X\n", i, pbuf[i]);
	}
#endif
	return 0;
}

static int parse_node(sr_session_ctx_t *session, sr_val_t *value,
			struct item_cfg *conf)
{
	int rc = SR_ERR_OK;
	sr_xpath_ctx_t xp_ctx = {0};
	char *index = NULL;
	uint8_t u8_val = 0;
	uint32_t u32_val = 0;
	uint64_t u64_val = 0;
	char *nodename = NULL;
	char err_msg[MSG_MAX_LEN] = {0};

	if (!session || !value || !conf)
		return rc;

	sr_xpath_recover(&xp_ctx);
	nodename = sr_xpath_node_name(value->xpath);
	if (!nodename)
		goto ret_tag;

	PRINT("nodename:%s type:%d\n", nodename, value->type);

	if (!strcmp(nodename, "address")) {
		if (!conf->valid) {
			snprintf(conf->mac_addr, MAC_ADDR_MAX_LEN, "%s",
					value->data.string_val);
			conf->valid = true;
			printf("\nVALID mac_addr = %s\n", conf->mac_addr);
		}
	} else if (!strcmp(nodename, "name")) {
		if (!conf->valid) {
			snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", value->data.string_val);
			printf("\nVALID ifname = %s\n", conf->ifname);
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
	char ifname_bak[IF_NAME_MAX_LEN] = {0};
	struct item_cfg *conf = &sitem_conf;

	memset(conf, 0, sizeof(struct item_cfg));

	snprintf(xpath, XPATH_MAX_LEN, "%s//*", BRIDGE_XPATH);

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
		PRINT("BRNAME:%s xpath:%s new:%p old:%p\n", ifname,
				value->xpath, new_value, old_value);

		sr_free_val(old_value);
		sr_free_val(new_value);

		if (!ifname)
			continue;

		if (!strcmp(ifname, ifname_bak))
			continue;
		snprintf(ifname_bak, IF_NAME_MAX_LEN, "%s", ifname);

		snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", ifname);

		PRINT("SUBXPATH:%s ifname:%s len:%ld\n", xpath, ifname, strlen(ifname));
		rc = config_per_item(session, xpath, conf);
		if (rc != SR_ERR_OK)
			break;
	}

cleanup:
	if (rc == SR_ERR_NOT_FOUND)
		rc = SR_ERR_OK;

	return rc;
}

static int set_config(sr_session_ctx_t *session, bool abort)
{
	int i = 0;
	int rc = SR_ERR_OK;
	char *ifname = NULL;
	uint8_t mac[IFHWADDRLEN];
	struct sub_item_cfg *ipv4 = NULL;
	struct item_cfg *conf = &sitem_conf;

	if (abort) {
		memset(conf, 0, sizeof(struct item_cfg));
		return rc;
	}

	if (!conf->valid)
		return rc;

	/* config mac address */
	convert_mac_address(conf->mac_addr, mac, sizeof(mac));
	set_inet_mac(conf->ifname, mac, sizeof(mac));
	printf("set_inet_mac ifname:%s mac:%s\n", conf->ifname, conf->mac_addr);

	return rc;
}

int mac_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
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
