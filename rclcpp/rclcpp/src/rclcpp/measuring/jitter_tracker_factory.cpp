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

#include "rclcpp/measuring/jitter_tracker_factory.hpp"
#include "rclcpp/measuring/message_tracker_host_info.hpp" //todo removeme

namespace rclcpp {

JitterTrackerFactory::UniquePtr JitterTrackerFactory::make(JitterTrackerEnum jte) {
    switch (jte) {
        case JitterTrackerEnum::ACTIVATION_JITTER:
            return std::make_unique<ActivationJitterTrackerFactory>();
        case JitterTrackerEnum::NONE:
            return std::make_unique<DummyJitterTrackerFactory>();
        default:
            throw std::invalid_argument( "Unrecognized enum value for JitterTrackerEnum." );
    }
}

IMeasurementWriter::UniquePtr JitterTrackerFactory::create_result_writer(MeasurementWriterEnum mwe, const TimerOptions& opts) const {
    /* 
    Fake a topic, name, and namespace.
    This input struct being called "MessageTrackerHostInfo" is a problem that should be fixed. The input is no longer constrained to "Message Trackers",
    nor are the fields accurate (we do not have a 'topic' or a 'namespace' here, just a name.)
    Worse, emtpy string in the namespace would be UB for file measurement writer, so we fake a global namespace '/'.
    And then we need to offset the name (always starting with '/') by 1 char on the pointer level to avoid double / in the output.

    This struct should be removed and either become something more abstract, like a vector of pairs that represent constant meta-information for the tracker, and their values...
    Or we just pass in an rcl_node_t * directly, and let users fill in meta-info from there.
    */
    auto host_information = MessageTrackerHostInfo("__rclcpp_timer_activation_jitter", opts.timer_name.c_str() + 1, "/");
    switch(mwe) {
        case MeasurementWriterEnum::INFLUXDB:
            return std::make_unique<InfluxDBMeasurementWriter>(host_information);
        case MeasurementWriterEnum::FILE:
            return std::make_unique<FileMeasurementWriter>(host_information);
        case MeasurementWriterEnum::PRINT:
            return std::make_unique<PrintMeasurementWriter>();
        case MeasurementWriterEnum::NONE:
            return std::make_unique<DummyMeasurementWriter>();
        default:
            throw std::invalid_argument( "Unrecognized enum value for MeasurementWriterEnum" );
    }
}

IJitterTracker::UniquePtr
ActivationJitterTrackerFactory::create_jitter_tracker(MeasurementWriterEnum mwe, const TimerOptions& timer_opts) const {
    auto writer = create_result_writer(mwe, timer_opts);
    return std::make_unique<ActivationJitterTracker>(std::move(writer));
}

IJitterTracker::UniquePtr
DummyJitterTrackerFactory::create_jitter_tracker(MeasurementWriterEnum mwe, const TimerOptions& timer_opts) const {
    auto writer = create_result_writer(mwe, timer_opts);
    return std::make_unique<DummyJitterTracker>(std::move(writer));
}

} // namespace rclcpp