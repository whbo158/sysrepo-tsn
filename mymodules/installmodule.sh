#!/bin/bash

sudo sysrepoctl -i --yang=ieee802-types.yang
sudo sysrepoctl -i --yang=ieee802-dot1q-types.yang
sudo sysrepoctl -i --yang=ietf-interfaces@2014-05-08.yang
sudo sysrepoctl -i --yang=ietf-inet-types@2013-07-15.yang
sudo sysrepoctl -i --yang=iana-if-type@2017-01-19.yang
sudo sysrepoctl -i --yang=ietf-ip@2014-06-16.yang
sudo sysrepoctl -i --feature-enable=ipv4-non-contiguous-netmasks --module=ietf-ip
sudo sysrepoctl -i --yang=ieee802-dot1q-bridge.yang
sudo sysrepoctl -i --yang=ieee802-dot1q-sched.yang
sudo sysrepoctl -i --feature-enable=scheduled-traffic --module=ieee802-dot1q-sched
sudo sysrepoctl -i --yang=nxp-bridge-vlan-tc-flower.yang
