#ifndef TCP_CMD_H
#define TCP_CMD_H

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

extern "C"
{
#include <tcp/tcp_tracker.h>
}

// ebpf tcp tracker interface
// the true implementation is in tcp/tcp_tracker.h
//
// trace tcp start and exit
struct tcp_tracker : public tracker_with_config<tcp_env, tcp_event>
{
  using config_data = tracker_config<tcp_env, tcp_event>;
  using tracker_event_handler = std::shared_ptr<event_handler<tcp_event>>;

  tcp_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<tcp_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  tcp_tracker(tcp_env env);
  // start tcp tracker
  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<tcp_event>
  {
    //prometheus::Family<prometheus::Counter> &eunomia_tcp_start_counter;
    //prometheus::Family<prometheus::Counter> &eunomia_tcp_exit_counter;
    void report_prometheus_event(const struct tcp_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<tcp_event> &e);
  };

  // convert event to json
  struct json_event_handler_base : public event_handler<tcp_event>
  {
    json to_json(const struct tcp_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler_base
  {
    void handle(tracker_event<tcp_event> &e);
  };

  struct plain_text_event_printer : public event_handler<tcp_event>
  {
    void handle(tracker_event<tcp_event> &e);
  };
private:
    static void handle_tcp_sample_event(void *ctx, int cpu, void *data, unsigned int data_sz);
};

#endif