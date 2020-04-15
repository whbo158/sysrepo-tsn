#!/bin/bash

sudo sysrepoctl -u nxp-bridge-vlan-tc-flower.yang
sudo sysrepoctl -u ieee802-dot1q-sched.yang
sudo sysrepoctl -u ieee802-types.yang
sudo sysrepoctl -u ieee802-dot1q-types.yang
sudo sysrepoctl -u ietf-interfaces@2014-05-08.yang
sudo sysrepoctl -u ietf-inet-types@2013-07-15.yang
sudo sysrepoctl -u iana-if-type@2017-01-19.yang
sudo sysrepoctl -u ietf-ip@2014-06-16.yang -e ipv4-non-contiguous-netmasks
sudo sysrepoctl -u ieee802-dot1q-bridge.yang
