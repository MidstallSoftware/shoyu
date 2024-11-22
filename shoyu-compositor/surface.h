#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <shoyu-compositor/compositor.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuSurface ShoyuSurface;
#else
typedef GObject ShoyuSurface;
#endif
typedef struct _ShoyuSurfaceClass ShoyuSurfaceClass;

#define SHOYU_TYPE_SURFACE (shoyu_surface_get_type())
#define SHOYU_SURFACE(object)                                                  \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_SURFACE, ShoyuSurface))
#define SHOYU_SURFACE_CLASS(klass)                                             \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_SURFACE, ShoyuSurfaceClass))
#define SHOYU_IS_SURFACE(object)                                               \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_SURFACE))
#define SHOYU_IS_SURFACE_CLASS(klass)                                          \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_SURFACE))
#define SHOYU_SURFACE_GET_CLASS(obj)                                           \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_SURFACE, ShoyuSurfaceClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_surface_get_type(void) G_GNUC_CONST;

SHOYU_AVAILABLE_IN_ALL
ShoyuSurface *shoyu_surface_new(ShoyuCompositor *compositor);

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor *shoyu_surface_get_compositor(ShoyuSurface *self);

G_END_DECLS
