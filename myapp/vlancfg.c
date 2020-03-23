#include "header.h" 

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

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
//	ifr.u.name_type = VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD;

	if (addflag) {
		ifr.cmd = ADD_VLAN_CMD;
		snprintf(ifr.device1, sizeof(ifr.device1) - 1, "%s", ifname);
	} else {
		ifr.cmd = DEL_VLAN_CMD;
		snprintf(ifr.device1, sizeof(ifr.device1) - 1, "%s.%d", ifname, vid);
	}

	ret = ioctl(sockfd, SIOCSIFVLAN, &ifr);
	close(sockfd);
	if (ret < 0) {
		PRINT("ioctl error! ret:%d, need root account!\n", ret);
		PRINT("Note: this operation needs root permission!\n");
		return -4;
	}

	return 0;
}

int test_vlan_cfg(void)
{
//	set_inet_vlan("vethmy0", 101, true);
	set_inet_vlan("vethmy0", 100, false);

	return 0;
}
