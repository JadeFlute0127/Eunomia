/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/process.h"

//#include <gtest/gtest.h>

#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
  {
    tracker_manager manager;
    std::cout << "start ebpf...\n";

    auto server = prometheus_server("127.0.0.1:8528");

    auto prometheus_event_handler =
        std::make_shared<process_tracker::prometheus_event_handler>(process_tracker::prometheus_event_handler(server));
    auto json_event_printer = std::make_shared<process_tracker::json_event_printer>(process_tracker::json_event_printer{});
    // auto json_event_printer2 =
    // std::make_shared<process_tracker::json_event_printer>(process_tracker::json_event_printer{});
    // prometheus_event_handler->add_handler(json_event_printer);
    //  prometheus_event_handler->add_handler(json_event_printer)->add_handler(json_event_printer2);

    auto tracker_ptr = process_tracker::create_tracker_with_default_env(json_event_printer);
    manager.start_tracker(std::move(tracker_ptr));

    server.start_prometheus_server();

    std::this_thread::sleep_for(1s);
  }
  return 0;
}
