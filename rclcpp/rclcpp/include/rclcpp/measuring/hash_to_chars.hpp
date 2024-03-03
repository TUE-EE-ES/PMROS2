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

#ifndef RCLCPP__HASH_TO_CHARS_HPP_
#define RCLCPP__HASH_TO_CHARS_HPP_

#include <iostream>
#include <array>

namespace rclcpp {

class hex_char_array_t {
    std::array<char, 8> string_;

    friend std::ostream& operator<<(std::ostream& os, const hex_char_array_t & str);

public:
    hex_char_array_t(uint32_t input) {
        static const char to_char[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        for (int j = 0, i = 7; i >= 0; --i, ++j) {
            int four_bits = ((input >> (i * 4)) & 15);
            string_[j] = to_char[four_bits];
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const hex_char_array_t & str) {
    for (const auto& s : str.string_) {
        os << s;
    }

    return os;
}

} // namespace rclcpp

#endif // RCLCPP__HASH_TO_CHARS_HPP_