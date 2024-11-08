#include "wayland-event-source-private.h"

static gboolean shoyu_wayland_event_source_prepare(GSource* source, int* timeout) {
  ShoyuWaylandEventSource* self = (ShoyuWaylandEventSource*)source;

  *timeout = -1;

  if (self->wl_display != NULL) wl_display_flush_clients(self->wl_display);

  return FALSE;
}

static gboolean shoyu_wayland_event_source_dispatch(GSource* source, GSourceFunc callback, gpointer data) {
  ShoyuWaylandEventSource* self = (ShoyuWaylandEventSource*)source;

  wl_event_loop_dispatch(self->event_loop, 0);
  return TRUE;
}

static GSourceFuncs shoyu_wayland_event_source_funcs = {
  shoyu_wayland_event_source_prepare,
  NULL,
  shoyu_wayland_event_source_dispatch,
  NULL,
};

GSource* shoyu_wayland_event_source_new(struct wl_display* wl_display, struct wl_event_loop* event_loop) {
  GSource* source = g_source_new(&shoyu_wayland_event_source_funcs, sizeof (ShoyuWaylandEventSource));

  char* name = g_strdup_printf("Wayland Event Loop source (%p)", event_loop);
  g_source_set_name(source, name);
  g_free(name);

  ShoyuWaylandEventSource* self = (ShoyuWaylandEventSource*)source;

  self->wl_display = wl_display;
  self->event_loop = event_loop;

  g_source_add_unix_fd(source, wl_event_loop_get_fd(event_loop), G_IO_IN | G_IO_ERR);
  g_source_set_can_recurse(source, TRUE);
  g_source_attach(source, NULL);

  return source;
}
