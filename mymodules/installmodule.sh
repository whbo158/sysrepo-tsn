#!/bin/bash

sudo sysrepoctl -i ieee802-dot1q-types.yang
sudo sysrepoctl -i ietf-interfaces@2014-05-08.yang
sudo sysrepoctl -i ietf-inet-types@2013-07-15.yang
sudo sysrepoctl -i iana-if-type@2017-01-19.yang
sudo sysrepoctl -i ietf-ip@2014-06-16.yang -e ipv4-non-contiguous-netmasks
sudo sysrepoctl -i ieee802-dot1q-bridge.yang
sudo sysrepoctl -i nxp-bridge-vlan-tc-flower.yang
