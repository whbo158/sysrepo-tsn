CC_PATH = /opt/fsl-qoriq/2.0/sysroots/x86_64-fslsdk-linux/usr/bin/aarch64-fsl-linux/
MK_PATH = /opt/fsl-qoriq/2.0/sysroots/x86_64-fslsdk-linux/usr/bin/

all:
	gcc main.c ipcfg.c -o myapp -lpthread

clean:
	rm -fr *.o
	rm myapp 
