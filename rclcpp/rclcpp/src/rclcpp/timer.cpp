// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include "rclcpp/timer.hpp"

#include <chrono>
#include <string>
#include <memory>

#include "rclcpp/contexts/default_context.hpp"
#include "rclcpp/exceptions.hpp"
#include "rclcpp/measuring/jitter_tracker_factory.hpp"

#include "rcutils/logging_macros.h"
#include <iostream>
using rclcpp::TimerBase;

// free floating function to be passed into the C library as function pointer.
namespace rclcpp {
void rcl_timer_callback(rcl_timer_t *, int64_t l, int64_t i, void * instance) {
  static_cast<TimerBase *>(instance)->timer_activate_callback(l, i);
}
}

TimerBase::TimerBase(
  rclcpp::Clock::SharedPtr clock,
  std::chrono::nanoseconds period,
  rclcpp::Context::SharedPtr context,
  const rclcpp::TimerOptions& options)
: clock_(clock), timer_handle_(nullptr)
{
  {
    auto tracker_factory = JitterTrackerFactory::make(options.jitter_tracking_options.jitter_tracker_option);
    // todo: options must become an abstract struct just like the deprecated MessageTrackerHostInfo , some type of vector of pairs such that writers use it for tags.
    jitter_tracker_ = tracker_factory->create_jitter_tracker(options.jitter_tracking_options.result_writer_option, options);
  }

  if (nullptr == context) {
    context = rclcpp::contexts::default_context::get_global_default_context();
  }

  auto rcl_context = context->get_rcl_context();

  timer_handle_ = std::shared_ptr<rcl_timer_t>(
    new rcl_timer_t, [ = ](rcl_timer_t * timer) mutable
    {
      if (rcl_timer_fini(timer) != RCL_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
          "rclcpp",
          "Failed to clean up rcl timer handle: %s", rcl_get_error_string().str);
        rcl_reset_error();
      }
      delete timer;
      // Captured shared pointers by copy, reset to make sure timer is finalized before clock
      clock.reset();
      rcl_context.reset();
    });

  *timer_handle_.get() = rcl_get_zero_initialized_timer();

  rcl_clock_t * clock_handle = clock_->get_clock_handle();
  if (rcl_timer_init(
      timer_handle_.get(), clock_handle, rcl_context.get(), period.count(), &rcl_timer_callback,
      rcl_get_default_allocator()) != RCL_RET_OK)
  {
    RCUTILS_LOG_ERROR_NAMED(
      "rclcpp",
      "Couldn't initialize rcl timer handle: %s\n", rcl_get_error_string().str);
    rcl_reset_error();
  } else {
    rcl_timer_bind_instance(timer_handle_.get(), (void*)this); // not part of init due to backwards compatability with normal ROS2.
  }
}

TimerBase::~TimerBase()
{}

void
TimerBase::timer_activate_callback(int64_t time_since_last_call, int64_t intended_activation_time) {
  jitter_tracker_->track_jitter(clock_, time_since_last_call, intended_activation_time);
}

void
TimerBase::cancel()
{
  if (rcl_timer_cancel(timer_handle_.get()) != RCL_RET_OK) {
    throw std::runtime_error(std::string("Couldn't cancel timer: ") + rcl_get_error_string().str);
  }
}

bool
TimerBase::is_canceled()
{
  bool is_canceled = false;
  rcl_ret_t ret = rcl_timer_is_canceled(timer_handle_.get(), &is_canceled);
  if (ret != RCL_RET_OK) {
    rclcpp::exceptions::throw_from_rcl_error(ret, "Couldn't get timer cancelled state");
  }
  return is_canceled;
}

void
TimerBase::reset()
{
  if (rcl_timer_reset(timer_handle_.get()) != RCL_RET_OK) {
    throw std::runtime_error(std::string("Couldn't reset timer: ") + rcl_get_error_string().str);
  }
}

bool
TimerBase::is_ready()
{
  bool ready = false;
  if (rcl_timer_is_ready(timer_handle_.get(), &ready) != RCL_RET_OK) {
    throw std::runtime_error(std::string("Failed to check timer: ") + rcl_get_error_string().str);
  }
  return ready;
}

std::chrono::nanoseconds
TimerBase::time_until_trigger()
{
  int64_t time_until_next_call = 0;
  if (rcl_timer_get_time_until_next_call(timer_handle_.get(),
    &time_until_next_call) != RCL_RET_OK)
  {
    throw std::runtime_error(
            std::string("Timer could not get time until next call: ") +
            rcl_get_error_string().str);
  }
  return std::chrono::nanoseconds(time_until_next_call);
}

std::shared_ptr<const rcl_timer_t>
TimerBase::get_timer_handle()
{
  return timer_handle_;
}
