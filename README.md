# PMROS2

A performance observability framework for ROS2 (version Dashing, RCLCPP client library).

*This repository is a product of a Master's thesis. The associated report can be found at: [todo: link TU/e library]()*

# High Level Overview

This framework tracks `Timer` Activations, Messages passing through `Publisher` instances, and messages passing through `Subscriber` instances.
The framework is extensible both in what measurements are produced from this tracking, and how these measurements are stored. Implementations for measuring `Timer Activation Jitter`, `Node-to-node Message Latency`, and `End-to-end Message Latency`[^1] are provided. Implementations for writing measurements to CSV, stdout, and InfluxDB are provided.

[^1]: Time between a message being published by some node $n$, and received by some node $m$, with 0 or more intermediate nodes.

# Installation

1. Follow the steps for [building ROS2 Dashing from source](https://docs.ros.org/en/dashing/Installation.html),
until you are at the step where the ROS2 source code is compiled

2. Drag and drop the four ROS2 packages (*rosidl, ros2cli, rcl, rclcpp*) of this repository into `src/ros2`. This should overwrite the equivalently named ROS2 packages in this directory.

3. If you will be using InfluxDB for storing measurements, navigate to `src/ros2/rclcpp/rclcpp/include/rclcpp/measuring/influxdb_measurement_writer.hpp`. Edit this file (line 29 and on) to specify your InfluxDB token, port, bucket, etc.

4. Proceed with the steps for building ROS2 Dashing from source.

## Installing InfluxDB

Follow the instructions for [installing InfluxDB 2.x OSS](docs.influxdata.com/influxdb/v2/install). Version 2.7 was used in the report, so this is the recommended minor version number.

Be sure to store the access token and name of the bucket in which you want to store measurements.

## Changing InfluxDB upload settings

Because InfluxDB settings are currently hardcoded in RCLCPP, changing these values requires recompiling PMROS2.
It is not necessary to rebuild everything from source, however. Use the colcon `--packages-select` flag to rebuild only RCLCPP.

## Querying InfluxDB

The `InfluxDBMeasurementWriter` class uploads measurements named `message_latency` and `activation_jitter`.
Node-to-Node Message Latency can be directly computed from a measurement by subtracting `send_time` from `arrival_time`.
End-to-end Message Latency must be computed by an inner join on message ID. 
Caution: At the time of writing, it is required to drop all tags from data subject to an inner join first, or the result of the inner join will be empty! This may be a bug with Flux.

# Using existing nodes with PMROS2

If a node compiles & runs on standard ROS2 Dashing, it should automatically work for PMROS2 as well.
Simply source the PMROS2 installation, and recompile your nodes. You may have to do a clean build of your nodes, to prevent undefined behavior.

# Configuration

By default, all nodes using PMROS2 are tracked for `Timer Activation Jitter`, `Node-to-node Message Latency` and `End-to-end Message Latency`. Trackers on `Timer` and `Subscriber` by default report measurements to InfluxDB.

This can be changed by using optional parameters of Timer, Subscriber & Publisher constructors.[^2]
These objects all have constructor overloads for specifying a (Timer/Subscriber/Publisher)Options struct.
This struct has enum values for specifying what instances of tracker & writer to use.

Timers of nodes can be explicitly named in `TimerOptions`. 
In default ROS2 Dashing, timers are unnamed objects. 
Therefore having multiple timers on one node may yield measurements that are difficult to analyse, unless the timers are named.

[^2]: Or the create_wall_timer member function of `Node`, which is what one uses to create `Timer` instances for `Node`.

# Accessing hidden variables on messages

It may be necessary to access the hidden variables of messages.
For instance when measuring complicated paths in end-to-end message latency,
or when extending the framework to provide trackers that read or write to these variables.

The 3 hidden variables on all messages are[^3]:

* vandenhoven_timestamp
* vandenhoven_publisher_hash
* vandenhoven_msg_id

Please reference the linked report for an explanation on the purpose and necessity of these variables.

[^3]: ROSIDL does not allow message variable names to start with underscores, or double underscores anywhere. I instead used my uncommon last name to ensure no message would exist that already uses these variable names.

# Possible improvements

This repository welcomes contributions for the improvement of PMROS2.
Please reference the linked report (Future Work chapter) for some large-scope improvement ideas.

Smaller areas of improvement are:

* Change ROSIDL to allow internal exceptions in its naming restrictions. This enables hidden message variables to start with underscores, while still ensuring these variable names are unique.
* Alternate `MeasurementWriterInterface` instances, such as for InfluxDB 1.x, 3.x, or even other TSDBs like Prometheus.
* Moving InfluxDB configuration to config files.
* Replacing the bundled single-header InfluxDB 2.x client library with something more robust.
* Tracking for intra-process messages
* Tracking for services (ROSIDL already generates messages used in services with hidden variables)
* Allowing the tracker factory to work with shared_ptr alongside enums. This way the ability to provide alternative tracker & writer interface instances is accessible to the application layer.
* Improve the Timer Tracker implementation to report Activation Jitter as intended & actual activation time, instead of computing activation jitter in the tracker and reporting that value.
* Generate a diff between standard ROS2 Dashing and this framework, to provide an exhaustive list of files added or modified.
* General code improvements. PMROS2 is not perfect, and it never will be :)