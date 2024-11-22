#pragma once

#include "wayland-event-source.h"

struct _ShoyuWaylandEventSource {
    GSource source;
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;
};
