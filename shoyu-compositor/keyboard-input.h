#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <shoyu-compositor/input.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuKeyboardInput ShoyuKeyboardInput;
#else
typedef ShoyuInput ShoyuKeyboardInput;
#endif
typedef struct _ShoyuKeyboardInputClass ShoyuKeyboardInputClass;

#define SHOYU_TYPE_KEYBOARD_INPUT (shoyu_keyboard_input_get_type())
#define SHOYU_KEYBOARD_INPUT(object)                                           \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_KEYBOARD_INPUT,             \
                              ShoyuKeyboardInput))
#define SHOYU_KEYBOARD_INPUT_CLASS(klass)                                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_KEYBOARD_INPUT,                 \
                           ShoyuKeyboardInputClass))
#define SHOYU_IS_KEYBOARD_INPUT(object)                                        \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_KEYBOARD_INPUT))
#define SHOYU_IS_KEYBOARD_INPUT_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_KEYBOARD_INPUT))
#define SHOYU_KEYBOARD_INPUT_GET_CLASS(obj)                                    \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_KEYBOARD_INPUT,                 \
                             ShoyuKeyboardInputClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_keyboard_input_get_type(void) G_GNUC_CONST;

G_END_DECLS
