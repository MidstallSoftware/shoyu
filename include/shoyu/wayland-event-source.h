#pragma once

#include <glib.h>
#include <wayland-server-core.h>

GSource* shoyu_wayland_event_source_new(struct wl_display* wl_display, struct wl_event_loop* event_loop);
