// Copyright 2019 the Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Co-developed by Tier IV, Inc. and Apex.AI, Inc.

#ifndef LOCALIZATION_NODES__LOCALIZATION_NODE_HPP_
#define LOCALIZATION_NODES__LOCALIZATION_NODE_HPP_

#include <localization_common/localizer_base.hpp>
#include <localization_common/initialization.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2/buffer_core.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <time_utils/time_utils.hpp>
#include <helper_functions/message_adapters.hpp>
#include <localization_nodes/visibility_control.hpp>
#include <memory>
#include <string>
#include <utility>

namespace autoware
{
namespace localization
{
namespace localization_nodes
{
using common::helper_functions::message_field_adapters::get_frame_id;
using common::helper_functions::message_field_adapters::get_stamp;

/// Helper struct that groups topic name and QoS setting for a publisher or subscription
struct TopicQoS
{
  std::string topic;
  rclcpp::QoS qos;
};

/// Enum to specify if the localizer node must publish to `/tf` topic or not
enum class LocalizerPublishMode
{
  PUBLISH_TF,
  NO_PUBLISH_TF
};

/// Base relative localizer node that publishes map->base_link relative
/// transform messages for a given observation source and map.
/// \tparam ObservationMsgT Message type to register against a map.
/// \tparam MapMsgT Map type
/// \tparam LocalizerT Localizer type.
/// \tparam PoseInitializerT Pose initializer type.
template<typename ObservationMsgT, typename MapMsgT, typename LocalizerT,
  typename PoseInitializerT>
class LOCALIZATION_NODES_PUBLIC RelativeLocalizerNode : public rclcpp::Node
{
public:
  using LocalizerBase = localization_common::RelativeLocalizerBase<ObservationMsgT, MapMsgT,
      typename LocalizerT::RegistrationSummary>;
  using LocalizerBasePtr = std::unique_ptr<LocalizerBase>;
  using Cloud = sensor_msgs::msg::PointCloud2;
  using PoseWithCovarianceStamped = typename LocalizerBase::PoseWithCovarianceStamped;
  using TransformStamped = typename LocalizerBase::Transform;
  using RegistrationSummary = typename LocalizerT::RegistrationSummary;

  /// Constructor
  /// \param node_name Name of node
  /// \param name_space Namespace of node
  /// \param observation_sub_config topic and QoS setting for the observation subscription.
  /// \param map_sub_config topic and QoS setting for the map subscription.
  /// \param pose_pub_config topic and QoS setting for the output pose publisher.
  /// \param initial_pose_sub_config topic and QoS setting for the initialpose subscription.
  /// \param pose_initializer Pose initializer.
  /// \param publish_tf Whether to publish to the `tf` topic. This can be used to publish transform
  /// messages when the relative localizer is the only source of localization.
  RelativeLocalizerNode(
    const std::string & node_name, const std::string & name_space,
    const TopicQoS & observation_sub_config,
    const TopicQoS & map_sub_config,
    const TopicQoS & pose_pub_config,
    const TopicQoS & initial_pose_sub_config,
    const PoseInitializerT & pose_initializer,
    LocalizerPublishMode publish_tf = LocalizerPublishMode::NO_PUBLISH_TF)
  : Node(node_name, name_space),
    m_pose_initializer(pose_initializer),
    m_tf_listener(m_tf_buffer, std::shared_ptr<rclcpp::Node>(this, [](auto) {}), false),
    m_observation_sub(create_subscription<ObservationMsgT>(
        observation_sub_config.topic,
        observation_sub_config.qos,
        [this](typename ObservationMsgT::ConstSharedPtr msg) {observation_callback(msg);})),
    m_map_sub(
      create_subscription<MapMsgT>(
        map_sub_config.topic, map_sub_config.qos,
        [this](typename MapMsgT::ConstSharedPtr msg) {map_callback(msg);})),
    m_pose_publisher(
      create_publisher<PoseWithCovarianceStamped>(
        pose_pub_config.topic,
        pose_pub_config.qos)),
    m_initial_pose_sub(
      create_subscription<PoseWithCovarianceStamped>(
        initial_pose_sub_config.topic, initial_pose_sub_config.qos,
        [this](const typename PoseWithCovarianceStamped::ConstSharedPtr msg) {
          initial_pose_callback(msg);
        })) {
    if (publish_tf == LocalizerPublishMode::PUBLISH_TF) {
      m_tf_publisher = create_publisher<tf2_msgs::msg::TFMessage>("/tf", pose_pub_config.qos);
    }
  }

