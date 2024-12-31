#include "application.h"

int main(int argc, char **argv) {
  shoyu_init();

  g_autoptr(GApplication) app = shoyu_compositor_runner_application_new();
  return g_application_run(app, argc, argv);
}
