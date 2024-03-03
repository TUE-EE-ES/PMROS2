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

#include "rclcpp/measuring/influxdb_measurement_writer.hpp"
#include "rclcpp/measuring/simpletimer.hpp"

namespace rclcpp {


InfluxDBMeasurementWriter::InfluxDBMeasurementWriter(const rclcpp::MessageTrackerHostInfo & host_info) {

    host_fully_qualified_name_ = std::string(host_info.node_namespace) + std::string(host_info.node_name); 
    host_topic_ = host_info.topic_name;

    // todo: These hardcoded variables could be read from a config file somewhere.
    // Furthermore, it should be a static struct that is assigned from a static function call.
    // Unless it is necessary to support multiple, in which case the default-config stuff needs to be extended to allow a file path to influx config files.
    static std::string influx_ip = "127.0.0.1";
    static uint32_t influx_port = 8086;
    static std::string influx_org = "";
    static std::string influx_token = "";
    static std::string influx_bucket = "";

    // 'false' -> do not inspect HTTP post responses. This speeds up posting by a factor ~10,000
    // We do not care about responses, because the cost is multiple milliseconds.
    server_info_ = std::make_unique<influxdb_cpp::server_info>(influx_ip, influx_port, influx_org, influx_token, influx_bucket, false);
    influx_socket_ = std::make_unique<influxdb_cpp::influx_socket>(*server_info_);

    // Similar to the hardcoded server_info, it would be real nice if this was user-configurable.
    // Possibly the heuristic should be integrated into server_info even, such that the library manages it instead of the writer.
    // It would also centralize configurables of influxdb-writer variables to one spot (namely that of server_info)

    //                                                    0 sec,  0KB first, after first upload 15 sec 64KB: see bottom of code for WHY.
    heuristic_ = std::make_unique<InfluxDBUploadHeuristic>(00000, 00000);
    heuristic_->set_last_upload_time(); 
}

uint32_t InfluxDBMeasurementWriter::register_measurement_class(const std::string& name, const std::vector<std::string>&) {
    measurements_.emplace_back(name);

    return static_cast<uint32_t>(measurements_.size() - 1);
}

void InfluxDBMeasurementWriter::record_latency(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher, int64_t arrival_time) {
    const auto& measurement = measurements_[key];
    
    // internally the chained calls to builder change its type to present a CFG-like availability of function calls.
    // The returned object after each call is still just the builder, so we take the result of this chain by reference.
    // The reference is thus to the same object as influx_builder_, except its type is such that post_http() can be called on it.
    auto& ready_to_post_builder = influx_builder_
        .meas(measurement)
        .tag("publisher", publisher)
        .tag("topic", host_topic_)
        .tag("node_full_name", host_fully_qualified_name_)
        .field("sent_time", msg.vandenhoven_timestamp)
        .field("arrive_time", arrival_time)
        .field("msg_id", msg.vandenhoven_identifier)
        .timestamp(output_timestamp());

    maybe_upload(ready_to_post_builder);
}

void InfluxDBMeasurementWriter::record_arrival(uint32_t key, const MessageTrackingVariables& msg, const hex_char_array_t& publisher) {
    const auto& measurement = measurements_[key];
    auto& ready_to_post_builder = influx_builder_
        .meas(measurement)
        .tag("publisher", publisher)
        .tag("topic", host_topic_)
        .tag("node_full_name", host_fully_qualified_name_)
        .field("msg_id", msg.vandenhoven_identifier)
        .timestamp(output_timestamp());

    maybe_upload(ready_to_post_builder);
}

void InfluxDBMeasurementWriter::record_activation_jitter(uint32_t key, int64_t activation_jitter) {
    const auto& measurement = measurements_[key];
    auto& ready_to_post_builder = influx_builder_
        .meas(measurement)
        .tag("timer", host_fully_qualified_name_)
        .field("activation_jitter", activation_jitter)
        .timestamp(output_timestamp());

    maybe_upload(ready_to_post_builder);
}

void InfluxDBMeasurementWriter::maybe_upload(influxdb_cpp::detail::ts_caller& uploadable) {
    if (heuristic_->should_upload(influx_builder_)) {
        // note alternatively, inspect return code of post and only update upload time on success. 
        // Because we dont await the HTTP response, its not very helpful here, I think.
        // It could also lead to spamming uploads if there is an error, which degrades performance.
        heuristic_->set_last_upload_time();
        uploadable.post_http(*server_info_, *influx_socket_);
        influx_builder_.clear();

        heuristic_->updateHeuristic( // see bottom of file as to why this is updated after the first upload.
            std::make_pair(true, 64000),
            std::make_pair(true, 15000)
        );
    }
}

InfluxDBMeasurementWriter::~InfluxDBMeasurementWriter() {
    // We are terminating?! OK let's try to upload the last batch to avoid data loss. Especially helpful if it's due to a crash!!
    if (influx_builder_.getBufferSize() > 0) {
        std::cerr << "[INFLUXDB_MEASUREMENT_WRITER]" << host_fully_qualified_name_ << ": " << host_topic_ << " -- Uploading left-over measurements before shutdown...";
        server_info_->await_post_response_ = true; // complete the request in full, do not prematurely close the socket.
        auto ret_code = static_cast<influxdb_cpp::detail::ts_caller *>(& influx_builder_)->post_http(*server_info_, *influx_socket_);

        std::cerr << (ret_code == 0 ? std::string("Success.\n") : std::string("Failure (" + std::to_string(ret_code) + ")\n"));
    }
}

}

// Why first 0 sec and 0kb, DO NOT increase this or you will lose days debugging:
/*
    If you send some 17k bytes over the socket (writev in influxdb.hpp) to influxDB, your pipe will be broken.
    There is no HTTP response,
    There is no error logging on their side.
    It just breaks, and even in the Go source code I cannot find why (Granted, I do not know Go  very well).

    In fact, normally HTTP requests are logged if you set the log level of influxDB to debug.
    But this type of " too large " request does not even get logged, like it never ever arrives.

    The only way around I could eventually find is first sending a smaller amount, then I can send 64k+ afterwards no problem.
    This sounds like a TCP windowing thing, for my lackluster understanding of the TCP protocol.
    However, I was under the impression that the kernel handled the TCP protocol after I call writev.
    
    This is backed up by the fact that the first call to writeV is capped at 21888 bytes,
    (Even though the send buffer is 2.6MB as confirmed by getsockopt).

    But subsequent calls, which I tried up to 64K, do get sent with writev in one go.
    This indicates to me that writev is already handling the amount of bytes written as per the TCP protocol,
    and yet if I use all the resources the kernel allows, my program breaks.

    I have no good guesses as to why that is. My best idea is that the window is incorrectly communicated or assumed,
    and the receiver TCP protocol breaks off connection after receiving more than it wanted.
    
    It is thus also unknown what amount is the maximum after the first send. It is more than 64k, but I don't know the actual limit.

*/
