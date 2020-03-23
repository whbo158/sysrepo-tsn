#include "header.h" 

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

static int set_inet_brctl(char *brname, bool addflag)
{
	int ret = 0;
	int sockfd = 0;
	struct ifreq ifr;

	if (!brname)
		return -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		PRINT("create socket failed! ret:%d\n", sockfd);
		return -2;
	}

	memset(&ifr, 0, sizeof(ifr));
	//prep_ifreq(&ifr, bridge);
	//ifr.ifr_ifindex = if_nametoindex(iface);
	if (addflag)
		ret = ioctl(sockfd, SIOCBRADDBR, brname);
	else	
		ret = ioctl(sockfd, SIOCBRDELBR, brname);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -4;
	}

	return 0;
}

static int set_inet_brifctl(char *brname, char *ifname, bool addflag)
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
	//prep_ifreq(&ifr, bridge);
	//ifr.ifr_ifindex = if_nametoindex(iface);
	ret = ioctl(sockfd, SIOCBRADDIF, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -4;
	}

	return 0;
}

int test_br_cfg(void)
{
	set_inet_brctl("wmybr0", true);
	//set_inet_brctl("wmybr0", false);

	return 0;
}
