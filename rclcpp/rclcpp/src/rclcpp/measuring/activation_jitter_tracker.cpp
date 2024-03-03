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

#include "rclcpp/measuring/activation_jitter_tracker.hpp"

#include <iostream>

namespace rclcpp {
ActivationJitterTracker::ActivationJitterTracker(rclcpp::IMeasurementWriter::UniquePtr writer) : IJitterTracker(std::move(writer)) {
    activation_jitter_key_ = writer_->register_measurement_class("timer_activation_jitter", {"activation_jitter"});
}

void ActivationJitterTracker::track_jitter(const Clock::SharedPtr& clock, [[maybe_unused]] int64_t time_since_last_activate, int64_t intended_activation_time) {
    auto now = clock->now().nanoseconds(); // this way, the time should be obtained from the same clock that rcl used to get the 2 nanosecond values in the function parameter.
    // todo: This should not be calculated here. These two distinct values (current and intended time) should both be passed to the writer interface.
    // Doing this requires changing the writer interface and itsimplementations.
    auto activation_jitter = now - intended_activation_time; // rcl_time_point_t -> int64_t at the time of writing.

    writer_->use_timestamp(get_unix_time_64b_ns()); // do not use clock param for this! it is not necessarily unix, most likely it is std::chrono::steady_clock (monotonic).
    writer_->record_activation_jitter(activation_jitter_key_, static_cast<int64_t>(activation_jitter));
}

}
