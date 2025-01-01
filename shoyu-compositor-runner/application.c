#include "application.h"
#include <shoyu-compositor/shoyu-compositor.h>
#include <shoyu-config.h>

struct _ShoyuCompositorRunnerApplication {
    GApplication parent_instance;
    ShoyuCompositor *compositor;
    gchar **argv;
    GSubprocess *shell;
};

struct _ShoyuCompositorRunnerApplicationClass {
    GApplicationClass parent_class;
};

G_DEFINE_TYPE(ShoyuCompositorRunnerApplication,
              shoyu_compositor_runner_application, G_TYPE_APPLICATION)

static void shoyu_compositor_runner_application_constructed(GObject *object) {
  G_OBJECT_CLASS(shoyu_compositor_runner_application_parent_class)
      ->constructed(object);

  ShoyuCompositorRunnerApplication *self =
      SHOYU_COMPOSITOR_RUNNER_APPLICATION(object);

  self->compositor = shoyu_compositor_new_with_application(G_APPLICATION(self));

  g_setenv("WAYLAND_DISPLAY", shoyu_compositor_get_socket(self->compositor),
           TRUE);
}

static void shoyu_compositor_runner_application_finalize(GObject *object) {
  ShoyuCompositorRunnerApplication *self =
      SHOYU_COMPOSITOR_RUNNER_APPLICATION(object);

  g_clear_object(&self->compositor);
  g_clear_object(&self->shell);
  g_clear_pointer(&self->argv, (GDestroyNotify)g_strfreev);

  G_OBJECT_CLASS(shoyu_compositor_runner_application_parent_class)
      ->finalize(object);
}

static void
shoyu_compositor_runner_application_activate(GApplication *application) {
  ShoyuCompositorRunnerApplication *self =
      SHOYU_COMPOSITOR_RUNNER_APPLICATION(application);

  shoyu_compositor_start(self->compositor);

  if (self->argv != NULL) {
    GError *error = NULL;
    self->shell =
        g_subprocess_newv(self->argv,
                          G_SUBPROCESS_FLAGS_STDIN_INHERIT |
                              G_SUBPROCESS_FLAGS_SEARCH_PATH_FROM_ENVP |
                              G_SUBPROCESS_FLAGS_INHERIT_FDS,
                          &error);
    if (self->shell == NULL) {
      g_error("Failed to launch the shell process: %s", error->message);
      g_error_free(error);
      return;
    }
  }
}

static int shoyu_compositor_runner_application_command_line(
    GApplication *application, GApplicationCommandLine *cmdline) {
  ShoyuCompositorRunnerApplication *self =
      SHOYU_COMPOSITOR_RUNNER_APPLICATION(application);

  int argc = 0;
  gchar **argv = g_application_command_line_get_arguments(cmdline, &argc);
  g_assert(argv != NULL && argc > 0);

  GOptionContext *context = g_option_context_new(NULL);
  g_option_context_set_help_enabled(context, TRUE);
  g_option_context_set_summary(
      context, "Starts a Wayland compositor for a Shoyu Shell application.");

  GError *error = NULL;
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_application_command_line_printerr(cmdline, "%s\n", error->message);
    g_error_free(error);
    g_application_command_line_set_exit_status(cmdline, 1);
    return 0;
  }

  self->argv =
      argv > 1 ? g_strdupv(argv + (g_strcmp0(argv[0], "--") ? 2 : 1)) : NULL;

  g_free(argv);
  g_option_context_free(context);

  g_application_activate(application);
  return 0;
}

static void shoyu_compositor_runner_application_class_init(
    ShoyuCompositorRunnerApplicationClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GApplicationClass *application_class = G_APPLICATION_CLASS(class);

  object_class->constructed = shoyu_compositor_runner_application_constructed;
  object_class->finalize = shoyu_compositor_runner_application_finalize;

  application_class->activate = shoyu_compositor_runner_application_activate;
  application_class->command_line =
      shoyu_compositor_runner_application_command_line;
}

static void shoyu_compositor_runner_application_init(
    ShoyuCompositorRunnerApplication *self) {
  GApplication *application = G_APPLICATION(self);

  g_application_set_flags(application, G_APPLICATION_HANDLES_COMMAND_LINE);
}

GApplication *shoyu_compositor_runner_application_new() {
  return G_APPLICATION(g_object_new(SHOYU_COMPOSITOR_RUNNER_TYPE_APPLICATION,
                                    "application-id",
                                    "com.midstall.shoyu.Compositor", NULL));
}