  // Constructor for ros2 components
  // TODO(yunus.caliskan): refactor constructors together reduce the repeated code.
  RelativeLocalizerNode(
    const std::string & node_name,
    const rclcpp::NodeOptions & options,
    const PoseInitializerT & pose_initializer)
  : Node(node_name, options),
    m_pose_initializer(pose_initializer),
    m_tf_listener(m_tf_buffer, std::shared_ptr<rclcpp::Node>(this, [](auto) {}), false),
    m_observation_sub(create_subscription<ObservationMsgT>(
        "points_in",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter("observation_sub.history_depth").template
            get<size_t>())}},
        [this](typename ObservationMsgT::ConstSharedPtr msg) {observation_callback(msg);})),
    m_map_sub(
      create_subscription<MapMsgT>(
        "ndt_map",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter("map_sub.history_depth").
            template get<size_t>())}}.transient_local(),
        [this](typename MapMsgT::ConstSharedPtr msg) {map_callback(msg);})),
    m_pose_publisher(
      create_publisher<PoseWithCovarianceStamped>(
        "ndt_pose",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter(
              "pose_pub.history_depth").template get<size_t>())}})),
    m_initial_pose_sub(
      create_subscription<PoseWithCovarianceStamped>(
        "initialpose",
        rclcpp::QoS{rclcpp::KeepLast{10}},
        [this](const typename PoseWithCovarianceStamped::ConstSharedPtr msg) {
          initial_pose_callback(msg);
        }))
  {
    init();
  }

  /// Constructor using ros parameters
  /// \param node_name Node name
  /// \param name_space Node namespace
  /// \param pose_initializer Pose initializer
  RelativeLocalizerNode(
    const std::string & node_name, const std::string & name_space,
    const PoseInitializerT & pose_initializer)
  : Node(node_name, name_space),
    m_pose_initializer(pose_initializer),
    m_tf_listener(m_tf_buffer, std::shared_ptr<rclcpp::Node>(this, [](auto) {}), false),
    m_observation_sub(create_subscription<ObservationMsgT>(
        "points_in",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter("observation_sub.history_depth").template
            get<size_t>())}},
        [this](typename ObservationMsgT::ConstSharedPtr msg) {observation_callback(msg);})),
    m_map_sub(
      create_subscription<MapMsgT>(
        "ndt_map",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter("map_sub.history_depth").
            template get<size_t>())}}.transient_local(),
        [this](typename MapMsgT::ConstSharedPtr msg) {map_callback(msg);})),
    m_pose_publisher(
      create_publisher<PoseWithCovarianceStamped>(
        "ndt_pose",
        rclcpp::QoS{rclcpp::KeepLast{
            static_cast<size_t>(declare_parameter(
              "pose_pub.history_depth").template get<size_t>())}})),
    m_initial_pose_sub(
      create_subscription<PoseWithCovarianceStamped>(
        "initialpose",
        rclcpp::QoS{rclcpp::KeepLast{10}},
        [this](const typename PoseWithCovarianceStamped::ConstSharedPtr msg) {
          initial_pose_callback(msg);
        }))
  {
    init();
  }

  /// Get a const pointer of the output publisher. Can be used for matching against subscriptions.
  const typename rclcpp::Publisher<PoseWithCovarianceStamped>::ConstSharedPtr get_publisher()
  {
    return m_pose_publisher;
  }

