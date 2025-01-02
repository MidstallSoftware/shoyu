#include "compositor-private.h"
#include "output-private.h"
#include "pointer-input-private.h"
#include "xdg-toplevel-private.h"

/**
 * ShoyuPointerInput:
 *
 * An input which represents a mouse pointer for #ShoyuCompositor.
 */

G_DEFINE_TYPE(ShoyuPointerInput, shoyu_pointer_input, SHOYU_TYPE_INPUT)

static void shoyu_pointer_input_process_cursor_motion(ShoyuPointerInput *self,
                                                      uint32_t time) {
  ShoyuInput *input = SHOYU_INPUT(self);

  ShoyuXdgToplevel *xdg_toplevel =
      shoyu_compositor_get_focused_xdg_toplevel(input->compositor);

  struct wl_output *wl_output = wlr_output_layout_output_at(
      input->compositor->output_layout, self->cursor->x, self->cursor->y);
  g_return_if_fail(wl_output != NULL);

  ShoyuOutput *output =
      shoyu_compositor_get_output(input->compositor, wl_output);
  g_return_if_fail(output != NULL);

  double sx = 0.0;
  double sy = 0.0;

  wlr_output_layout_output_coords(input->compositor->output_layout, wl_output,
                                  &sx, &sy);

  double rx = self->cursor->x - sx;
  double ry = self->cursor->y - sy;

  if (xdg_toplevel != NULL) {
    wlr_seat_pointer_notify_enter(input->compositor->wlr_seat,
                                  xdg_toplevel->wlr_xdg_toplevel->base->surface,
                                  rx, ry);

    wlr_seat_pointer_notify_motion(input->compositor->wlr_seat, time, rx, ry);
  } else {
    if (output->wlr_surface != NULL) {
      struct wlr_xdg_toplevel *wlr_xdg_toplevel =
          wlr_xdg_toplevel_try_from_wlr_surface(output->wlr_surface);

      if (wlr_xdg_toplevel != NULL) {
        wlr_xdg_toplevel_set_activated(wlr_xdg_toplevel, TRUE);
      }

      wlr_seat_pointer_notify_enter(input->compositor->wlr_seat,
                                    output->wlr_surface, rx, ry);

      wlr_seat_pointer_notify_motion(input->compositor->wlr_seat, time, rx, ry);
    } else {
      wlr_seat_pointer_clear_focus(input->compositor->wlr_seat);
    }
  }
}

static void shoyu_pointer_input_cursor_motion(struct wl_listener *listener,
                                              void *data) {
  ShoyuPointerInput *self = wl_container_of(listener, self, cursor_motion);
  struct wlr_pointer_motion_event *event = data;

  wlr_cursor_move(self->cursor, &event->pointer->base, event->delta_x,
                  event->delta_y);
  shoyu_pointer_input_process_cursor_motion(self, event->time_msec);
}

static void
shoyu_pointer_input_cursor_motion_absolute(struct wl_listener *listener,
                                           void *data) {
  ShoyuPointerInput *self =
      wl_container_of(listener, self, cursor_motion_absolute);
  struct wlr_pointer_motion_absolute_event *event = data;

  wlr_cursor_warp_absolute(self->cursor, &event->pointer->base, event->x,
                           event->y);
  shoyu_pointer_input_process_cursor_motion(self, event->time_msec);
}

static void shoyu_pointer_input_cursor_button(struct wl_listener *listener,
                                              void *data) {
  ShoyuPointerInput *self = wl_container_of(listener, self, cursor_button);
  ShoyuInput *input = SHOYU_INPUT(self);

  struct wlr_pointer_button_event *event = data;

  wlr_seat_pointer_notify_button(input->compositor->wlr_seat, event->time_msec,
                                 event->button, event->state);
}

static void shoyu_pointer_input_cursor_axis(struct wl_listener *listener,
                                            void *data) {
  ShoyuPointerInput *self = wl_container_of(listener, self, cursor_axis);
  ShoyuInput *input = SHOYU_INPUT(self);

  struct wlr_pointer_axis_event *event = data;

  wlr_seat_pointer_notify_axis(input->compositor->wlr_seat, event->time_msec,
                               event->orientation, event->delta,
                               event->delta_discrete, event->source,
                               event->relative_direction);
}

static void shoyu_pointer_input_cursor_frame(struct wl_listener *listener,
                                             void *data) {
  ShoyuPointerInput *self = wl_container_of(listener, self, cursor_frame);

  ShoyuInput *input = SHOYU_INPUT(self);

  wlr_seat_pointer_notify_frame(input->compositor->wlr_seat);
}

static void shoyu_pointer_input_constructed(GObject *object) {
  G_OBJECT_CLASS(shoyu_pointer_input_parent_class)->constructed(object);

  ShoyuPointerInput *self = SHOYU_POINTER_INPUT(object);
  ShoyuInput *input = SHOYU_INPUT(self);

  wlr_cursor_set_xcursor(self->cursor, input->compositor->wlr_xcursor_manager,
                         "default");
}

static void shoyu_pointer_input_finalize(GObject *object) {
  ShoyuPointerInput *self = SHOYU_POINTER_INPUT(object);

  g_clear_pointer(&self->cursor, (GDestroyNotify)wlr_cursor_destroy);

  G_OBJECT_CLASS(shoyu_pointer_input_parent_class)->finalize(object);
}

static void
shoyu_pointer_input_realized(ShoyuInput *input,
                             struct wlr_input_device *wlr_input_device) {
  ShoyuPointerInput *self = SHOYU_POINTER_INPUT(input);

  self->cursor_motion.notify = shoyu_pointer_input_cursor_motion;
  wl_signal_add(&self->cursor->events.motion, &self->cursor_motion);

  self->cursor_motion_absolute.notify =
      shoyu_pointer_input_cursor_motion_absolute;
  wl_signal_add(&self->cursor->events.motion_absolute,
                &self->cursor_motion_absolute);

  self->cursor_button.notify = shoyu_pointer_input_cursor_button;
  wl_signal_add(&self->cursor->events.button, &self->cursor_button);

  self->cursor_axis.notify = shoyu_pointer_input_cursor_axis;
  wl_signal_add(&self->cursor->events.axis, &self->cursor_axis);

  self->cursor_frame.notify = shoyu_pointer_input_cursor_frame;
  wl_signal_add(&self->cursor->events.frame, &self->cursor_frame);

  wlr_cursor_attach_input_device(self->cursor, wlr_input_device);
  wlr_cursor_attach_output_layout(self->cursor,
                                  input->compositor->output_layout);
}

static void shoyu_pointer_input_unrealized(ShoyuInput *input) {
  ShoyuPointerInput *self = SHOYU_POINTER_INPUT(input);

  wl_list_remove(&self->cursor_motion.link);
  wl_list_remove(&self->cursor_motion_absolute.link);
  wl_list_remove(&self->cursor_button.link);
  wl_list_remove(&self->cursor_axis.link);
  wl_list_remove(&self->cursor_frame.link);

  wlr_cursor_detach_input_device(self->cursor, input->wlr_input_device);
}

static void shoyu_pointer_input_class_init(ShoyuPointerInputClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  ShoyuInputClass *input_class = SHOYU_INPUT_CLASS(class);

  object_class->constructed = shoyu_pointer_input_constructed;
  object_class->finalize = shoyu_pointer_input_finalize;

  input_class->realized = shoyu_pointer_input_realized;
  input_class->unrealized = shoyu_pointer_input_unrealized;
}

static void shoyu_pointer_input_init(ShoyuPointerInput *self) {
  self->cursor = wlr_cursor_create();
}
