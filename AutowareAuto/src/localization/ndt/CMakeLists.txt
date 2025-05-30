# Copyright 2019-2020 Apex.AI, Inc., Arm Limited
# All rights reserved.

cmake_minimum_required(VERSION 3.5)
project(ndt)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#dependencies
find_package(Eigen3 REQUIRED)
find_package(GEOGRAPHICLIB REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(PCL 1.10 REQUIRED COMPONENTS io)
ament_auto_find_build_dependencies()

# includes
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR} ${YAML_CPP_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS})

set(NDT_NODES_LIB_SRC
    src/ndt.cpp
    src/ndt_map.cpp
    src/ndt_map_publisher.cpp
    src/ndt_voxel.cpp
    src/ndt_voxel_view.cpp
)

set(NDT_NODES_LIB_HEADERS
    include/ndt/visibility_control.hpp
    include/ndt/ndt_config.hpp
    include/ndt/ndt_common.hpp
    include/ndt/ndt_optimization_problem.hpp
    include/ndt/ndt_voxel.hpp
    include/ndt/ndt_voxel_view.hpp
    include/ndt/ndt_map.hpp
    include/ndt/ndt_map_publisher.hpp
    include/ndt/ndt_scan.hpp
    include/ndt/ndt_localizer.hpp
    include/ndt/utils.hpp)

ament_auto_add_library(
${PROJECT_NAME} SHARED
        ${NDT_NODES_LIB_SRC}
)

autoware_set_compile_options(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
  ${GeographicLib_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  ${PCL_LIBRARIES})
# turn off warnings to be able to successfully compile upstream ros packages
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${tf2_INCLUDE_DIRS})
set(ROS_NO_WARN_LIST
        -Wno-sign-conversion
        -Wno-conversion
        -Wno-old-style-cast
        -Wno-useless-cast
        -Wno-double-promotion
        -Wno-nonnull-compare
        -Wuseless-cast)
target_compile_options(${PROJECT_NAME} PRIVATE ${ROS_NO_WARN_LIST})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(PCL 1.10 REQUIRED COMPONENTS registration)
  include_directories(SYSTEM ${PCL_INCLUDE_DIRS})

  # gtest
  set(NDT_TEST ndt_gtest)

  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(${NDT_TEST}
          test/test_ndt_utils.hpp
          test/test_ndt_utils.cpp
          test/test_ndt_map.hpp
          test/test_ndt_scan.hpp
          test/test_ndt_map.cpp
          test/test_ndt_scan.cpp
          test/test_ndt_optimization.hpp
          test/test_ndt_optimization.cpp
          test/test_ndt_localizer.cpp)

  target_link_libraries(${NDT_TEST} ${PROJECT_NAME} ${PCL_LIBRARIES})
  autoware_set_compile_options(${NDT_TEST})
  # turn off warnings to be able to successfully compile upstream ros packages
  target_compile_options(${NDT_TEST} PRIVATE ${ROS_NO_WARN_LIST})
endif()

ament_export_include_directories(${EIGEN3_INCLUDE_DIR} ${PCL_INCLUDE_DIRS})
ament_auto_package()
