#!/bin/bash

cd /home/whb/netopeer

cd Netopeer2/server/build/
sudo make uninstall
rm -fr *
cd -

cd Netopeer2/cli/build/
sudo make uninstall
rm -fr *
cd -

cd libnetconf2/build/
sudo make uninstall
rm -fr *
cd -

cd sysrepo/build/
sudo make uninstall
rm -fr *
cd -

cd libyang/build/
sudo make uninstall
rm -fr *
cd -
