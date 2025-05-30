// Copyright 2019 Christopher Ho
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mpc_controller_nodes/mpc_controller_node.hpp"

#include <memory>

int32_t main(int32_t argc, char ** argv)
{
  rclcpp::init(argc, argv);

  using motion::control::mpc_controller_nodes::MpcControllerNode;
  const auto nd = std::make_shared<MpcControllerNode>("mpc_controller", "", rclcpp::NodeOptions(rcl_get_default_allocator()));

  rclcpp::spin(nd);

  rclcpp::shutdown();

  return 0;
}
