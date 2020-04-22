#!/bin/bash

cd /home/whb/netopeer
mkdir ~/mynetopeer/ 

rm -fr libyang/build
mkdir libyang/build
cd libyang/build/
CC=/usr/bin/aarch64-linux-gnu-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/mynetopeer/ ..
make
make install
cd -

rm -fr sysrepo/build
mkdir sysrepo/build
cd sysrepo/build/
CC=/usr/bin/aarch64-linux-gnu-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/mynetopeer/ ..
make
make install
cd -

rm -fr libnetconf2/build
mkdir libnetconf2/build
cd libnetconf2/build/
CC=/usr/bin/aarch64-linux-gnu-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/mynetopeer/ ..
make
make install
cd -

rm -fr Netopeer2/server/build
mkdir Netopeer2/server/build
cd Netopeer2/server/build/
CC=/usr/bin/aarch64-linux-gnu-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/mynetopeer/ ..
make
make install
cd -

rm -fr Netopeer2/cli/build
mkdir Netopeer2/cli/build
cd Netopeer2/cli/build/
CC=/usr/bin/aarch64-linux-gnu-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/mynetopeer/ ..
make
make install
cd -

