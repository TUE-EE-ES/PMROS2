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

#ifndef RCLCPP__INFLUXDB_MEASUREMENT_WRITER_HPP_
#define RCLCPP__INFLUXDB_MEASUREMENT_WRITER_HPP_

#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/measuring/message_tracker_host_info.hpp"

#include "rclcpp/influxdb/influxdb.hpp"

#include <memory>
#include <chrono>

namespace rclcpp {

class InfluxDBUploadHeuristic; // fwd declare because writer has this as a member.

class InfluxDBMeasurementWriter : public IMeasurementWriter {
public:
    InfluxDBMeasurementWriter() = delete;
    InfluxDBMeasurementWriter(const rclcpp::MessageTrackerHostInfo & host_info);
    ~InfluxDBMeasurementWriter();

    uint32_t register_measurement_class(const std::string& name, const std::vector<std::string>& columns = {}) override;

    void record_latency(uint32_t key, const MessageTrackingVariables& meas, const hex_char_array_t& publisher, int64_t arrival_time) override;

    void record_arrival(uint32_t key, const MessageTrackingVariables& meas, const hex_char_array_t& publisher) override;

    void record_activation_jitter(uint32_t, int64_t) override;

private:

    void maybe_upload(influxdb_cpp::detail::ts_caller& uploadable);

    std::unique_ptr<influxdb_cpp::server_info> server_info_;
    std::unique_ptr<influxdb_cpp::influx_socket> influx_socket_;
    std::unique_ptr<InfluxDBUploadHeuristic> heuristic_;

    influxdb_cpp::builder influx_builder_;

    // std::string host_name_;
    // std::string host_namespace_;
    std::string host_fully_qualified_name_;
    std::string host_topic_;

    std::vector<std::string> measurements_;
};

// personal note: If more of these 'heuristics' for uploading/writing are made, they should (1) get their own files (2) implement a bool check_heuristic() interface.
/**
 * Exposes a should_upload function.
 * Decides based on internal parameters whether a provided upload candidate (influxdb_cpp::builder object) should be uploaded.
 * The internal parameters can be specified in the constructor, namely:
 * 1) time since last upload
 * 2) size of the upload.
 *
 * For example, locally hosted influx servers might tolerate a larger upload size due to loopback.
 * Upload frequency is mostly up to user preference.
 */
class InfluxDBUploadHeuristic {
public:
    InfluxDBUploadHeuristic(uint32_t timeMS, uint32_t sizeBytes)
        : lastUploadTime_(0)
        , sizeConstraintBytes_(sizeBytes)
        , timeConstraintMS_(timeMS)
        {}

    // cannot be const-ref, getBufferSize uses seekp and tellp.
    inline bool should_upload(influxdb_cpp::builder& upload_candidate) {
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch() - lastUploadTime_).count();

        if (timeDiff > timeConstraintMS_) {
            return true;
        }

        if (upload_candidate.getBufferSize() > sizeConstraintBytes_) {
            return true;
        }

        return false;
    }

    inline void set_last_upload_time() {
        auto now = std::chrono::steady_clock::now();
        lastUploadTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    }

    inline void updateHeuristic(
        std::pair<bool, uint32_t> maybeNewSizeConstraint,
        std::pair<bool, uint32_t> maybeNewTimeConstraint
    ) {
        if (maybeNewSizeConstraint.first) {
            sizeConstraintBytes_ = maybeNewSizeConstraint.second;
        }
        if (maybeNewTimeConstraint.first) {
            timeConstraintMS_ = maybeNewTimeConstraint.second;
        }
    }
    
private:
    std::chrono::milliseconds lastUploadTime_;
    uint32_t sizeConstraintBytes_;
    uint32_t timeConstraintMS_;
};

}

#endif // RCLCPP__INFLUXDB_MEASUREMENT_WRITER_HPP_