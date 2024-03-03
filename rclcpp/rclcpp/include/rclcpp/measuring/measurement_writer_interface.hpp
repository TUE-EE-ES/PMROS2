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

#ifndef RCLCPP__MEASUREMENT_WRITER_INTERFACE_HPP_
#define RCLCPP__MEASUREMENT_WRITER_INTERFACE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "rclcpp/macros.hpp"
#include "rclcpp/measuring/message_tracking_variables.hpp"
#include "rclcpp/measuring/hash_to_chars.hpp"

namespace rclcpp {

class IMeasurementWriter {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(IMeasurementWriter)

    virtual ~IMeasurementWriter() {}

    virtual uint32_t register_measurement_class(const std::string& name, const std::vector<std::string>& columns = {}) = 0;

    virtual void record_latency(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher, int64_t arrival_time) = 0;

    virtual void record_arrival(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher) = 0;

    virtual void record_activation_jitter(uint32_t key, int64_t activation_jitter) = 0;

    // Useful for keeping consistent timestamping when writing multiple measurements in sequence.
    inline void use_timestamp(int64_t timestamp) { output_timestamp_ = timestamp; }

protected:
    // Implementing writers should use this in writing timestamps, as opposed to getting the timestamp themselves. Let the dependent of the writer decide the timestamp and use that.
    inline int64_t output_timestamp () { return output_timestamp_; }

private:
    int64_t output_timestamp_ = 0; // preferable over something smart like std::chrono::stead_clock::now(), because it will be obvious an implementing writer has a bug this way.
};

} // namespace rclcpp

#endif // RCLCPP__MEASUREMENT_WRITER_INTERFACE_HPP_