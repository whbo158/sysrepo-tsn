CC_PATH = /opt/fsl-qoriq/2.0/sysroots/x86_64-fslsdk-linux/usr/bin/aarch64-fsl-linux/
MK_PATH = /opt/fsl-qoriq/2.0/sysroots/x86_64-fslsdk-linux/usr/bin/

all:
	gcc main.c common.c ip_cfg.c vlan_cfg.c mac_cfg.c -o myplugin -lpthread -lsysrepo -lyang

clean:
	rm -fr *.o
	rm myplugin
