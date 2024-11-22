#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib.h>
#include <shoyu-compositor/version.h>
#include <wayland-server-core.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuWaylandEventSource ShoyuWaylandEventSource;
#else
typedef GSource ShoyuWaylandEventSource;
#endif

SHOYU_AVAILABLE_IN_ALL
GSource *shoyu_wayland_event_source_new(struct wl_display *wl_display,
                                        struct wl_event_loop *event_loop);

G_END_DECLS
