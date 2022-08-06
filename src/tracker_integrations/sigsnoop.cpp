#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "sigsnoop/sigsnoop_tracker.h"
}

std::unique_ptr<sigsnoop_tracker> sigsnoop_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "sigsnoop";
  config.env = tracker_alone_env{ .main_func = start_sigsnoop };
  return std::make_unique<sigsnoop_tracker>(config);
}