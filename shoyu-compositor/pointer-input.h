#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <shoyu-compositor/input.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuPointerInput ShoyuPointerInput;
#else
typedef ShoyuInput ShoyuPointerInput;
#endif
typedef struct _ShoyuPointerInputClass ShoyuPointerInputClass;

#define SHOYU_TYPE_POINTER_INPUT (shoyu_pointer_input_get_type())
#define SHOYU_POINTER_INPUT(object)                                            \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_POINTER_INPUT,              \
                              ShoyuPointerInput))
#define SHOYU_POINTER_INPUT_CLASS(klass)                                       \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_POINTER_INPUT,                  \
                           ShoyuPointerInputClass))
#define SHOYU_IS_POINTER_INPUT(object)                                         \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_POINTER_INPUT))
#define SHOYU_IS_POINTER_INPUT_CLASS(klass)                                    \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_POINTER_INPUT))
#define SHOYU_POINTER_INPUT_GET_CLASS(obj)                                     \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_POINTER_INPUT,                  \
                             ShoyuPointerInputClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_pointer_input_get_type(void) G_GNUC_CONST;

G_END_DECLS
