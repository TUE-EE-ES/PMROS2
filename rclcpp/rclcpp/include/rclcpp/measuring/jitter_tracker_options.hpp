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

#ifndef RCLCPP__JITTER_TRACKER_OPTIONS_HPP_
#define RCLCPP__JITTER_TRACKER_OPTIONS_HPP_

#include "rclcpp/measuring/message_tracker_options.hpp" // MeasurementWriterEnum

namespace rclcpp {


enum class JitterTrackerEnum : uint8_t {
    ACTIVATION_JITTER,
    NONE
};

struct JitterTrackerOptions {

    JitterTrackerOptions() = delete;

    JitterTrackerOptions(JitterTrackerEnum jte, MeasurementWriterEnum mwe) 
    : result_writer_option(mwe)
    , jitter_tracker_option(jte)
    {}

    MeasurementWriterEnum result_writer_option;
    JitterTrackerEnum jitter_tracker_option;
};

} // namespace rclcpp

#endif // RCLCPP__JITTER_TRACKER_OPTIONS_HPP_