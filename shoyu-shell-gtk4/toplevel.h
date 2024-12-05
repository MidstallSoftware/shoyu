#pragma once

#if !defined(__SHOYU_SHELL_GTK_H_INSIDE__) &&                                  \
    !defined(SHOYU_SHELL_GTK_COMPILATION)
#error "Only <shoyu-shell-gtk4/shoyu-shell-gtk4.h> can be included directly."
#endif

#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <shoyu-shell-gtk4/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_SHELL_GTK_COMPILATION
typedef struct _ShoyuShellGtkToplevel ShoyuShellGtkToplevel;
#else
typedef GObject ShoyuShellGtkToplevel;
#endif
typedef struct _ShoyuShellGtkToplevelClass ShoyuShellGtkToplevelClass;

#define SHOYU_SHELL_GTK_TYPE_TOPLEVEL (shoyu_shell_gtk_toplevel_get_type())
#define SHOYU_SHELL_GTK_TOPLEVEL(object)                                       \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_SHELL_GTK_TYPE_TOPLEVEL,         \
                              ShoyuShellGtkToplevel))
#define SHOYU_SHELL_GTK_TOPLEVEL_CLASS(klass)                                  \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_SHELL_GTK_TYPE_TOPLEVEL,             \
                           ShoyuShellGtkToplevelClass))
#define SHOYU_SHELL_GTK_IS_TOPLEVEL(object)                                    \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_SHELL_GTK_TYPE_TOPLEVEL))
#define SHOYU_SHELL_GTK_IS_TOPLEVEL_CLASS(klass)                               \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_SHELL_GTK_TYPE_TOPLEVEL))
#define SHOYU_SHELL_GTK_TOPLEVEL_GET_CLASS(obj)                                \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_SHELL_GTK_TYPE_TOPLEVEL,             \
                             ShoyuShellGtkToplevelClass))

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
GType shoyu_shell_gtk_toplevel_get_type(void) G_GNUC_CONST;

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
void shoyu_shell_gtk_toplevel_set_geometry(ShoyuShellGtkToplevel *self,
                                           uint32_t x, uint32_t y,
                                           uint32_t width, uint32_t height);

G_END_DECLS
