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

#include "rclcpp/measuring/file_measurement_writer.hpp"
#include <fstream>
#include <iostream>
#include <chrono>

using rclcpp::FileMeasurementWriter;

// Writes files in the directory that the process is run from, 
// The files are in CSV format representing time, value pairs.
// Files are named after topic::namespace::hostname_[key].
FileMeasurementWriter::FileMeasurementWriter(const rclcpp::MessageTrackerHostInfo & host_info) {
    host_full_name_ = host_info.topic_name;
    host_full_name_ += "->";
    
    // [namespace]::[nodename] or [nodename] if namespace is 'empty' (just the global ns which is "/").
    auto ns_string = std::string(host_info.node_namespace).substr(1);
    host_full_name_ += ns_string.size() > 0 ? ns_string + "::" : "";
    
    host_full_name_ += host_info.node_name;
}

uint32_t FileMeasurementWriter::register_measurement_class(const std::string& name, const std::vector<std::string>& columns) {
    std::ofstream stream;
    // todo: You can set the buffer here using `rdbuf()->pubsetbuf(buffer, bufferSize);`.
    // This MUST be done before file opening! It will do nothing otherwise.

    // Open the file, named '{topic}->{node_full_name}_[measurement_name]'. 
    // Todo: think of a method to provide an absolute path. Could be in a factory of measurementWriter.
    // Todo: consider not opening the stream until a record-like function is called for it. You create a few garbage empty files this way (parameter events)
    // Note: The file naming scheme in this class is not compatible with windows. Too bad!
    std::string fileName = host_full_name_ + "_[" + name + "].txt";
    stream.open(fileName);
    std::cout << "OPENED STREAM FOR: '" << fileName << "'\n";

    // CSV header, let's hope there's no commas in the colum names.
    stream << "unix_time";
    for (const auto& v : columns) { 
        stream << ',' << v;
    }
    stream << "\n";

    // Register the filestream...
    measurement_classes_.emplace_back(std::move(stream));

    return static_cast<uint32_t>(measurement_classes_.size() - 1);
}

void FileMeasurementWriter::record_latency(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher, int64_t arrival_time) {
    std::ofstream& stream = measurement_classes_[key];
    stream << output_timestamp() << ",";
    stream << publisher << ",";
    stream << msg.vandenhoven_timestamp << "," << arrival_time << "\n";
}

void FileMeasurementWriter::record_arrival(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher) {
    std::ofstream& stream = measurement_classes_[key];

    stream << output_timestamp() << "," << publisher << "," << msg.vandenhoven_timestamp << "\n";
}

void FileMeasurementWriter::record_activation_jitter(uint32_t key, int64_t activation_jitter) {
    std::ofstream& stream = measurement_classes_[key];

    stream << output_timestamp() << "," << activation_jitter << "\n";
}