#include "header.h" 

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

int set_inet_brctl(char *brname, bool addflag)
{
	int ret = 0;
	int sockfd = 0;

	if (!brname)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	if (addflag)
		ret = ioctl(sockfd, SIOCBRADDBR, brname);
	else	
		ret = ioctl(sockfd, SIOCBRDELBR, brname);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -3;
	}

	return 0;
}

int set_inet_brifctl(char *brname, char *ifname, bool addflag)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr;

	if (!ifname)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name) - 1, "%s", brname);
	ifr.ifr_ifindex = if_nametoindex(ifname);

	if (addflag)
		ret = ioctl(sockfd, SIOCBRADDIF, &ifr);
	else
		ret = ioctl(sockfd, SIOCBRDELIF, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -3;
	}

	return 0;
}

int test_br_cfg(void)
{
#ifdef TEST_ADD
	set_inet_vlan("vethmy0", 100, true);
	set_inet_brctl("vmybr0", true);
	set_inet_brifctl("vmybr0", "vethmy0.100", true);
#else
	set_inet_brifctl("vmybr0", "vethmy0.100", false);
	set_inet_brctl("vmybr0", false);
	set_inet_vlan("vethmy0", 100, false);
#endif
	return 0;
}
