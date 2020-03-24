#!/bin/bash

cd /home/whb/netopeer

cd libyang/build/
cmake ..
make
sudo make install
cd -

cd sysrepo/build/
cmake ..
make
sudo make install
cd -

cd libnetconf2/build/
cmake ..
make
sudo make install
cd -

cd Netopeer2/server/build/
cmake ..
make
sudo make install
cd -

cd Netopeer2/cli/build/
cmake ..
make
sudo make install
cd -

