#include "ip_cfg.h" 

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

static int get_inet_cfg(char *ifname, int req, void *buf, int len)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr = {0};
	struct sockaddr_in *sin = NULL;

	if (!ifname || !buf)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name) - 1, "%s", ifname);

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

int get_inet_ip(char *ifname, struct in_addr *ip)
{
	return get_inet_cfg(ifname, SIOCGIFADDR, ip, ADDR_LEN);
}

int get_inet_mask(char *ifname, struct in_addr *mask)
{
	return get_inet_cfg(ifname, SIOCGIFNETMASK, mask, ADDR_LEN);
}

int get_inet_mac(char *ifname, uint8 *buf, int len)
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
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name) - 1, "%s", ifname);

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

int set_inet_ip(char *ifname, struct in_addr *ip)
{
	return set_inet_cfg(ifname, SIOCSIFADDR, ip, ADDR_LEN);
}

int set_inet_mask(char *ifname, struct in_addr *mask)
{
	return set_inet_cfg(ifname, SIOCSIFNETMASK, mask, ADDR_LEN);
}

int set_inet_mac(char *ifname, uint8 *buf, int len)
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
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name) - 1, "%s", ifname);

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

bool is_valid_addr(uint8 *ip)
{
	int ret = 0;
	struct in_addr ip_addr;

	if (!ip)
	      return false;

	ret = inet_aton(ip, &ip_addr);
	if (0 == ret)
		return false;

	return true;
}

int convert_mac_address(char *str, uint8 *pbuf, int buflen)
{
	int i = 0;
	int ret = 0;
	int len = 0;
	int cnt = 0;
	uint32 data = 0;
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
#if 1
	for (i = 0; i < cnt; i++) {
		printf("%d:%X\n", i, pbuf[i]);
	}
#endif
	return 0;
}

int parse_node(sr_session_ctx_t *session, sr_val_t *value, struct inet_cfg *conf)
{
	int rc = SR_ERR_OK;
	sr_xpath_ctx_t xp_ctx = {0};
	char *index = NULL;
	uint8_t u8_val = 0;
	uint32_t u32_val = 0;
	uint64_t u64_val = 0;
	char *nodename = NULL;
	char err_msg[MSG_MAX_LEN] = {0};
	char *strval = NULL;

	if (!session || !value || !conf)
		return rc;

	strval = value->data.string_val;

	sr_xpath_recover(&xp_ctx);
	nodename = sr_xpath_node_name(value->xpath);
	if (!nodename)
		goto out;
printf("WHB nodename:%s type:%d\n", nodename, value->type);

	if (!strcmp(nodename, "ip")) {
		if (is_valid_addr(strval)) {
			conf->ip.s_addr = inet_addr(strval);
			printf("\nVALID ip= %s\n", strval);
		}
	} else if (!strcmp(nodename, "netmask")) {
		if (is_valid_addr(strval)) {
			conf->mask.s_addr = inet_addr(strval);
			printf("\nVALID netmask = %s\n", strval);
		}
	}

out:
	return rc;
}

static int config_per_node(sr_session_ctx_t *session, char *path,
			bool abort, char *ifname)
{
	int rc = SR_ERR_OK;
	sr_val_t *values;
	size_t count;
	size_t i;
	int valid = 0;
	char err_msg[MSG_MAX_LEN] = {0};
	struct inet_cfg *conf = &sinet_conf;

printf("IFNAME00:%s--len:%d\n", ifname, strlen(ifname));
	memset(conf, 0, sizeof(struct inet_cfg));
	snprintf(conf->ifname, IF_NAME_MAX_LEN, "%s", ifname);
printf("IFNAME11:%s--%s len:%d\n", ifname, conf->ifname, strlen(ifname));

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

printf("CUR COUNT:%d\n", count);
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
	sr_xpath_ctx_t xp_ctx = {0};
	sr_change_iter_t *it;
	sr_val_t *old_value;
	sr_val_t *new_value;
	sr_val_t *value;
	sr_change_oper_t oper;
	char *ifname;
	char ifname_bak[IF_NAME_MAX_LEN] = {0,};
	char xpath[XPATH_MAX_LEN] = {0};
	char err_msg[MSG_MAX_LEN] = {0};
	struct inet_cfg *conf = &sinet_conf;

	memset(conf, 0, sizeof(struct inet_cfg));

	snprintf(xpath, XPATH_MAX_LEN, "%s//*", path);
	rc = sr_get_changes_iter(session, xpath, &it);
	if (rc != SR_ERR_OK) {
		snprintf(err_msg, MSG_MAX_LEN,
			 "Get changes from %s failed", path);
		sr_set_error(session, err_msg, path);

		printf("ERROR: %s sr_get_changes_iter: %s\n", __func__,
		       sr_strerror(rc));
		goto cleanup;
	}
printf("XPATH:%s\n", xpath);
	while (SR_ERR_OK == (rc = sr_get_change_next(session, it,
					&oper, &old_value, &new_value))) {

		//print_change(oper, old_value, new_value);

		value = new_value ? new_value : old_value;
		ifname = sr_xpath_key_value(value->xpath, "interface",
					    "name", &xp_ctx);
printf("IFNAME:%s\n", ifname);

		sr_free_val(old_value);
		sr_free_val(new_value);

		if (!ifname)
			continue;

		if (strcmp(ifname, ifname_bak)) {
			snprintf(ifname_bak, IF_NAME_MAX_LEN, "%s", ifname);
			snprintf(xpath, XPATH_MAX_LEN, "%s[name='%s']/%s:*//*",
					IF_XPATH, ifname, IP_MODULE_NAME);

			printf("SUBXPATH:%s ifname:%s len:%d\n", xpath, ifname, strlen(ifname));
			rc = config_per_node(session, xpath, abort, ifname);
			if (rc != SR_ERR_OK)
				break;
		}
	}
	if (rc == SR_ERR_NOT_FOUND)
		rc = SR_ERR_OK;

	if (conf->ip.s_addr) {
		set_inet_ip(conf->ifname, &conf->ip);
	}

	if (conf->mask.s_addr) {
		set_inet_mask(conf->ifname, &conf->mask);
	}

cleanup:
	return rc;
}

int ip_subtree_change_cb(sr_session_ctx_t *session, const char *module_name,
		const char *path, sr_event_t event, void *private_ctx)
{
	int rc = SR_ERR_OK;
	char xpath[XPATH_MAX_LEN] = {0};

	printf("INET mod:%s path:%s event:%d\n", module_name, path, event);

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

int test_inet_cfg(void)
{
	struct in_addr ip;
	uint8 mac[IFHWADDRLEN];

	convert_mac_address("d6-ad-62-c1-60-f7", mac, sizeof(mac));
	convert_mac_address("d6-AD-62-C1-60-f7", mac, sizeof(mac));

	get_inet_ip("vethmy0", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_ip("vethmy1", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy0", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy1", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	get_inet_mac("vethmy0", mac, sizeof(mac));
	PRINT("mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#if 0
	get_inet_mac("vethmy1", mac, sizeof(mac));
	PRINT("mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
	ip.s_addr = inet_addr("192.168.15.1");
	set_inet_ip("vethmy0", &ip);

	ip.s_addr = inet_addr("255.255.0.0");
	set_inet_mask("vethmy0", &ip);

	get_inet_mac("switch", mac, sizeof(mac));
	mac[5] += 1;
	set_inet_mac("switch", mac, sizeof(mac));

#ifdef TEST_ADD
	set_inet_updown("vethmy0", true);
#else
	set_inet_updown("vethmy0", false);
#endif

	return 0;
}
