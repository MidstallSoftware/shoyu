#include <shoyu-compositor/shoyu-compositor.h>

static void activate(GApplication* application) {
  ShoyuCompositor* compositor = shoyu_compositor_new_with_application(application);
  shoyu_compositor_start(compositor);
}

int main(int argc, char** argv) {
  shoyu_init();

  GApplication* app = g_application_new("com.midstall.shoyu.Compositor", G_APPLICATION_NON_UNIQUE);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(app, argc, argv);
  g_object_unref(app);
  return status;
}
