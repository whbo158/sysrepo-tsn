#!/bin/bash

cd /home/whb/netopeer

cd Netopeer2/server/build/
make uninstall
rm -fr *
cd -

cd Netopeer2/cli/build/
make uninstall
rm -fr *
cd -

cd libnetconf2/build/
make uninstall
rm -fr *
cd -

cd sysrepo/build/
make uninstall
rm -fr *
cd -

cd libyang/build/
make uninstall
rm -fr *
cd -
