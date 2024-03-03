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

#ifndef RCLCPP_PUBLISHER_MESSAGE_TRACKER_HPP_
#define RCLCPP_PUBLISHER_MESSAGE_TRACKER_HPP_

#include <memory>
#include "rclcpp/measuring/message_tracker_interface.hpp"
#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/macros.hpp"

namespace rclcpp {

class PublisherMessageTracker : public IMessageTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(PublisherMessageTracker)

    PublisherMessageTracker(rclcpp::IMeasurementWriter::UniquePtr writer, uint32_t host_hash) : IMessageTracker(std::move(writer)), host_hash_(host_hash) {}

    /// Gather metrics on a message that just got published.
    void track_message(const MessageTrackingVariables &) override;
private:
    int64_t current_msg_id = 1;
    uint32_t host_hash_;
};

} // namespace rclcpp

#endif // RCLCPP_PUBLISHER_MESSAGE_TRACKER_HPP_