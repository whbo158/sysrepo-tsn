#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

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

#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
#include "sysrepo.h"

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

#define PRINT printf("%s-%d: ", __func__, __LINE__);printf
#define ADDR_LEN (sizeof(struct in_addr))

//#define TEST_ADD (1)

#define XPATH_MAX_LEN		300
#define IF_NAME_MAX_LEN		20
#define NODE_NAME_MAX_LEN	80
#define MSG_MAX_LEN		100

#define IF_XPATH "/ietf-interfaces:interfaces/interface"
#define BRIDGE_XPATH "/ieee802-dot1q-bridge:bridges/bridge"
#define BRIDGE_COMPONENT_XPATH (BRIDGE_XPATH "/component")

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

bool is_del_oper(sr_session_ctx_t *session, char *path);

#endif
