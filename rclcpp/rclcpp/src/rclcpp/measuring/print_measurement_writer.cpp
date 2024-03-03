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

#include "rclcpp/measuring/print_measurement_writer.hpp"
#include <iostream>

using rclcpp::PrintMeasurementWriter;

uint32_t PrintMeasurementWriter::register_measurement_class(const std::string& name, const std::vector<std::string>&) {
    measurement_classes_.emplace_back(name);

    return static_cast<uint32_t>(measurement_classes_.size() - 1);
}

void PrintMeasurementWriter::record_latency(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher, int64_t arrival_time) {
    const auto& name = measurement_classes_[key];
    std::cout << "[ " << publisher << "->" << name << " ]: (" << msg.vandenhoven_timestamp << ", " << arrival_time << ")\n";
}

void PrintMeasurementWriter::record_arrival(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher) {
    const auto& name = measurement_classes_[key];
    std::cout << "[ " << publisher << "->" << name << " ]: (" << msg.vandenhoven_identifier << ")\n";
}

void PrintMeasurementWriter::record_activation_jitter(uint32_t key, int64_t activation_jitter) {
    const auto& name = measurement_classes_[key];
    std::cout << "[ " << name << " ]: (" << activation_jitter << ")\n";
}