# Copyright 2020 The Autoware Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 3.5)

project(lane_planner_nodes)

# dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()


set(LANE_PLANNER_NODES_NODE_SRC
  src/lane_planner_node.cpp
)

set(LANE_PLANNER_NODES_NODE_HEADERS
  include/lane_planner_nodes/lane_planner_node.hpp
)

# generate component node library
ament_auto_add_library(lane_planner_node SHARED
  ${LANE_PLANNER_NODES_NODE_SRC}
  ${LANE_PLANNER_NODES_NODE_HEADERS}
)

autoware_set_compile_options(lane_planner_node)
rclcpp_components_register_node(lane_planner_node
  PLUGIN "autoware::lane_planner_nodes::LanePlannerNode"
  EXECUTABLE lane_planner_node_exe
)

# required for tf2
target_compile_options(lane_planner_node PRIVATE -Wno-sign-conversion -Wno-conversion -Wno-old-style-cast
        -Wno-useless-cast -Wno-double-promotion -Wno-nonnull-compare -Wuseless-cast)
target_include_directories(lane_planner_node SYSTEM PUBLIC ${tf2_INCLUDE_DIRS})

# Testing
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  add_ros_test(
    test/lane_planner_node_launch.test.py
    TIMEOUT "30"
  )

  # Unit tests
  set(TEST_SOURCES test/test_lane_planner_node.cpp)
  set(TEST_LANE_PLANNER_NODE_EXE test_lane_planner_node)
  ament_add_gtest(${TEST_LANE_PLANNER_NODE_EXE} ${TEST_SOURCES})
  target_link_libraries(${TEST_LANE_PLANNER_NODE_EXE} lane_planner_node)
endif()

# ament package generation and installing
ament_auto_package(
  INSTALL_TO_SHARE
    param
)
