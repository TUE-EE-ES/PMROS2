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

#ifndef RCLCPP__MESSAGE_TRACKER_FACTORY_HPP_
#define RCLCPP__MESSAGE_TRACKER_FACTORY_HPP_

#include <memory>

#include "rclcpp/measuring/message_tracker_interface.hpp"
#include "rclcpp/measuring/publisher_message_tracker.hpp"
#include "rclcpp/measuring/tracing_publisher_message_tracker.hpp"
#include "rclcpp/measuring/subscriber_message_tracker.hpp"
#include "rclcpp/measuring/dummy_message_tracker.hpp"

#include "rclcpp/measuring/measurement_writer_interface.hpp"
#include "rclcpp/measuring/dummy_measurement_writer.hpp"
#include "rclcpp/measuring/print_measurement_writer.hpp"
#include "rclcpp/measuring/file_measurement_writer.hpp"
#include "rclcpp/measuring/influxdb_measurement_writer.hpp"

#include "rclcpp/macros.hpp"
#include "rclcpp/measuring/message_tracker_options.hpp"
#include "rclcpp/measuring/message_tracker_host_info.hpp"

namespace rclcpp {

struct MessageTrackerFactory {
    RCLCPP_SMART_PTR_DEFINITIONS(MessageTrackerFactory)

    virtual rclcpp::IMessageTracker::UniquePtr create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const = 0;

    static MessageTrackerFactory::UniquePtr make(MessageTrackerEnum mte);

    rclcpp::IMeasurementWriter::UniquePtr create_result_writer(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const;
};

struct PublisherMessageTrackerFactory : public MessageTrackerFactory {
    rclcpp::IMessageTracker::UniquePtr create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const override;
};

struct TracingPublisherMessageTrackerFactory : public MessageTrackerFactory {
    rclcpp::IMessageTracker::UniquePtr create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const override;
};

struct SubscriberMessageTrackerFactory : public MessageTrackerFactory {
    rclcpp::IMessageTracker::UniquePtr create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const override;
};

struct DummyMessageTrackerFactory : public MessageTrackerFactory {
    rclcpp::IMessageTracker::UniquePtr create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const override;
};

} // namespace rclcpp

#endif // RCLCPP__MESSAGE_TRACKER_FACTORY_HPP_