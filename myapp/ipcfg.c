#include "header.h" 

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

static int get_inet_cfg(char *ifname, int req, void *buf, int len)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr = {0};
	struct sockaddr_in *sin = NULL;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name) - 1, "%s", ifname);

	ret = ioctl(sockfd, req, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d\n", ret);
		return -2;
	}

	if (req == SIOCGIFHWADDR) {
		uint8 *mac = (uint8 *)buf;

		memcpy(mac, &ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
		PRINT("mac:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", 
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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

int set_ip_buf(char *ip)
{
	printf("%s:%d ip:%s\n", __func__, __LINE__, ip);
	return 0;
}

int test_inet_cfg(void)
{
	struct in_addr ip;

	get_inet_ip("vethmy0", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_ip("vethmy1", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy0", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy1", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	return 0;
}
