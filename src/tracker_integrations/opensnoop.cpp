#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "opensnoop/opensnoop_tracker.h"
}

std::unique_ptr<opensnoop_tracker> opensnoop_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "opensnoop";
  config.env = tracker_alone_env{ .main_func = start_opensnoop };
  return std::make_unique<opensnoop_tracker>(config);
}

std::unique_ptr<opensnoop_tracker> opensnoop_tracker::create_tracker_with_args(
    tracker_event_handler handler,
    const std::vector<std::string> &args)
{
  auto tracker = opensnoop_tracker::create_tracker_with_default_env(handler);
  if (tracker)
  {
    tracker->current_config.env.process_args = args;
  }
  return tracker;
}
