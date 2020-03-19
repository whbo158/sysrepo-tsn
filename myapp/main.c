#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>


/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/

int set_ip_addr(char *ip);

int main(int argc, char **argv)
{
	set_ip_addr("192.168.11.1");
	return 0;
}
