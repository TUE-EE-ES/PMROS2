// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#ifndef RCLCPP__TIMER_OPTIONS_HPP_
#define RCLCPP__TIMER_OPTIONS_HPP_

#include <sstream>

#include "rclcpp/measuring/message_tracker_options.hpp" // MeasurementWriterEnum
#include "rclcpp/measuring/jitter_tracker_options.hpp"
#include "rcl/node.h"

namespace rclcpp {

struct TimerOptions {
    // The only timer option as of now is providing a default for its tracker.
    // Future developers could integrate other timer.cpp constructor params such as the clock onto this struct,
    // However the goal currently is to provide a backwards-compatible add-on such that any existing ROS application can switch into this version.
    // In the future, backwards compatability can be traded for convenience by moving all Timer constructor params into this struct.
    TimerOptions() : attached_to_node(nullptr) {
        std::stringstream ss;
        ss << "independent timer (" << reinterpret_cast<void*>(this) << ")";

        timer_name = ss.str();
    }

    TimerOptions(const rcl_node_t * node) : attached_to_node(node) {
        timer_name = rcl_node_get_fully_qualified_name(node);
    }

    /// Useful when multiple timers are attached to a node.
    TimerOptions(const rcl_node_t * node, const std::string& distinguisher) : attached_to_node(node) {
        timer_name = rcl_node_get_fully_qualified_name(node);
        timer_name += distinguisher;
    }

    JitterTrackerOptions jitter_tracking_options = JitterTrackerOptions(
        rclcpp::JitterTrackerEnum::ACTIVATION_JITTER,
        rclcpp::MeasurementWriterEnum::INFLUXDB
    );

    std::string timer_name;

private:
    const rcl_node_t * attached_to_node;
};

}

#endif // RCLCPP__TIMER_OPTIONS_HPP_