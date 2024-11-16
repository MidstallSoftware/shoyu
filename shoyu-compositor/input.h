#pragma once

#if !defined (__SHOYU_COMPOSITOR_H_INSIDE__) && !defined (SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <shoyu-compositor/compositor.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuInput ShoyuInput;
#else
typedef GObject ShoyuInput;
#endif
typedef struct _ShoyuInputClass ShoyuInputClass;

#define SHOYU_TYPE_INPUT (shoyu_input_get_type())
#define SHOYU_INPUT(object) (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_INPUT, ShoyuInput))
#define SHOYU_INPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_INPUT, ShoyuInputClass))
#define SHOYU_IS_INPUT(object) (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_INPUT))
#define SHOYU_IS_INPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_INPUT))
#define SHOYU_INPUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_INPUT, ShoyuInputClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_input_get_type(void) G_GNUC_CONST;

SHOYU_AVAILABLE_IN_ALL
ShoyuInput* shoyu_input_new(ShoyuCompositor* compositor);

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor* shoyu_input_get_compositor(ShoyuInput* self);

G_END_DECLS
