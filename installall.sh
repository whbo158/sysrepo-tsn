#!/bin/bash

cd /home/whb/netopeer

mkdir libyang/build
cd libyang/build/
cmake ..
make
make install
cd -

mkdir sysrepo/build
cd sysrepo/build/
cmake ..
make
make install
cd -

mkdir libnetconf2/build
cd libnetconf2/build/
cmake ..
make
make install
cd -

mkdir Netopeer2/server/build
cd Netopeer2/server/build/
cmake ..
make
make install
cd -

mkdir Netopeer2/cli/build
cd Netopeer2/cli/build/
cmake ..
make
make install
cd -

