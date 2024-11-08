#pragma once

#include <shoyu/wayland-event-source.h>

typedef struct _ShoyuWaylandEventSource {
  GSource source;
  struct wl_display* wl_display;
  struct wl_event_loop* event_loop;
} ShoyuWaylandEventSource;
