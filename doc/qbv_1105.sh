#!/bin/bash

set -e -u -o pipefail

NSEC_PER_SEC="1000000000"

gatemask() {
        local tc_list="$1"
        local mask=0

        for tc in ${tc_list}; do
                mask=$((${mask} | (1 << ${tc})))
        done

        printf "%02x" ${mask}
}

if ! systemctl is-active --quiet ptp4l; then
        echo "Please start the ptp4l service"
        exit
fi

now=$(phc_ctl /dev/ptp1 get | gawk '/clock time is/ { print $5; }')
# Phase-align the base time to the start of the next second.
sec=$(echo "${now}" | gawk -F. '{ print $1; }')
base_time="$(((${sec} + 1) * ${NSEC_PER_SEC}))"

tc qdisc add dev swp5 parent root handle 100 taprio \
        num_tc 8 \
        map 0 1 2 3 5 6 7 \
        queues 1@0 1@1 1@2 1@3 1@4 1@5 1@6 1@7 \
        base-time ${base_time} \
        sched-entry S $(gatemask 7) 100000 \
        sched-entry S $(gatemask "0 1 2 3 4 5 6") 400000 \
        flags 2
