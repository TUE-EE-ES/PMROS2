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

#ifndef RCLCPP__PRINT_MEASUREMENT_WRITER_HPP_
#define RCLCPP__PRINT_MEASUREMENT_WRITER_HPP_

#include "rclcpp/measuring/measurement_writer_interface.hpp"

namespace rclcpp {

class PrintMeasurementWriter : public IMeasurementWriter {
public:
    uint32_t register_measurement_class(const std::string& name, const std::vector<std::string>& columns = {}) override;

    void record_latency(uint32_t key, const MessageTrackingVariables& meas, const hex_char_array_t& publisher, int64_t arrival_time) override;

    void record_arrival(uint32_t key, const MessageTrackingVariables& meas, const hex_char_array_t& publisher) override;

    void record_activation_jitter(uint32_t, int64_t) override;
private:
    std::vector<std::string> measurement_classes_;
};

} // namespace rclcpp

#endif // RCLCPP__PRINT_MEASUREMENT_WRITER_HPP_