#pragma once

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _ShoyuCompositorRunnerApplication
    ShoyuCompositorRunnerApplication;
typedef struct _ShoyuCompositorRunnerApplicationClass
    ShoyuCompositorRunnerApplicationClass;

#define SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION                               \
  (shoyu_compositor_runner_application_get_type())
#define SHOYU_COMPOSITOR_RUNNER_APPLICATION(object)                            \
  (G_TYPE_CHECK_INSTANCE_CAST((object),                                        \
                              SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION,        \
                              ShoyuCompositorRunnerApplication))
#define SHOYU_COMPOSITOR_RUNNER_APPLICATION_CLASS(klass)                       \
  (G_TYPE_CHECK_CLASS_CAST((klass), SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION,  \
                           ShoyuCompositorRunnerApplicationClass))
#define SHOYU_COMPOSITOR_RUNNER_IS_APPLICATION(object)                         \
  (G_TYPE_CHECK_INSTANCE_TYPE((object),                                        \
                              SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION))
#define SHOYU_COMPOSITOR_RUNNER_IS_APPLICATION_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_TYPE((klass), SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION))
#define SHOYU_COMPOSITOR_RUNNER_APPLICATION_GET_CLASS(obj)                     \
  (G_TYPE_INSTANCE_GET_CLASS((obj), SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION,  \
                             ShoyuCompositorRunnerApplicationClass))

GType shoyu_compositor_runner_application_get_type(void) G_GNUC_CONST;
GApplication *shoyu_compositor_runner_application_new();

G_END_DECLS
