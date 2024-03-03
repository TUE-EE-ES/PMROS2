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

#include <iostream>

#include "rclcpp/measuring/tracing_publisher_message_tracker.hpp"

using rclcpp::TracingPublisherMessageTracker;

// The tracing publisher message tracker offers extra features over the normal publisher message tracker.
// Namely, it better supports 'tracing' latency of messages as they pass through multiple publishings. e.g. A -> B -> C -> D, what is the latency of A->D ?
// To do this, one must get the difference of the send_time of A->B, and the receive time of C->D.
// This can be done if both are written by MeasurementWriters, namely by doing an inner join on msg_id.
// However, this inner join can only be correct if the ID is preserved. That is, msg with ID 1 of A->B corresponds to msg with ID 1 of C->D.
// This tracker facilitates this by only writing IDs if the incoming ID is zero. That way IDs are forwarded if they are assigned already.
// On the user side, traces can be done trivially if a message just gets forwarded, but the ID must be manually assigned if a new message is created from a message-receiving callback.

//Tracing that is supported:
// Trivial things, A->B->C
// Stacking traces, A->B->C while also managing A->B->C->D

//Tracing that is not supported:
// Interfering traces, A->B->C while there exists an A'->B. 
// This would cause multiple messages with the same ID to be created in B. (2x for every ID, once from A once from A'. 
// Even if A and A' publish to different topics, B->C would drop this info)
// This can be avoided on a user level by keeping the topics for A and A' separate through the entire trace: A-(a)->B-(b)->C and A'-(a')->B-(b')->C.
// Alternatively we can look into supporting this by adding a 32bit trace_origin variable to messagetrackingvariables, such that distinguishing can be made on the inner join query.
// However, doing this still would not support stacked tracing with interference, that is: A->B->C->D and B->C->D when there exists B'->C. 
// A trace origin is needed to filter B', but the trace origin of A gets overwritten by B when publishing B->C.
// Thus to support this on the client library level, A std::vector type variable is needed, which takes 4+L bytes, where L is the number of stacked traces the user is using.

void TracingPublisherMessageTracker::track_message(const MessageTrackingVariables & msg) {
    auto & message = const_cast<MessageTrackingVariables &>(msg);

    message.vandenhoven_timestamp = get_monotonic_time_64b_ns();

    if (message.vandenhoven_identifier == 0) {
        message.vandenhoven_identifier = current_msg_id;
        current_msg_id++;
    }

    message.vandenhoven_publisher_hash = host_hash_;
}