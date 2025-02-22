#!/bin/bash

set -e

source /opt/ros/foxy/setup.bash

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

# note that environment variables aren't always picked up
# by colcon -- perhaps they're overridden? this necessitates
# duplicating some of them in --cmake-args 
cd transport_drivers \
    && colcon build \
        --cmake-args \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -Wno-dev \
        --symlink-install \
        --parallel-workers 4 \
    && cd ..
source transport_drivers/install/setup.bash

colcon build \
    --cmake-args \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -Wno-dev \
    --base-paths src \
    --symlink-install \
    --parallel-workers 4 \
    # for verbosity add --event-handlers console_direct+
