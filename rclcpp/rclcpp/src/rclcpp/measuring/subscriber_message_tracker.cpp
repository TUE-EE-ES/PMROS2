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

#include <chrono>

#include "rclcpp/measuring/subscriber_message_tracker.hpp"
#include "rclcpp/measuring/simpletimer.hpp"

using rclcpp::SubscriberMessageTracker;
using rclcpp::hex_char_array_t;

SubscriberMessageTracker::SubscriberMessageTracker(rclcpp::IMeasurementWriter::UniquePtr writer) : IMessageTracker(std::move(writer)) {
    // this creates file streams if you are using file_measurement_writer, even if the host never receives messages.
    // should this class be mindful of it and not register until it is actually getting messages (overhead),
    // or should the writer dependency be mindful itself instead, and just not actually create files until it is receiving measurements?
    latencyKey_ = writer_->register_measurement_class("message_latency", {"publisher_hash","send_time","receive_time"});
    arrivalKey_ = writer_->register_measurement_class("message_arrival", {"publisher_hash","msg_id"});
}

void SubscriberMessageTracker::track_message(const MessageTrackingVariables & msg) {
    // SimpleTimer s("(" + std::to_string(msg.vandenhoven_identifier) + ") subscriber message track");
    auto monotonic_time = get_monotonic_time_64b_ns(); // refresh the stamp for this flurry of measurements. Do this as early as possible.
    writer_->use_timestamp(get_unix_time_64b_ns()); // we need unix time here, not monotonic time! Do not make the mistake I did!! My InfluxDB has data 3 hours past 1970 now!!!
    hex_char_array_t hexString(msg.vandenhoven_publisher_hash);
    
    writer_->record_latency(latencyKey_, msg, hexString, monotonic_time);
    writer_->record_arrival(arrivalKey_, msg, hexString);
}
