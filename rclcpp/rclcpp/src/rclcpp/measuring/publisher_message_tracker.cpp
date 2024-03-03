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

#include <iostream>

#include "rclcpp/measuring/publisher_message_tracker.hpp"

using rclcpp::PublisherMessageTracker;

void PublisherMessageTracker::track_message(const MessageTrackingVariables & msg) {
    auto & message = const_cast<MessageTrackingVariables &>(msg);

    message.vandenhoven_timestamp = get_monotonic_time_64b_ns();

    message.vandenhoven_identifier = current_msg_id;
    current_msg_id++;

    message.vandenhoven_publisher_hash = host_hash_;
}