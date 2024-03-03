// todo: this should be removed at the end of the project. It is for debugging my stuff only.

#ifndef RCLCPP__SIMPLETIMER_HPP_
#define RCLCPP__SIMPLETIMER_HPP_

#include <chrono>

namespace rclcpp {

class SimpleTimer {
public:
    SimpleTimer() : name_("unnamed") {}
    SimpleTimer(const std::string& n) : name_(n) {}

    ~SimpleTimer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << name_ << ": " << duration.count() << " us\n";
    }
private:
    std::string name_;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
};

}

#endif // RCLCPP__SIMPLETIMER_HPP_