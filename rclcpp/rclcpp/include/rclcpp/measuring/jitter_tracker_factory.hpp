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

#ifndef RCLCPP__JITTER_TRACKER_FACTORY_HPP_
#define RCLCPP__JITTER_TRACKER_FACTORY_HPP_

#include <memory>

#include "rclcpp/measuring/jitter_tracker_interface.hpp"
#include "rclcpp/measuring/dummy_jitter_tracker.hpp"
#include "rclcpp/measuring/activation_jitter_tracker.hpp"

#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/measuring/influxdb_measurement_writer.hpp"
#include "rclcpp/measuring/dummy_measurement_writer.hpp"
#include "rclcpp/measuring/print_measurement_writer.hpp"
#include "rclcpp/measuring/file_measurement_writer.hpp"

#include "rclcpp/macros.hpp"
#include "rclcpp/measuring/jitter_tracker_options.hpp"
#include "rclcpp//timer_options.hpp"

namespace rclcpp {

struct JitterTrackerFactory {
    RCLCPP_SMART_PTR_DEFINITIONS(JitterTrackerFactory)

    virtual rclcpp::IJitterTracker::UniquePtr create_jitter_tracker(MeasurementWriterEnum mwe, const TimerOptions& timer_opts) const = 0;

    static JitterTrackerFactory::UniquePtr make(JitterTrackerEnum jte);

    // Note: this is duplicated from message_tracker_factory. Writer should gets its own factory if more of these factories get made.
    rclcpp::IMeasurementWriter::UniquePtr create_result_writer(MeasurementWriterEnum mwe, const TimerOptions& opts) const;
};

struct ActivationJitterTrackerFactory : public JitterTrackerFactory {
    rclcpp::IJitterTracker::UniquePtr create_jitter_tracker(MeasurementWriterEnum mwe, const TimerOptions& timer_opts) const override;
};

struct DummyJitterTrackerFactory : public JitterTrackerFactory {
    rclcpp::IJitterTracker::UniquePtr create_jitter_tracker(MeasurementWriterEnum mwe, const TimerOptions& timer_opts) const override;
};

} // namespace rclcpp

#endif // RCLCPP__JITTER_TRACKER_FACTORY_HPP_