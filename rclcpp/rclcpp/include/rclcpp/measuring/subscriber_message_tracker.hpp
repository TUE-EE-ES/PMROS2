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

#ifndef RCLCPP_SUBSCRIBER_MESSAGE_TRACKER_HPP_
#define RCLCPP_SUBSCRIBER_MESSAGE_TRACKER_HPP_

#include <memory>
#include <unordered_map>
#include "rclcpp/measuring/message_tracker_interface.hpp"

namespace rclcpp {

class SubscriberMessageTracker : public IMessageTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(SubscriberMessageTracker)

    SubscriberMessageTracker(rclcpp::IMeasurementWriter::UniquePtr writer);

    /// Gathers metrics on a message that just got received.
    void track_message(const MessageTrackingVariables &) override;

private:
    uint32_t latencyKey_;
    uint32_t arrivalKey_;
};

} // namespace rclcpp

#endif // RCLCPP_SUBSCRIBER_MESSAGE_TRACKER_HPP_