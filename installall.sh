#!/bin/bash

cd /home/whb/work/netopeer

rm -fr libyang/build
mkdir libyang/build
cd libyang/build/
cmake ..
make
sudo make install
cd -

rm -fr sysrepo/build
mkdir sysrepo/build
cd sysrepo/build/
cmake ..
make
sudo make install
cd -

rm -fr libnetconf2/build
mkdir libnetconf2/build
cd libnetconf2/build/
cmake ..
make
sudo make install
cd -

rm -fr Netopeer2/keystored/build
mkdir Netopeer2/keystored/build
cd Netopeer2/server/build/
cmake ..
make
sudo make install
cd -

rm -fr Netopeer2/server/build
mkdir Netopeer2/server/build
cd Netopeer2/server/build/
cmake ..
make
sudo make install
cd -

rm -fr Netopeer2/cli/build
mkdir Netopeer2/cli/build
cd Netopeer2/cli/build/
cmake ..
make
sudo make install
cd -

