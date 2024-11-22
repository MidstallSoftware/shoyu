#pragma once

#if !defined (__SHOYU_COMPOSITOR_H_INSIDE__) && !defined (SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>
#include <shoyu-compositor/shell.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

#ifdef SHOYU_COMPILATION
typedef struct _ShoyuCompositor ShoyuCompositor;
#else
typedef GObject ShoyuCompositor;
#endif
typedef struct _ShoyuCompositorClass ShoyuCompositorClass;

#define SHOYU_TYPE_COMPOSITOR (shoyu_compositor_get_type())
#define SHOYU_COMPOSITOR(object) (G_TYPE_CHECK_INSTANCE_CAST((object), SHOYU_TYPE_COMPOSITOR, ShoyuCompositor))
#define SHOYU_COMPOSITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_TYPE_COMPOSITOR, ShoyuCompositorClass))
#define SHOYU_IS_COMPOSITOR(object) (G_TYPE_CHECK_INSTANCE_TYPE((object), SHOYU_TYPE_COMPOSITOR))
#define SHOYU_IS_COMPOSITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_TYPE_COMPOSITOR))
#define SHOYU_COMPOSITOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_TYPE_COMPOSITOR, ShoyuCompositorClass))

SHOYU_AVAILABLE_IN_ALL
GType shoyu_compositor_get_type(void) G_GNUC_CONST;

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor* shoyu_compositor_new(void);

SHOYU_AVAILABLE_IN_ALL
ShoyuCompositor* shoyu_compositor_new_with_application(GApplication* application);

SHOYU_AVAILABLE_IN_ALL
GApplication* shoyu_compositor_get_application(ShoyuCompositor* self);

SHOYU_AVAILABLE_IN_ALL
const char* shoyu_compositor_get_socket(ShoyuCompositor* self);

SHOYU_AVAILABLE_IN_ALL
gboolean shoyu_compositor_start(ShoyuCompositor* self);

SHOYU_AVAILABLE_IN_ALL
ShoyuShell* shoyu_compositor_get_shell(ShoyuCompositor* self);

G_END_DECLS
