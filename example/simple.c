#include <shoyu/shoyu.h>

static void activate_cb(ShoyuApplication* application) {
  struct wl_display* wl_display = shoyu_application_get_wl_display(application);
  ShoyuCompositor* compositor = shoyu_compositor_new_with_application(wl_display, G_APPLICATION(application));
}

int main(int argc, char** argv) {
  g_autoptr(ShoyuApplication) app = shoyu_application_new("com.midstall.shoyu.example", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
