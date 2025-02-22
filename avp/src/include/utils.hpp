#ifndef LFAVP_UTILS_HPP
#define LFAVP_UTILS_HPP

#include "rclcpp/parameter_map.hpp"
#include "rcl_yaml_param_parser/parser.h"
#include <string>
#include <cstdlib>

extern "C" {
	#include "util.h"
}

inline rclcpp::NodeOptions get_node_options_from_yaml(const char* path, const char* root_name) {
	// Use a try - catch for now so that if the rclcpp context is already initialized,
	// ROS doesn't throw a tantrum.
	try {
		rclcpp::init(0, NULL);
	} catch (...) { /*  Ignore */ }
	rclcpp::NodeOptions nodeOptions;
	rcl_params_t* params = rcl_yaml_node_struct_init(rcl_get_default_allocator());
	bool out = rcl_parse_yaml_file(path,params);
	if (!out) {
		lf_print_error_and_exit("Failed to load the yaml file.");
	}

	auto options = rclcpp::parameter_map_from(params);
	for (std::pair<const std::__cxx11::basic_string<char>, std::vector<rclcpp::Parameter> > option : options) {
		if (option.first == root_name) {
			nodeOptions.parameter_overrides() = option.second;
		}
	}

	return nodeOptions;
}

inline rclcpp::NodeOptions get_node_options_from_yaml(const char* path, const char* root_name, int argc, char *argv[]) {
	try {
		rclcpp::init(argc, argv);
	} catch (...) { /*  Ignore */ }
	return get_node_options_from_yaml(path, root_name);
}

template <typename ServiceT, typename ValueT>
auto create_future_from_service(std::shared_ptr<ValueT> ros_service_msg) {
    // The following is how ROS 2 creates a future for a client of a service
    using SharedResponse = typename ServiceT::Response::SharedPtr;
    using Promise = std::promise<SharedResponse>;
    using SharedPromise = std::shared_ptr<Promise>;
    using SharedFuture = std::shared_future<SharedResponse>;
    SharedPromise promise = std::make_shared<Promise>();
    SharedFuture future(promise->get_future());
    SharedResponse response = std::make_shared<typename ServiceT::Response>();
    response->map = *ros_service_msg.get();
    promise->set_value(response);
    return future;
}

inline std::string get_lfavp_config() {
	const char* lfavp_config = std::getenv("LFAVP_CONFIG");
	if (!lfavp_config) {
		lf_print_error_and_exit("ERROR: Environment variable $LFAVP_CONFIG is not declared.");
	}
	return std::string(lfavp_config);
}

inline std::thread create_spin_thread(std::shared_ptr<rclcpp::Node> node) {
	return std::thread(
		[=]() {
			while (rclcpp::ok()) {
				rclcpp::spin(node);
			}
		}
	);
}

// This templated function can be passed to lf_set_destructor so that
// the C-runtimes token management uses `delete` instead of the default `free`.
// This is necessary if the tokens were created with `new`.
template<typename T>
void cpp_delete(void * value) {
		delete static_cast<T*>(value);
}

#endif // LFAVP_UTILS_HPP
