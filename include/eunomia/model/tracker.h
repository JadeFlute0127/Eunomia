/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef TRAKER_H
#define TRAKER_H

#include <iostream>
#include <mutex>
#include <thread>

#include "tracker_config.h"

// the base type of a tracker
// for tracker manager to manage
struct tracker_base
{
  // base thread
  std::thread thread;
  volatile bool exiting;
  // TODO: use the mutex
  std::mutex mutex;

 public:
  virtual ~tracker_base()
  {
    stop_tracker();
  }
  virtual void start_tracker(void) = 0;
  void stop_tracker(void)
  {
    exiting = true;
    if (thread.joinable())
    {
      thread.join();
    }
  }
};

class prometheus_server;

// all tracker should inherit from this class
template<typename ENV, typename EVENT>
struct tracker_with_config : public tracker_base
{
  // type alias
  using config_data = tracker_config<ENV, EVENT>;
  using tracker_event_handler = std::shared_ptr<event_handler<EVENT>>;

  // default event handlers
  struct prometheus_event_handler final : public event_handler<EVENT>
  {
    prometheus_event_handler(prometheus_server &server)
    {
    }
    void handle(tracker_event<EVENT> &e)
    {
    }
  };
  // print to plain text
  struct plain_text_event_printer final : public event_handler<EVENT>
  {
    void handle(tracker_event<EVENT> &e)
    {
    }
  };
  // used for json exporter, inherits from json_event_handler
  struct json_event_printer final
  {
    std::string to_json(const EVENT &e)
    {
      return std::string("{}");
    }
    void handle(tracker_event<EVENT> &e)
    {
      std::cout << to_json(e.event) << std::endl;
    }
  };
  // print to csv
  struct csv_event_printer final : public event_handler<EVENT>
  {
    void handle(tracker_event<EVENT> &e)
    {
    }
  };

  tracker_config<ENV, EVENT> current_config;
  tracker_with_config(tracker_config<ENV, EVENT> config) : current_config(config)
  {
  }
  virtual ~tracker_with_config(){
    stop_tracker();
  }
};

// concept for tracker
// all tracker should have these types
template<typename TRACKER>
concept tracker_concept = requires
{
  typename TRACKER::config_data;
  typename TRACKER::tracker_event_handler;
  typename TRACKER::prometheus_event_handler;
  typename TRACKER::json_event_printer;
  typename TRACKER::plain_text_event_printer;
  typename TRACKER::csv_event_printer;
};

// function for handler tracker event call back
// used when running a tracker
// Example:
//
// start_process_tracker(handle_tracker_event<process_tracker, process_event>, libbpf_print_fn, current_config.env, skel,
// (void *)this);
template<tracker_concept TRACKER, typename EVENT>
static int handle_tracker_event(void *ctx, void *data, size_t data_sz)
{
  if (!data || !ctx)
  {
    std::cout << "warn: no data or no ctx" << std::endl;
    return 0;
  }
  const EVENT &e = *(const EVENT *)data;
  TRACKER &pt = *(TRACKER *)ctx;
  auto event = tracker_event<EVENT>{ e };
  if (pt.current_config.handler)
  {
    pt.current_config.handler->do_handle_event(event);
  }
  else
  {
    std::cout << "warn: no handler for tracker event" << std::endl;
  }
  return 0;
}

#endif
