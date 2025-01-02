#include "compositor-private.h"
#include "keyboard-input-private.h"
#include "output-private.h"
#include "xdg-toplevel-private.h"

/**
 * ShoyuKeyboardInput:
 *
 * An input which represents a keyboard for #ShoyuCompositor.
 */

G_DEFINE_TYPE(ShoyuKeyboardInput, shoyu_keyboard_input, SHOYU_TYPE_INPUT)

static void shoyu_keyboard_input_modifiers(struct wl_listener *listener,
                                           void *data) {
  ShoyuKeyboardInput *self = wl_container_of(listener, self, modifiers);
  ShoyuInput *input = SHOYU_INPUT(self);

  struct wlr_keyboard *wlr_keyboard =
      wlr_keyboard_from_input_device(input->wlr_input_device);

  wlr_seat_set_keyboard(input->compositor->wlr_seat, wlr_keyboard);
  wlr_seat_keyboard_notify_modifiers(input->compositor->wlr_seat,
                                     &wlr_keyboard->modifiers);
}

static void shoyu_keyboard_input_key(struct wl_listener *listener, void *data) {
  ShoyuKeyboardInput *self = wl_container_of(listener, self, key);
  ShoyuInput *input = SHOYU_INPUT(self);

  struct wlr_keyboard_key_event *event = data;

  struct wlr_keyboard *wlr_keyboard =
      wlr_keyboard_from_input_device(input->wlr_input_device);

  wlr_seat_set_keyboard(input->compositor->wlr_seat, wlr_keyboard);

  wlr_seat_keyboard_notify_key(input->compositor->wlr_seat, event->time_msec,
                               event->keycode, event->state);
}

static void shoyu_keyboard_input_finalize(GObject *object) {
  ShoyuKeyboardInput *self = SHOYU_KEYBOARD_INPUT(object);

  g_clear_pointer(&self->xkb_keymap, (GDestroyNotify)xkb_keymap_unref);

  G_OBJECT_CLASS(shoyu_keyboard_input_parent_class)->finalize(object);
}

static void
shoyu_keyboard_input_realized(ShoyuInput *input,
                              struct wlr_input_device *wlr_input_device) {
  ShoyuKeyboardInput *self = SHOYU_KEYBOARD_INPUT(input);

  struct wlr_keyboard *wlr_keyboard =
      wlr_keyboard_from_input_device(wlr_input_device);

  wlr_keyboard_set_keymap(wlr_keyboard, self->xkb_keymap);
  wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

  self->modifiers.notify = shoyu_keyboard_input_modifiers;
  wl_signal_add(&wlr_keyboard->events.modifiers, &self->modifiers);

  self->key.notify = shoyu_keyboard_input_key;
  wl_signal_add(&wlr_keyboard->events.key, &self->key);

  wlr_seat_set_keyboard(input->compositor->wlr_seat, wlr_keyboard);
}

static void shoyu_keyboard_input_unrealized(ShoyuInput *input) {
  ShoyuKeyboardInput *self = SHOYU_KEYBOARD_INPUT(input);

  wl_list_remove(&self->modifiers.link);
  wl_list_remove(&self->key.link);
}

static void shoyu_keyboard_input_class_init(ShoyuKeyboardInputClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  ShoyuInputClass *input_class = SHOYU_INPUT_CLASS(class);

  object_class->finalize = shoyu_keyboard_input_finalize;

  input_class->realized = shoyu_keyboard_input_realized;
  input_class->unrealized = shoyu_keyboard_input_unrealized;
}

static void shoyu_keyboard_input_init(ShoyuKeyboardInput *self) {
  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  self->xkb_keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

  xkb_context_unref(context);
}
