#pragma once

#if !defined (__SHOYU_COMPOSITOR_H_INSIDE__) && !defined (SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuShell ShoyuShell;
#else
typedef GObject ShoyuShell;
#endif
typedef struct _ShoyuShellClass ShoyuShellClass;

#define SHOYU_TYPE_SHELL (shoyu_shell_get_type())
#define SHOYU_SHELL(object) (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_SHELL, ShoyuShell))
#define SHOYU_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_SHELL, ShoyuShellClass))
#define SHOYU_IS_SHELL(object) (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_SHELL))
#define SHOYU_IS_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_SHELL))
#define SHOYU_SHELL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_SHELL, ShoyuShellClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_shell_get_type(void) G_GNUC_CONST;

G_END_DECLS
