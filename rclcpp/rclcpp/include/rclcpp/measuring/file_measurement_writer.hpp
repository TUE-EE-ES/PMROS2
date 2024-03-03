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

#ifndef RCLCPP__FILE_MEASUREMENT_WRITER_HPP_
#define RCLCPP__FILE_MEASUREMENT_WRITER_HPP_

#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/measuring/message_tracker_host_info.hpp"
#include <vector>
#include <fstream>

namespace rclcpp {

class FileMeasurementWriter : public IMeasurementWriter {
public:
    FileMeasurementWriter() = delete;
    FileMeasurementWriter(const rclcpp::MessageTrackerHostInfo & host_info);

    uint32_t register_measurement_class(const std::string& name, const std::vector<std::string>& columns = {}) override;

    void record_latency(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher, int64_t arrival_time) override;

    void record_arrival(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher) override;

    void record_activation_jitter(uint32_t, int64_t) override;

private:
    std::vector<std::ofstream> measurement_classes_;

    std::string host_full_name_;
};

} // namespace rclcpp

#endif // RCLCPP__FILE_MEASUREMENT_WRITER_HPP_