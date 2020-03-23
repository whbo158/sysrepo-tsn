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
		memcpy(buf, &ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
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

int get_inet_mac(char *ifname, uint8 *buf)
{
	return get_inet_cfg(ifname, SIOCGIFHWADDR, buf, IFHWADDRLEN);
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
		memcpy(&ifr.ifr_ifru.ifru_hwaddr.sa_data, buf, IFHWADDRLEN);
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

int set_inet_mac(char *ifname, uint8 *buf)
{
	return set_inet_cfg(ifname, SIOCSIFHWADDR, buf, IFHWADDRLEN);
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

int test_inet_cfg(void)
{
	struct in_addr ip;
	uint8 mac[IFHWADDRLEN];

	get_inet_ip("vethmy0", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_ip("vethmy1", &ip);
	PRINT("ip:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy0", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	get_inet_mask("vethmy1", &ip);
	PRINT("mask:%s\n", inet_ntoa(ip));

	get_inet_mac("vethmy0", mac);
	PRINT("mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#if 0
	get_inet_mac("vethmy1", mac);
	PRINT("mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
	ip.s_addr = inet_addr("192.168.15.1");
	set_inet_ip("vethmy0", &ip);

	ip.s_addr = inet_addr("255.255.0.0");
	set_inet_mask("vethmy0", &ip);

	get_inet_mac("switch", mac);
	mac[5] += 1;
	set_inet_mac("switch", mac);

#ifdef TEST_ADD
	set_inet_updown("vethmy0", true);
#else
	set_inet_updown("vethmy0", false);
#endif

	return 0;
}
