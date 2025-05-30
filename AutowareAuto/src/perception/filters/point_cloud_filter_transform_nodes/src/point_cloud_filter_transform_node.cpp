// Copyright 2017-2020 the Autoware Foundation
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
//
// Co-developed by Tier IV, Inc. and Apex.AI, Inc.

#include <common/types.hpp>
#include <point_cloud_filter_transform_nodes/point_cloud_filter_transform_node.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <memory>
#include <string>

namespace autoware
{
namespace perception
{
namespace filters
{
/// \brief Boilerplate Apex.OS nodes around point_cloud_filter_transform_nodes
namespace point_cloud_filter_transform_nodes
{
using autoware::common::lidar_utils::add_point_to_cloud;
using autoware::common::lidar_utils::has_intensity_and_throw_if_no_xyz;
using autoware::common::lidar_utils::reset_pcl_msg;
using autoware::common::lidar_utils::resize_pcl_msg;
using autoware::common::lidar_utils::sanitize_point_cloud;
using autoware::common::types::float64_t;
using autoware::common::types::PointXYZIF;
using geometry_msgs::msg::TransformStamped;
using sensor_msgs::msg::PointCloud2;

TransformStamped get_transform(
  const std::string & input_frame_id,
  const std::string & output_frame_id,
  float64_t r_x, float64_t r_y, float64_t r_z, float64_t r_w, float64_t t_x,
  float64_t t_y, float64_t t_z)
{
  TransformStamped ret;
  ret.header.frame_id = input_frame_id;
  ret.child_frame_id = output_frame_id;
  ret.transform.rotation.x = r_x;
  ret.transform.rotation.y = r_y;
  ret.transform.rotation.z = r_z;
  ret.transform.rotation.w = r_w;
  ret.transform.translation.x = t_x;
  ret.transform.translation.y = t_y;
  ret.transform.translation.z = t_z;
  return ret;
}

PointCloud2FilterTransformNode::PointCloud2FilterTransformNode(const rclcpp::NodeOptions & node_options)
: Node("point_cloud_filter_transform_node", node_options),
  m_angle_filter{
    static_cast<float32_t>(declare_parameter("start_angle").get<float64_t>()),
    static_cast<float32_t>(declare_parameter("end_angle").get<float64_t>())},
  m_distance_filter{
    static_cast<float32_t>(declare_parameter("min_radius").get<float64_t>()),
    static_cast<float32_t>(declare_parameter("max_radius").get<float64_t>())},
  m_input_frame_id{declare_parameter("input_frame_id").get<std::string>()},
  m_output_frame_id{declare_parameter("output_frame_id").get<std::string>()},
  m_static_transformer{
    get_transform(
      m_input_frame_id, m_output_frame_id,
      declare_parameter("static_transformer.quaternion.x").get<float64_t>(),
      declare_parameter("static_transformer.quaternion.y").get<float64_t>(),
      declare_parameter("static_transformer.quaternion.z").get<float64_t>(),
      declare_parameter("static_transformer.quaternion.w").get<float64_t>(),
      declare_parameter("static_transformer.translation.x").get<float64_t>(),
      declare_parameter("static_transformer.translation.y").get<float64_t>(),
      declare_parameter("static_transformer.translation.z").get<float64_t>()).transform},
  m_init_timeout{std::chrono::milliseconds{declare_parameter("init_timeout_ms").get<int32_t>()}},
  m_timeout{std::chrono::milliseconds{declare_parameter("timeout_ms").get<int32_t>()}},
  m_sub_ptr{create_subscription<PointCloud2>(
      "points_in", rclcpp::QoS{10},
      std::bind(
        &PointCloud2FilterTransformNode::process_filtered_transformed_message, this, _1))},
  m_pub_ptr{create_publisher<PointCloud2>("points_filtered", rclcpp::QoS{10})},
  m_expected_num_publishers{
    static_cast<size_t>(declare_parameter("expected_num_publishers").get<int32_t>())},
  m_expected_num_subscribers{
    static_cast<size_t>(declare_parameter("expected_num_subscribers").get<int32_t>())},
  m_pcl_size{static_cast<size_t>(declare_parameter("pcl_size").get<int32_t>())}
{
  common::lidar_utils::init_pcl_msg(
    m_filtered_transformed_msg,
    m_output_frame_id.c_str(), m_pcl_size);
}

PointCloud2FilterTransformNode::PointCloud2FilterTransformNode(
  const rclcpp::NodeOptions & node_options, 
  void* handle_m_filtered_transformed_msg,
  int (*schedule_value)(void*, int64_t, void*, int))
: PointCloud2FilterTransformNode(node_options)
{
  this->handle_m_filtered_transformed_msg = handle_m_filtered_transformed_msg;
  this->schedule_value = schedule_value;
}

// Called in callback of points_raw/points_in
const PointCloud2 & PointCloud2FilterTransformNode::filter_and_transform(const PointCloud2 & msg)
{
  // Verify frame_id
  if (msg.header.frame_id != m_input_frame_id) {
    throw std::runtime_error(
            "Raw topic from unexpected frame. Expected: " +
            m_input_frame_id + ", got: " + msg.header.frame_id);
  }

  sensor_msgs::PointCloud2ConstIterator<float32_t> x_it(msg, "x");
  sensor_msgs::PointCloud2ConstIterator<float32_t> y_it(msg, "y");
  sensor_msgs::PointCloud2ConstIterator<float32_t> z_it(msg, "z");

  auto && intensity_it = autoware::common::lidar_utils::IntensityIteratorWrapper(msg);

  auto point_cloud_idx = 0U;
  reset_pcl_msg(m_filtered_transformed_msg, m_pcl_size, point_cloud_idx);
  m_filtered_transformed_msg.header.stamp = msg.header.stamp;

  for (size_t it = 0; it < (msg.data.size() / 16); it++) {
    PointXYZIF pt;
    pt.x = *x_it;
    pt.y = *y_it;
    pt.z = *z_it;
    intensity_it.get_current_value(pt.intensity);

    if (point_not_filtered(pt)) {
      auto transformed_point = transform_point(pt);
      transformed_point.intensity = pt.intensity;
      if (!add_point_to_cloud(
          m_filtered_transformed_msg, transformed_point, point_cloud_idx))
      {
        throw std::runtime_error(
                "Overran cloud msg point capacity");
      }
    }

    ++x_it;
    ++y_it;
    ++z_it;
    intensity_it.next();

    if (intensity_it.eof()) {
      break;
    }
  }
  resize_pcl_msg(m_filtered_transformed_msg, point_cloud_idx);

  return m_filtered_transformed_msg;
}

void
PointCloud2FilterTransformNode::process_filtered_transformed_message(
  const PointCloud2::SharedPtr msg)
{
  filter_and_transform(*msg);
  auto msg_copy = new PointCloud2(m_filtered_transformed_msg);
  this->schedule_value(this->handle_m_filtered_transformed_msg, 0, static_cast<void*>(msg_copy), 1);
  // m_pub_ptr->publish(filtered_transformed_msg);
}

}  // namespace point_cloud_filter_transform_nodes
}  // namespace filters
}  // namespace perception
}  // namespace autoware

RCLCPP_COMPONENTS_REGISTER_NODE(
  autoware::perception::filters::point_cloud_filter_transform_nodes::PointCloud2FilterTransformNode)
