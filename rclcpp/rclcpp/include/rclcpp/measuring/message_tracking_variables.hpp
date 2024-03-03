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

#ifndef RCLCPP__MESSAGE_TRACKING_VARIABLES_HPP_
#define RCLCPP__MESSAGE_TRACKING_VARIABLES_HPP_

// necessary for HasRequiredFields
#include <cstddef>
#include <type_traits>
#include <typeinfo>
#include <string>

namespace rclcpp {

// This matches the extra hidden variables added to a message in rosidl_adapter/parser.py.
// Because each message is a lonely struct it was necessary to add these fields each time, 
// rather than adding some base class all structs extend from.
// (This is in part because .idl files do not seem to have inheritance in the CFG, and adding it would a lot of work. That would include generating for all supported languages.)
// A problem then, is that the input to IMessageTracker is not consistent. While the vandenhoven fields are there each time, they belong to different types.
// It would therefore be ideal to lump these variables in a struct, so that you can pass this struct into IMessageTracker and have far less complexity.
// But this is complicated with idl:
// While it is possible to have structs inside messages (ex: OtherPackage/MessageName VariableName) this introduces dependencies that must be declared in package.xml,
// And the whole intent of this feature is to work interchangably with normal ROS2, 
// such that any robot's can be run onto this version to analyse its performance without preparation or software analysis.
// Therefore in the end, it is necessary to use a template function on IMessageTracker.
// However... this function is virtual, so it cannot have a template!
// A possible way to maybe fix this is to maybe make the class itself a template, 
// however this causes the factory to be a template, which on its own is messy but not a killer, but then...
// the factory type must be specified in PublisherOptions / SubscriberOption, and on this there is no MessageT template, so the template cannot be specified then.
// To a user creating a publisher/subscriber, the MessageT is 100% known when they create the options for the messager they wish to create, 
// however, adding this feature again runs into inter-operability with regular ROS2: no prep or code analysis should be required to run anyones ROS2 code on this ROS2 mode.

// This leaves us with a trick, to have a non-virtual template function on the interface, such that any MessageT matches it, which calls a regularly-typed virtual function.
// This can be done because we know part of the memory layout of any message: our variables are first.
// To have some semblance of safety, we include here a bit of SFINAE magic,
// to statically assert any input type has a memory layout that can be used as a 'MessageTrackingVariables' struct.
// Then we can reinterpret_cast<MessageTrackingVariables *> without exploding the program at runtime.
// Nevertheless, this is not a good thing. It is a complicated hack born from the tech debt of how the variables were added to messages,
// combined with the necessity of Dependency Injection due to a nested dependency (Producer/Subscriber depends on a Tracker, Tracker depends on a DataWriter),
// It would be very messy code to work out the if/else for those choices directly in producer/subscriber, as well as needing an instance of each class to exist per Prod/Sub (memory waste).

// My best idea for fixing this mess is finding a way to have the vandenhoven variables exist as a struct in the message,
// By finding out a way to have the vandenhoven struct be defined as a special message, which is somehow built before all other messages.
// Then the generated code for this message could be #included here and it should be possible to pass that struct into the interface without compile errors.

struct MessageTrackingVariables {
    int64_t vandenhoven_timestamp;
    int64_t vandenhoven_identifier;
    int32_t vandenhoven_publisher_hash;
};

// Checks if a type has required fields with specific types.
// I think it requires reflection (which C++ does not have) to automatically evaluate 'offsetof'  and 'is_same<T, U>' for each member of the MessageTrackingVariables struct. 
template <typename T>
struct HasRequiredFields {
private:
    template <
        typename U,
        typename = typename std::enable_if_t <
            std::is_same<decltype(MessageTrackingVariables::vandenhoven_timestamp), decltype(U::vandenhoven_timestamp)>::value &&
            offsetof(MessageTrackingVariables, vandenhoven_timestamp) == offsetof(U, vandenhoven_timestamp) &&
            std::is_same<decltype(MessageTrackingVariables::vandenhoven_identifier), decltype(U::vandenhoven_identifier)>::value &&
            offsetof(MessageTrackingVariables, vandenhoven_identifier) == offsetof(U, vandenhoven_identifier) &&
            std::is_same<decltype(MessageTrackingVariables::vandenhoven_publisher_hash), decltype(U::vandenhoven_publisher_hash)>::value &&
            offsetof(MessageTrackingVariables, vandenhoven_publisher_hash) == offsetof(U, vandenhoven_publisher_hash)
        >
    >
    static std::true_type test(int);

    template <typename U>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

} // namespace rclcpp

#endif // RCLCPP__MESSAGE_TRACKING_VARIABLES_HPP_