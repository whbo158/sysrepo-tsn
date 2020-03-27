#include "header.h" 



/**
	author: hongbo.wang (hongbo.wang@nxp.com)
*/


int main(int argc, char **argv)
{
	char buf[3];

	snprintf(buf, 3, "%s", "b");
	printf("buf:%s\n", buf);
	return 0;
	test_inet_cfg();

	test_vlan_cfg();

	test_br_cfg();

	return 0;
}
