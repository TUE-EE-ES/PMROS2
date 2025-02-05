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

#ifndef RCLCPP__ACTIVATION_JITTER_TRACKER_HPP_
#define RCLCPP__ACTIVATION_JITTER_TRACKER_HPP_

#include <memory>
#include "rclcpp/measuring/jitter_tracker_interface.hpp"
#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/macros.hpp"

namespace rclcpp {

class ActivationJitterTracker : public IJitterTracker {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(ActivationJitterTracker)

    ActivationJitterTracker(rclcpp::IMeasurementWriter::UniquePtr writer);

    void track_jitter(const Clock::SharedPtr& clock, int64_t time_since_last_activate, int64_t intended_activation_time) override;
private:
    uint32_t activation_jitter_key_;
};

}

#endif