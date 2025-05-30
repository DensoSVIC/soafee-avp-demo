#ifndef LFAVP_CONSTANTS_HPP
#define LFAVP_CONSTANTS_HPP

#include <string>

// Paths within the AUTOWARE_HOME directory
const std::string LAUNCH_PARAM_PATH = "src/launch/autoware_auto_launch/param";
const std::string AVP_DEMO_PARAM_PATH = "src/tools/autoware_auto_avp_demo/param";
const std::string AVP_DEMO_CONFIG_PATH = "src/tools/autoware_auto_avp_demo/config";

// AVP Demo parameters
const std::string PC_FILTER_TRANSFORM_PARAM = "pc_filter_transform.param.yaml";
const std::string EUCLIDEAN_CLUSTER_DETECTOR_PARAM = "euclidean_cluster.param.yaml";
const std::string LANE_PLANNER_PARAM = "lane_planner.param.yaml";
const std::string LANELET2_MAP_PROVIDER_PARAM = "lanelet2_map_provider.param.yaml";
const std::string NDT_LOCALIZER_PARAM = "ndt_localizer.param.yaml";
const std::string NDT_MAP_PUBLISHER_PARAM = "map_publisher.param.yaml";
const std::string OFF_MAP_OBSTACLES_FILTER_PARAM = "off_map_obstacles_filter.param.yaml";
const std::string OBJ_COLLISION_EST_PARAM = "object_collision_estimator.param.yaml";
const std::string PARKING_PLANNER_PARAM = "parking_planner.param.yaml";
const std::string MPC_CONTROLLER_PARAM = "mpc.param.yaml";
const std::string BEHAVIOR_PLANNER_PARAM = "behavior_planner.param.yaml";
const std::string RVIZ_CONFIG = "ms3.rviz";
const std::string LGSVL_INTERFACE_CONFIG = "lgsvl_interface.param.yaml";
const std::string VLP16_SIM_LEXUS_PC_FUSION_CONFIG = "vlp16_sim_lexus_pc_fusion.param.yaml";
const std::string RAY_GROUND_CLASSIFIER_CONFIG = "ray_ground_classifier.param.yaml";
const std::string SCAN_DOWNSAMPLER_MS3_CONFIG = "scan_downsampler_ms3.param.yaml";

//URDF
const std::string VEHICLE_URDF = "lexus_rx_450h_description/urdf/lexus_rx_450h.urdf";

#endif // LFAVP_CONSTANTS_HPP
