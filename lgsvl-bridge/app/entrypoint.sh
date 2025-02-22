#!/bin/bash

export CYCLONEDDS_URI=file:///app/cyclonedds.xml
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export LGSVL__AUTOPILOT_0_HOST=${LGSVL__AUTOPILOT_0_HOST:-$(ip route get 1 | sed -n 's/^.*src \([0-9.]*\) .*$/\1/p')}
export LGSVL__AUTOPILOT_0_PORT=${LGSVL__AUTOPILOT_0_PORT:-9091}

if [ ! -f "${CYCLONEDDS_URI#file://}" ]; then
    echo "CYCLONEDDS_URI not found: ${CYCLONEDDS_URI#file://}"
    exit 1
fi

source /opt/ros/foxy/setup.bash

lgsvl_bridge --port $LGSVL__AUTOPILOT_0_PORT &
sleep 2
python3 -u lgsvl-connecting-bridge.py
