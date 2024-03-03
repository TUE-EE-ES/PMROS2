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

#ifndef MESSAGE_TRACKER_OPTIONS_HPP_
#define MESSAGE_TRACKER_OPTIONS_HPP_

#include <stdexcept>
#include <memory>
#include "rcl/message_tracker_options.h"

namespace rclcpp {

enum class MessageTrackerEnum : uint8_t {
    SUBSCRIBER,
    PUBLISHER,
    TRACING_PUBLISHER,
    NONE
};

enum class MeasurementWriterEnum : uint8_t {
    INFLUXDB,
    FILE,
    PRINT,
    NONE
};

// The enum class gets converted to an int to enter a C library, and converting it back is handled here.
inline MeasurementWriterEnum intToMWE(uint8_t x) {
    switch(x) {
        case static_cast<uint8_t>(MeasurementWriterEnum::INFLUXDB): return MeasurementWriterEnum::INFLUXDB;
        case static_cast<uint8_t>(MeasurementWriterEnum::FILE): return MeasurementWriterEnum::FILE;
        case static_cast<uint8_t>(MeasurementWriterEnum::PRINT): return MeasurementWriterEnum::PRINT;
        case static_cast<uint8_t>(MeasurementWriterEnum::NONE): return MeasurementWriterEnum::NONE;
        default: throw std::invalid_argument("Unknown input for intToMWE: " + std::to_string(x));
    }
}

inline MessageTrackerEnum intToMTE(uint8_t x) {
    switch(x) {
        case static_cast<uint8_t>(MessageTrackerEnum::SUBSCRIBER): return MessageTrackerEnum::SUBSCRIBER;
        case static_cast<uint8_t>(MessageTrackerEnum::PUBLISHER): return MessageTrackerEnum::PUBLISHER;
        case static_cast<uint8_t>(MessageTrackerEnum::TRACING_PUBLISHER): return MessageTrackerEnum::TRACING_PUBLISHER;
        case static_cast<uint8_t>(MessageTrackerEnum::NONE): return MessageTrackerEnum::NONE;
        default: throw std::invalid_argument("Unknown input for intToMTE: " + std::to_string(x));
    }
}

struct MessageTrackerOptions {

    MessageTrackerOptions() = delete; // A user _must_ provide values for this struct.

    MessageTrackerOptions(const rcl_message_tracker_options_t & c_type) {
        tracker_result_writing_option = intToMWE(c_type.measurement_writer_opt);
        tracker_option = intToMTE(c_type.message_tracker_opt);
    }

    MessageTrackerOptions(MessageTrackerEnum mto, MeasurementWriterEnum mwo) {
        tracker_option = mto;

        tracker_result_writing_option = mwo;
    }

    rcl_message_tracker_options_t to_rcl_message_tracker_options() const {
        rcl_message_tracker_options_t result;

        result.message_tracker_opt = static_cast<uint8_t>(tracker_option);
        result.measurement_writer_opt = static_cast<uint8_t>(tracker_result_writing_option);

        return result;
    }

    MeasurementWriterEnum tracker_result_writing_option;

    MessageTrackerEnum tracker_option;
};

} // namespace rclcpp

#endif // MESSAGE_TRACKER_OPTIONS_HPP_