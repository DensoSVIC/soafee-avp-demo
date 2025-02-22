#!/bin/bash

# exit on error
set -e

export AUTOWARE_HOME=/opt/autoware/
if [ ! -d "${AUTOWARE_HOME}" ]; then
    echo ERROR: AUTOWARE_HOME path ${AUTOWARE_HOME} not found
    exit 1
fi

# Autoware & ROS environment
export AUTOWARE_HOME=/opt/autoware
export CYCLONEDDS_URI=file:///lingua-franca/cyclonedds.xml
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# LF AVP environment
export LF_TELEGRAF_HOST_NAME=${LF_TELEGRAF_HOST_NAME:-host.docker.internal}
export LF_TELEGRAF_PORT=${LF_TELEGRAF_PORT:-8094}
export LF_TELEGRAF_URL_PATH=${LF_TELEGRAF_URL_PATH:-/telegraf}
export LFAVP_CONFIG=/lingua-franca/config

# link included files to expected filesystem locations
rm -rf /Autoware-support
ln -sf /lingua-franca/Autoware-support /Autoware-support

# source autoware and ROS environment
if [ "$(ps -p $$ -o comm=)" == "bash" ]; then
    . /opt/ros/foxy/setup.bash
    . /opt/autoware/install/setup.bash
fi

# build optimizations
export CC="ccache gcc"
export CXX="ccache g++"
export CMAKE_CXX_COMPILER_LAUNCHER=ccache
export CMAKE_C_COMPILER_LAUNCHER=ccache
export MAKEFLAGS="${MAKEFLAGS} -j4"
export CMAKE_BUILD_PARALLEL_LEVEL=4

# suppress warnings
export CMAKE_WARN_DEPRECATED=OFF
export CMAKE_SUPPRESS_DEVELOPER_WARNINGS=ON

# prefer NVIDIA GL drivers if available
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}
export LIBGL_DRIVERS_PATH=/usr/lib/x86_64-linux-gnu/dri:${LIBGL_DRIVERS_PATH}
