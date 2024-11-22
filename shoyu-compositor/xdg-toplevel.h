#pragma once

#if !defined(__SHOYU_COMPOSITOR_H_INSIDE__) && !defined(SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <gio/gio.h>
#include <glib-object.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuXdgToplevel ShoyuXdgToplevel;
#else
typedef GObject ShoyuXdgToplevel;
#endif
typedef struct _ShoyuXdgToplevelClass ShoyuXdgToplevelClass;

#define SHOYU_TYPE_XDG_TOPLEVEL (shoyu_xdg_toplevel_get_type())
#define SHOYU_XDG_TOPLEVEL(object)                                             \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_XDG_TOPLEVEL,               \
                              ShoyuXdgToplevel))
#define SHOYU_XDG_TOPLEVEL_CLASS(klass)                                        \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_XDG_TOPLEVEL,                   \
                           ShoyuXdgToplevelClass))
#define SHOYU_IS_XDG_TOPLEVEL(object)                                          \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_XDG_TOPLEVEL))
#define SHOYU_IS_XDG_TOPLEVEL_CLASS(klass)                                     \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_XDG_TOPLEVEL))
#define SHOYU_XDG_TOPLEVEL_GET_CLASS(obj)                                      \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_XDG_TOPLEVEL,                   \
                             ShoyuXdgToplevelClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_xdg_toplevel_get_type(void) G_GNUC_CONST;

SHOYU_AVAILABLE_IN_ALL
ShoyuXdgToplevel *shoyu_xdg_toplevel_new(ShoyuCompositor *compositor);

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor *shoyu_xdg_toplevel_get_compositor(ShoyuXdgToplevel *self);

G_END_DECLS