// protected:
  /// Set the localizer.
  /// \param localizer_ptr rvalue to the localizer to set.
  void set_localizer(LocalizerBasePtr && localizer_ptr)
  {
    m_localizer_ptr = std::forward<LocalizerBasePtr>(localizer_ptr);
  }

  /// Handle the exceptions during registration.
  virtual void on_bad_registration(std::exception_ptr eptr) // NOLINT
  {
    on_exception(eptr, "on_bad_registration");
  }

  /// Handle the exceptions during map setting.
  virtual void on_bad_map(std::exception_ptr eptr) // NOLINT
  {
    on_exception(eptr, "on_bad_map");
  }

  void on_exception(std::exception_ptr eptr, const std::string & error_source)  // NOLINT
  {
    try {
      if (eptr) {
        std::rethrow_exception(eptr);
      } else {
        RCLCPP_ERROR(get_logger(), error_source + ": error nullptr");
      }
    } catch (const std::exception & e) {
      RCLCPP_ERROR(get_logger(), e.what());
    }
  }

  /// Default behavior when an observation is received with no valid existing map.
  virtual void on_observation_with_invalid_map(typename ObservationMsgT::ConstSharedPtr)
  {
    RCLCPP_WARN(
      get_logger(), "Received observation without a valid map, "
      "ignoring the observation.");
  }

  /// Default behavior when hte pose output is evaluated to be invalid.
  /// \param pose Pose output.
  virtual void on_invalid_output(PoseWithCovarianceStamped & pose)
  {
    (void) pose;
    RCLCPP_WARN(
      get_logger(), "Relative localizer has an invalid pose estimate. "
      "The result is ignored.");
  }


  /// Validate the pose estimate given the registration summary and the initial guess.
  /// This function by default returns true.
  /// \param summary Registration summary.
  /// \param pose Pose estimate.
  /// \param guess Initial guess.
  /// \return True if the estimate is valid and can be published.
  virtual bool validate_output(
    const RegistrationSummary & summary,
    const PoseWithCovarianceStamped & pose, const TransformStamped & guess)
  {
    (void) summary;
    (void) pose;
    (void) guess;
    return true;
  }

