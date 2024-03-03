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

#ifndef RCLCPP__MESSAGE_TRACKER_INTERFACE_HPP_
#define RCLCPP__MESSAGE_TRACKER_INTERFACE_HPP_

#include <memory>
#include <chrono>
#include "rclcpp/macros.hpp"
#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/measuring/message_tracking_variables.hpp"

using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::nanoseconds;

namespace rclcpp {

class IMessageTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(IMessageTracker)

    virtual ~IMessageTracker() {}

    IMessageTracker() = delete;
    IMessageTracker(rclcpp::IMeasurementWriter::UniquePtr writer) : writer_(std::move(writer)) {}

    /// Gather metrics on the provided message. The metrics that are gathered vary by implementation of the interface, e.g. metrics for publishers, metrics for subscribers.
    virtual void track_message(const MessageTrackingVariables & msg) = 0;

    template <typename MessageT>
    void track_message(const MessageT & msg) {

        // Ideally, this would be a static assert. 
        // However, actions (as opposed to messages and services) do not have the messagetracking variables and are thus indeed not convertible.
        // Actions are generated as a group of structs that do have messageTrackingVariables, with a parent struct encapsulating them.
        // But even there, there are exceptions.

        // It seems an action generates the following structs:
        // STRUCT_NAME                  HAS_VARIABLES      HAS_VARIABLES_CONTAINED
        // [name]_Goal_                 X                  X
        // [name]_Result_               X                  X
        // [name]_Feedback_             X                  X
        // [name]_SendGoal_Request_                        X   ('goal' member is of type [name]_Goal_)
        // [name]_SendGoal_Response_                           (empty struct providing aliases for SendGoal_Request_ and SendGoal_Response_)
        // [name]_GetResult_Request_                           (This is a real struct that gets used, but does not have tracking variables)
        // [name]_GetResult_Response_                      X   ('result' member is of type [name]_Result_)
        // [name]_GetResult_                                   (empty struct providing aliases for GetResult_Request_ and GetResult_Response_)
        // [name]_FeedbackMessage_                         X   ('feedback' member is of type [name]_Feedback_)
        // [name]                                              (empty struct providing aliases for everything)

        // As such, figuring out for a template type if it is an action that is still track-able is complex,
        // And additionally the use-case of actions (Long running cancellable services with periodic feedback),
        // Does not match the type of tracking we want to implement on messages (latency, throughput, drop rate)/
        // Therefore, we choose to demote the static_assert to just an if statement.
        // In an ideal world, the static assert would be split up into many constexpr bool's (is_message, is_action_type_x, is_action_type_y ...),
        // To still be able to assert only the expected type of structs enter this function.
        //      (And only those of is_message are tracked, leaving easy possibility to expand to some action_type_xyz should it also need to be tracked after all.)

        // static_assert(HasRequiredFields<MessageT>::value, "Input of type MessageT cannot be safely reinterpret_cast into MessageTrackingVariables type.");

        // 'if constexpr'  would be best here, but that is C++17 while ROS2 Dashing unfortuntely targets C++14.
        constexpr bool is_tracked_message = HasRequiredFields<MessageT>::value;

        if (is_tracked_message) {
            track_message(* reinterpret_cast<const MessageTrackingVariables *>(& msg)); // yikes!
        }
    }

protected:
 // todo: if the number of utility functions gets too large, it should instead be separately inherited object so derived classes pick and choose.
    inline int64_t get_monotonic_time_64b_ns() {
        auto now = steady_clock::now();
        return duration_cast<nanoseconds>(now.time_since_epoch()).count();
    }

    inline int64_t get_unix_time_64b_ns() {
        auto now = system_clock::now(); // before C++20 this is implementation defined..
        return duration_cast<nanoseconds>(now.time_since_epoch()).count();
    }

    rclcpp::IMeasurementWriter::UniquePtr writer_;
};

} // namespace rclcpp

#endif // RCLCPP__MESSAGE_TRACKER_INTERFACE_HPP_