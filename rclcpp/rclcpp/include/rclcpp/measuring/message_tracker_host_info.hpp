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

#ifndef RCLCPP__MESSAGE_TRACKER_HOST_INFO_HPP_
#define RCLCPP__MESSAGE_TRACKER_HOST_INFO_HPP_

#include "rcl/node.h"
#include <cstring>
#include "rclcpp/hashing/mmh3.hpp"

namespace rclcpp {

// Information that the MessageTrackerFactory needs in some cases
// Specifically, the MeasurementWriter dependency of type 'File'  needs it for naming the files appropriately.
struct MessageTrackerHostInfo {
    MessageTrackerHostInfo(const char * tn, const char * name, const char * ns) : topic_name(tn), node_name(name), node_namespace(ns) {}
    MessageTrackerHostInfo(const char * tn, const rcl_node_t * node_handle) : topic_name(tn) {
        node_name = rcl_node_get_name(node_handle);
        node_namespace = rcl_node_get_namespace(node_handle);
    }

    const char * topic_name;
    const char * node_name;
    const char * node_namespace;

    uint32_t hash_full_node_name() const {
        // looks like it's either unsupported VLA, or heap allocation. rock and a hard place.
        size_t concat_len = strlen(node_namespace) + strlen(node_name);
        char * concatenated = new char[concat_len + 1]; // +1 for null terminator. It is necessary to have this because strcat will write a 0 there.

        strcpy(concatenated, node_namespace);
        strcat(concatenated, node_name);

        // note: The 'seed' parameter is not relevant for us yet. We could make use of it in the event of a collision, retrying the hash with seed '1', '2', ...
        // - That is, if collisions are even detectable in ros2 (distributed system, after all)
        // Alternatively, we could 'dodge' duplicately-named nodes by seeding with PID? this has a downside that naming is not consistent per run.
        uint32_t hash;
        MurmurHash3_x86_32(concatenated, static_cast<int>(concat_len), 0, &hash);

        delete[] concatenated;

        return hash;
    }
};

} // namespace rclcpp

#endif // RCLCPP__MESSAGE_TRACKER_HOST_INFO_HPP_