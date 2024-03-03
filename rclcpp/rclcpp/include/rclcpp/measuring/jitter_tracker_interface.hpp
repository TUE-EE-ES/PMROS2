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

#ifndef RCLCPP__JITTER_TRACKER_INTERFACE_HPP_
#define RCLCPP__JITTER_TRACKER_INTERFACE_HPP_

#include <memory>
#include <chrono>
#include "rclcpp/macros.hpp"
#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/clock.hpp"

namespace rclcpp {

class IJitterTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(IJitterTracker)

    virtual ~IJitterTracker() {}

    IJitterTracker() = delete;
    IJitterTracker(rclcpp::IMeasurementWriter::UniquePtr writer) : writer_(std::move(writer)) {}

    virtual void track_jitter(const Clock::SharedPtr& clock, int64_t time_since_last_activate, int64_t intended_activation_time) = 0;

protected:
    inline int64_t get_unix_time_64b_ns() {
        auto now = std::chrono::system_clock::now(); // before C++20 this is implementation defined..
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }

    rclcpp::IMeasurementWriter::UniquePtr writer_;
};

} // namespace rclcpp

#endif // RCLCPP__JITTER_TRACKER_INTERFACE_HPP_