// private:
  void init()
  {
    if (declare_parameter("publish_tf").template get<bool>()) {
      m_tf_publisher = create_publisher<tf2_msgs::msg::TFMessage>(
        "/tf",
        rclcpp::QoS{rclcpp::KeepLast{m_pose_publisher->get_queue_size()}});
    }

    if (declare_parameter("init_hack.enabled", false)) {
      /////////////////////////////////////////////////
      // TODO(yunus.caliskan): Remove in #425
      // Since this hack is only needed for the demo, it is not provided in the non-ros constructor.
      geometry_msgs::msg::TransformStamped init_hack_transform;
      auto & tf = init_hack_transform.transform;
      tf.rotation.x = declare_parameter("init_hack.quaternion.x").template get<float64_t>();
      tf.rotation.y = declare_parameter("init_hack.quaternion.y").template get<float64_t>();
      tf.rotation.z = declare_parameter("init_hack.quaternion.z").template get<float64_t>();
      tf.rotation.w = declare_parameter("init_hack.quaternion.w").template get<float64_t>();
      tf.translation.x = declare_parameter("init_hack.translation.x").template get<float64_t>();
      tf.translation.y = declare_parameter("init_hack.translation.y").template get<float64_t>();
      tf.translation.z = declare_parameter("init_hack.translation.z").template get<float64_t>();
      init_hack_transform.header.frame_id = "map";
      init_hack_transform.child_frame_id = "base_link";
      m_external_pose = init_hack_transform;
      m_external_pose_available = true;
      m_use_hack = true;
      // we currently need the hack for the AVP demo MS2.
      ////////////////////////////////////////////////////
    }
  }

  /// Process the registration summary. By default does nothing.
  virtual void handle_registration_summary(const RegistrationSummary &) {}

  // FIXMESB: bad class design

  /// Callback that registers each received observation and outputs the result.
  /// \param msg_ptr Pointer to the observation message.
  void observation_callback(typename ObservationMsgT::ConstSharedPtr msg_ptr)
  {
    check_localizer();
    if (!m_localizer_ptr->map_valid()) {
      on_observation_with_invalid_map(msg_ptr);
      return;
    }

    const auto observation_time = ::time_utils::from_message(get_stamp(*msg_ptr));
    const auto & observation_frame = get_frame_id(*msg_ptr);
    const auto & map_frame = m_localizer_ptr->map_frame_id();

    try {
      geometry_msgs::msg::TransformStamped initial_guess;
      if (m_external_pose_available) {
        // If someone set a transform and then requests a different transform, that's an error
        if (m_external_pose.header.frame_id != map_frame ||
          m_external_pose.child_frame_id != observation_frame)
        {
          throw std::runtime_error(
                  "The pose initializer's set_external_pose() "
                  "and guess() methods were called with different frames.");
        }
        m_external_pose_available = false;
        initial_guess = m_external_pose;
        initial_guess.header.stamp = get_stamp(*msg_ptr);
      } else {
        initial_guess =
          m_pose_initializer.guess(m_tf_buffer, observation_time, map_frame, observation_frame);
      }

      PoseWithCovarianceStamped pose_out;
      const auto summary =
        m_localizer_ptr->register_measurement(*msg_ptr, initial_guess, pose_out);
      if (validate_output(summary, pose_out, initial_guess)) {
        m_pose_publisher->publish(pose_out);
        // This is to be used when no state estimator or alternative source of
        // localization is available.
        if (m_tf_publisher) {
          publish_tf(pose_out);
          // republish point cloud so visualization has no issues with the timestamp
          // being too new (no transform yet). Reset the timestamp to zero so visualization
          // is not bothered if odom->base_link transformation is available
          // only at different time stamps.
          auto msg = *msg_ptr;
          msg.header.stamp = time_utils::to_message(tf2::TimePointZero);
          m_obs_republisher->publish(msg);
        }

        handle_registration_summary(summary);
      } else {
        on_invalid_output(pose_out);
      }
    } catch (...) {
      // TODO(mitsudome-r) remove this hack in #458
      if (m_tf_publisher && m_use_hack) {
        republish_tf(get_stamp(*msg_ptr));
      }
      on_bad_registration(std::current_exception());
    }
  }

  /// Callback that updates the map.
  /// \param msg_ptr Pointer to the map message.
  void map_callback(typename MapMsgT::ConstSharedPtr msg_ptr)
  {
    check_localizer();
    try {
      m_localizer_ptr->set_map(*msg_ptr);
    } catch (...) {
      on_bad_map(std::current_exception());
    }
  }

  /// Publish the pose message as a transform.
  void publish_tf(const PoseWithCovarianceStamped & pose_msg)
  {
    const auto & pose = pose_msg.pose.pose;
    tf2::Quaternion rotation{pose.orientation.x, pose.orientation.y, pose.orientation.z,
      pose.orientation.w};
    tf2::Vector3 translation{pose.position.x, pose.position.y, pose.position.z};
    const tf2::Transform map_base_link_transform{rotation, translation};

    // Wait for odom to base_link transform to be available
    bool odom_to_bl_found = m_tf_buffer.canTransform("odom", "base_link", tf2::TimePointZero);

    while (!odom_to_bl_found) {
      RCLCPP_INFO(get_logger(), "Waiting for odom to base_link transform to be available.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      odom_to_bl_found = m_tf_buffer.canTransform("odom", "base_link", tf2::TimePointZero);
    }

    geometry_msgs::msg::TransformStamped odom_tf;
    try {
      odom_tf = m_tf_buffer.lookupTransform(
        "odom", "base_link",
        time_utils::from_message(pose_msg.header.stamp));
    } catch (const tf2::ExtrapolationException &) {
      odom_tf = m_tf_buffer.lookupTransform("odom", "base_link", tf2::TimePointZero);
    }
    tf2::Quaternion odom_rotation{odom_tf.transform.rotation.x,
      odom_tf.transform.rotation.y, odom_tf.transform.rotation.z, odom_tf.transform.rotation.w};
    tf2::Vector3 odom_translation{odom_tf.transform.translation.x, odom_tf.transform.translation.y,
      odom_tf.transform.translation.z};
    const tf2::Transform odom_base_link_transform{odom_rotation, odom_translation};

    const auto map_odom_tf = map_base_link_transform * odom_base_link_transform.inverse();

    tf2_msgs::msg::TFMessage tf_message;
    geometry_msgs::msg::TransformStamped tf_stamped;
    tf_stamped.header.stamp = pose_msg.header.stamp;
    tf_stamped.header.frame_id = m_localizer_ptr->map_frame_id();
    tf_stamped.child_frame_id = "odom";
    const auto & tf_trans = map_odom_tf.getOrigin();
    const auto & tf_rot = map_odom_tf.getRotation();
    tf_stamped.transform.translation.set__x(tf_trans.x()).set__y(tf_trans.y()).
    set__z(tf_trans.z());
    tf_stamped.transform.rotation.set__x(tf_rot.x()).set__y(tf_rot.y()).set__z(tf_rot.z()).
    set__w(tf_rot.w());
    tf_message.transforms.push_back(tf_stamped);
    m_tf_publisher->publish(tf_message);
  }

  // TODO(mitsudome-r) remove this hack in #458
  /// Publish the pose message as a transform.
  void republish_tf(builtin_interfaces::msg::Time stamp)
  {
    auto map_odom_tf = m_tf_buffer.lookupTransform(
      m_localizer_ptr->map_frame_id(), "odom",
      tf2::TimePointZero);
    map_odom_tf.header.stamp = stamp;
    tf2_msgs::msg::TFMessage tf_message;
    tf_message.transforms.push_back(map_odom_tf);
    m_tf_publisher->publish(tf_message);
  }

  /// Check if localizer exist, throw otherwise.
  void check_localizer() const
  {
    if (!m_localizer_ptr) {
      throw std::runtime_error(
              "Localizer node needs a valid localizer to be set before it "
              "can register measurements. Call `set_localizer(...)` first.");
    }
  }

  void initial_pose_callback(const typename PoseWithCovarianceStamped::ConstSharedPtr msg_ptr)
  {
    // The child frame is implicitly base_link.
    // Ensure the parent frame is the map frame
    const std::string & map_frame = m_localizer_ptr->map_frame_id();
    if (!m_tf_buffer.canTransform(map_frame, msg_ptr->header.frame_id, tf2::TimePointZero)) {
      RCLCPP_ERROR(
        get_logger(),
        "Failed to find transform from %s to %s frame. Failed to give initial pose.",
        msg_ptr->header.frame_id, map_frame);
      return;
    }
    const auto transform = m_tf_buffer.lookupTransform(
      map_frame, msg_ptr->header.frame_id,
      tf2::TimePointZero);


    geometry_msgs::msg::TransformStamped input_pose_stamped;
    input_pose_stamped.header = msg_ptr->header;
    input_pose_stamped.child_frame_id = "base_link";
    input_pose_stamped.transform.rotation = msg_ptr->pose.pose.orientation;
    input_pose_stamped.transform.translation.x = msg_ptr->pose.pose.position.x;
    input_pose_stamped.transform.translation.y = msg_ptr->pose.pose.position.y;
    input_pose_stamped.transform.translation.z = msg_ptr->pose.pose.position.z;
    geometry_msgs::msg::TransformStamped transformed_pose_stamped;
    tf2::doTransform(input_pose_stamped, transformed_pose_stamped, transform);

    // Note: The frame_id in the result is already the map_frame, no need to set it.
    transformed_pose_stamped.child_frame_id = "base_link";

    // For future reference: We can't write this pose to /tf here.
    // If this message comes from RViz and we're running in simulation, RViz
    // and data coming from LGSVL (the observations) will sometimes have very
    // large differences in their timestamps (e.g. RViz being 40s newer).
    // If this pose was published to /tf, it would be seen as being way in the
    // future and the localizer couldn't use it as its next initial pose.
    // We'd need to know the current time before it can be published, and set the
    // time in the header to a recent time.
    m_external_pose = transformed_pose_stamped;
    m_external_pose_available = true;
  }

  LocalizerBasePtr m_localizer_ptr;
  PoseInitializerT m_pose_initializer;
  tf2::BufferCore m_tf_buffer;
  tf2_ros::TransformListener m_tf_listener;
  typename rclcpp::Subscription<ObservationMsgT>::SharedPtr m_observation_sub;
  typename rclcpp::Subscription<MapMsgT>::SharedPtr m_map_sub;
  typename rclcpp::Publisher<PoseWithCovarianceStamped>::SharedPtr m_pose_publisher;
  typename rclcpp::Publisher<tf2_msgs::msg::TFMessage>::SharedPtr m_tf_publisher{nullptr};
  typename rclcpp::Publisher<ObservationMsgT>::SharedPtr m_obs_republisher{
    create_publisher<ObservationMsgT>("observation_republish", 10)};

  // Receive updates from "/initialpose" (e.g. rviz2)
  typename rclcpp::Subscription<PoseWithCovarianceStamped>::SharedPtr m_initial_pose_sub;
  // Stores "/initialpose", the timestamp is not used/valid
  geometry_msgs::msg::TransformStamped m_external_pose;
  bool m_external_pose_available{false};

  bool m_use_hack{false};
};
}  // namespace localization_nodes
}  // namespace localization
}  // namespace autoware
#endif  // LOCALIZATION_NODES__LOCALIZATION_NODE_HPP_
