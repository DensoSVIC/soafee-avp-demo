# Copyright 2017-2020 the Autoware Foundation
# Co-developed by Tier IV, Inc. and Apex.AI, Inc.
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

### Build the nodes
project(euclidean_cluster_nodes)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

## dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Library
set(CLUSTER_NODE_LIB euclidean_cluster_node)
ament_auto_add_library(${CLUSTER_NODE_LIB} SHARED
  include/euclidean_cluster_nodes/euclidean_cluster_node.hpp
  src/euclidean_cluster_node.cpp)
autoware_set_compile_options(${CLUSTER_NODE_LIB})
target_compile_options(
  ${CLUSTER_NODE_LIB} PUBLIC "-Wno-sign-conversion" "-Wno-conversion")

rclcpp_components_register_node(${CLUSTER_NODE_LIB}
  PLUGIN "autoware::perception::segmentation::euclidean_cluster_nodes::EuclideanClusterNode"
  EXECUTABLE ${CLUSTER_NODE_LIB}_exe
)

## Testing
if(BUILD_TESTING)
  # Static checking only if built via ament
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  add_ros_test(
    test/euclidean_cluster_node_launch.test.py
    TIMEOUT "30"
  )

  # gtest
  set(CLUSTER_NODE_GTEST eucliden_cluster_nodes_gtest)
  ament_add_gtest(${CLUSTER_NODE_GTEST} test/test_euclidean_cluster_nodes.cpp)
  target_link_libraries(${CLUSTER_NODE_GTEST} ${CLUSTER_NODE_LIB})
  target_include_directories(${CLUSTER_NODE_GTEST} PRIVATE "test/include" "include")
  ament_target_dependencies(${CLUSTER_NODE_GTEST} "rclcpp")
endif()
# Ament Exporting
ament_auto_package(INSTALL_TO_SHARE
  launch
  param)
