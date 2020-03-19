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


int set_ip_addr(char *ip)
{
	printf("%s:%d ip:%s\n", __func__, __LINE__, ip);
	return 0;
}

