#!/bin/bash

sysrepoctl -u nxp-bridge-vlan-tc-flower.yang
sysrepoctl -u ieee802-dot1q-sched.yang
sysrepoctl -u ieee802-types.yang
sysrepoctl -u ieee802-dot1q-types.yang
sysrepoctl -u ietf-interfaces@2014-05-08.yang
sysrepoctl -u ietf-inet-types@2013-07-15.yang
sysrepoctl -u iana-if-type@2017-01-19.yang
sysrepoctl -u ietf-ip@2014-06-16.yang -e ipv4-non-contiguous-netmasks
sysrepoctl -u ieee802-dot1q-bridge.yang
