#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <shoyu-compositor/compositor.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuOutput ShoyuOutput;
#else
typedef GObject ShoyuOutput;
#endif
typedef struct _ShoyuOutputClass ShoyuOutputClass;

#define SHOYU_TYPE_OUTPUT (shoyu_output_get_type())
#define SHOYU_OUTPUT(object)                                                   \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_OUTPUT, ShoyuOutput))
#define SHOYU_OUTPUT_CLASS(klass)                                              \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_OUTPUT, ShoyuOutputClass))
#define SHOYU_IS_OUTPUT(object)                                                \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_OUTPUT))
#define SHOYU_IS_OUTPUT_CLASS(klass)                                           \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_OUTPUT))
#define SHOYU_OUTPUT_GET_CLASS(obj)                                            \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_OUTPUT, ShoyuOutputClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_output_get_type(void) G_GNUC_CONST;

SHOYU_AVAILABLE_IN_ALL
ShoyuOutput *shoyu_output_new(ShoyuCompositor *compositor);

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor *shoyu_output_get_compositor(ShoyuOutput *self);

G_END_DECLS
