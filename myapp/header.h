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

/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

#define PRINT printf
#define ADDR_LEN (sizeof(struct in_addr))

typedef unsigned char uint8;

int test_inet_cfg(void);


#endif
