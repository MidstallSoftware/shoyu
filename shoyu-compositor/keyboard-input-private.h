#pragma once

#include "input-private.h"
#include "keyboard-input.h"

#include <wlr/types/wlr_keyboard.h>

struct _ShoyuKeyboardInput {
    ShoyuInput parent_instance;
    struct xkb_keymap *xkb_keymap;

    struct wl_listener modifiers;
    struct wl_listener key;
};

struct _ShoyuKeyboardInputClass {
    ShoyuInputClass parent_class;
};
