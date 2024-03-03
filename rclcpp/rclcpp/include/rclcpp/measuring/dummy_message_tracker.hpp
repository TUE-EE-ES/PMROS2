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

#ifndef RCLCPP_DUMMY_MESSAGE_TRACKER_HPP_
#define RCLCPP_DUMMY_MESSAGE_TRACKER_HPP_

#include <memory>
#include "rclcpp/measuring/message_tracker_interface.hpp"

namespace rclcpp {

class DummyMessageTracker : public IMessageTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(DummyMessageTracker)

    using IMessageTracker::IMessageTracker;

    /// Do not gather any metrics, just return.
    void track_message(const MessageTrackingVariables &) override; 
};

} // namespace rclcpp

#endif // RCLCPP_DUMMY_MESSAGE_TRACKER_HPP_