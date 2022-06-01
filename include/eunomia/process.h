/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "model/tracker.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"

using json = nlohmann::json;

// ebpf process tracker interface
// the true implementation is in process/process_tracker.h
//
// trace process start and exit
struct process_tracker : public tracker_with_config<process_env, process_event>
{
  using config_data = tracker_config<process_env, process_event>;
  using tracker_event_handler = std::shared_ptr<event_handler<process_event>>;

  process_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<process_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  process_tracker(process_env env);
  // start process tracker
  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<process_event>
  {
    prometheus::Family<prometheus::Counter> &eunomia_process_start_counter;
    prometheus::Family<prometheus::Counter> &eunomia_process_exit_counter;
    void report_prometheus_event(const struct process_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<process_event> &e);
  };

  // convert event to json
  struct json_event_handler_base : public event_handler<process_event>
  {
    std::string to_json(const struct process_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler_base
  {
    void handle(tracker_event<process_event> &e);
  };

  struct plain_text_event_printer : public event_handler<process_event>
  {
    void handle(tracker_event<process_event> &e);
  };

  struct csv_event_printer : public event_handler<process_event>
  {
    void handle(tracker_event<process_event> &e);
  };

};

#endif