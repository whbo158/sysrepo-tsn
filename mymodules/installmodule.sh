#!/bin/bash

sysrepoctl -i ieee802-types.yang
sysrepoctl -i ieee802-dot1q-types.yang
sysrepoctl -i ietf-interfaces@2014-05-08.yang
sysrepoctl -i ietf-inet-types@2013-07-15.yang
sysrepoctl -i iana-if-type@2017-01-19.yang
sysrepoctl -i ietf-ip@2014-06-16.yang -e ipv4-non-contiguous-netmasks
sysrepoctl -i ieee802-dot1q-bridge.yang
sysrepoctl -i ieee802-dot1q-sched.yang -e scheduled-traffic
sysrepoctl -i nxp-bridge-vlan-tc-flower.yang
