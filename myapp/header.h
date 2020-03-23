#ifndef __IP_CFG_HEADER__
#define __IP_CFG_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_vlan.h>
#include <linux/sockios.h>

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

#define PRINT printf("%s-%d: ", __func__, __LINE__);printf
#define ADDR_LEN (sizeof(struct in_addr))

//#define TEST_ADD (1)

/*
 sudo ip link add name vethmy0 type veth peer name vethmy1

 * add:
 sudo vconfig add vethmy0 100
 sudo brctl addbr vmybr0
 sudo brctl addif vmybr0 vethmy0.100

 * del:
 sudo brctl delif vmybr0 vethmy0.100
 sudo brctl delbr vmybr0
 sudo vconfig rem vethmy0.100
 */

typedef unsigned char uint8;

int set_inet_vlan(char *ifname, int vid, bool addflag);
int set_inet_brctl(char *brname, bool addflag);
int set_inet_brifctl(char *brname, char *ifname, bool addflag);

int get_inet_ip(char *ifname, struct in_addr *ip);
int get_inet_mask(char *ifname, struct in_addr *mask);
int get_inet_mac(char *ifname, uint8 *buf);

int set_inet_ip(char *ifname, struct in_addr *ip);
int set_inet_mask(char *ifname, struct in_addr *mask);
int set_inet_mac(char *ifname, uint8 *buf);

int test_inet_cfg(void);
int test_vlan_cfg(void);
int test_br_cfg(void);

#endif
