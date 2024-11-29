#pragma once

#if !defined(__SHOYU_SHELL_GTK_H_INSIDE__) &&                                  \
    !defined(SHOYU_SHELL_GTK_COMPILATION)
#error "Only <shoyu-shell-gtk3/shoyu-shell-gtk3.h> can be included directly."
#endif

#include <gdk/gdk.h>
#include <glib-object.h>
#include <shoyu-shell-gtk3/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_SHELL_GTK_COMPILATION
typedef struct _ShoyuShellGtkDisplay ShoyuShellGtkDisplay;
#else
typedef GObject ShoyuShellGtkDisplay;
#endif
typedef struct _ShoyuShellGtkDisplayClass ShoyuShellGtkDisplayClass;

#define SHOYU_SHELL_GTK_TYPE_DISPLAY (shoyu_shell_gtk_display_get_type())
#define SHOYU_SHELL_GTK_DISPLAY(object)                                        \
  (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_SHELL_GTK_TYPE_DISPLAY,          \
                              ShoyuShellGtkDisplay))
#define SHOYU_SHELL_GTK_DISPLAY_CLASS(klass)                                   \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_SHELL_GTK_TYPE_DISPLAY,              \
                           ShoyuShellGtkDisplayClass))
#define SHOYU_SHELL_GTK_IS_DISPLAY(object)                                     \
  (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_SHELL_GTK_TYPE_DISPLAY))
#define SHOYU_SHELL_GTK_IS_DISPLAY_CLASS(klass)                                \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_SHELL_GTK_TYPE_DISPLAY))
#define SHOYU_SHELL_GTK_DISPLAY_GET_CLASS(obj)                                 \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_SHELL_GTK_TYPE_DISPLAY,              \
                             ShoyuShellGtkDisplayClass))

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
GType shoyu_shell_gtk_display_get_type(void) G_GNUC_CONST;

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
ShoyuShellGtkDisplay *shoyu_shell_gtk_display_get(GdkDisplay *display);

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
GListModel *shoyu_shell_gtk_display_get_toplevels(ShoyuShellGtkDisplay *self);

G_END_DECLS
