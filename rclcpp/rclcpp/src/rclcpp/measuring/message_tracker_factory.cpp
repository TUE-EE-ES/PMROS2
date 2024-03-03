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

#include "rclcpp/measuring/message_tracker_factory.hpp"

using rclcpp::MessageTrackerFactory;
using rclcpp::PublisherMessageTrackerFactory;
using rclcpp::SubscriberMessageTrackerFactory;
using rclcpp::DummyMessageTrackerFactory;
using rclcpp::PublisherMessageTracker;
using rclcpp::TracingPublisherMessageTrackerFactory;
using rclcpp::SubscriberMessageTracker;
using rclcpp::DummyMessageTracker;
using rclcpp::IMessageTracker;
using rclcpp::MessageTrackerEnum;
using rclcpp::IMeasurementWriter;
using rclcpp::FileMeasurementWriter;
using rclcpp::PrintMeasurementWriter;
using rclcpp::DummyMeasurementWriter;
using rclcpp::InfluxDBMeasurementWriter;

MessageTrackerFactory::UniquePtr MessageTrackerFactory::make(MessageTrackerEnum mte) {
    switch (mte) {
        case MessageTrackerEnum::SUBSCRIBER:
            return std::make_unique<rclcpp::SubscriberMessageTrackerFactory>();
        case MessageTrackerEnum::PUBLISHER:
            return std::make_unique<rclcpp::PublisherMessageTrackerFactory>();
        case MessageTrackerEnum::TRACING_PUBLISHER:
            return std::make_unique<rclcpp::TracingPublisherMessageTrackerFactory>();
        case MessageTrackerEnum::NONE:
            return std::make_unique<rclcpp::DummyMessageTrackerFactory>();
        default:
            throw std::invalid_argument( "Unrecognized enum value for MessageTrackerEnum." );
    }
}

IMeasurementWriter::UniquePtr MessageTrackerFactory::create_result_writer(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const {
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

IMessageTracker::UniquePtr
PublisherMessageTrackerFactory::create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const {
    auto writer = create_result_writer(mwe, host_information);
    uint32_t publisher_full_name_hash = host_information.hash_full_node_name();
    return std::make_unique<PublisherMessageTracker>(std::move(writer), publisher_full_name_hash);
}

IMessageTracker::UniquePtr
TracingPublisherMessageTrackerFactory::create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const {
    auto writer = create_result_writer(mwe, host_information);
    uint32_t publisher_full_name_hash = host_information.hash_full_node_name();
    return std::make_unique<TracingPublisherMessageTracker>(std::move(writer), publisher_full_name_hash);
}

IMessageTracker::UniquePtr
SubscriberMessageTrackerFactory::create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const {
    auto writer = create_result_writer(mwe, host_information);
    return std::make_unique<SubscriberMessageTracker>(std::move(writer));
}

IMessageTracker::UniquePtr
DummyMessageTrackerFactory::create_message_tracker(MeasurementWriterEnum mwe, const MessageTrackerHostInfo & host_information) const {
    auto writer = create_result_writer(mwe, host_information);
    return std::make_unique<DummyMessageTracker>(std::move(writer));
}