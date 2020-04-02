CC = /usr/bin/aarch64-linux-gnu-
MK_PATH = /opt/fsl-qoriq/2.0/sysroots/x86_64-fslsdk-linux/usr/bin/

all:
	${CC}gcc main.c ipcfg.c vlancfg.c brcfg.c -o myapp -lpthread

clean:
	rm -fr *.o
	rm myapp 
