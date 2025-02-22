#!/bin/bash

export CYCLONEDDS_URI=file:///app/cyclonedds.xml
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

source /opt/ros/foxy/setup.bash

APP_DIR=/app/web

echo -e "Launching WebSocket server...\n"

fifo=$(mktemp -u)
mkfifo $fifo

# remove FIFOs and kill all descendents (based on PGID) on exit
trap 'rm -f $fifo; kill -- -$$' EXIT

# start the rosbridge server (with websocket)
ros2 run rosbridge_server rosbridge_websocket > $fifo 2>&1 &

# wait for the server to start
rx=".*server started on port 9090$"
launch_out=""
while [[ ! "$launch_out" =~ $rx ]]; do
	read -r launch_out < $fifo
	echo "$launch_out"
done

echo -e "\nReady"

# print all further websocket server output
while IFS= read -r line; do
	printf '%s\n' "$line"
done < $fifo &

echo -e "\nLaunching Webserver\n"

# launch python server
python3 -m http.server 8000 --directory "$APP_DIR"
