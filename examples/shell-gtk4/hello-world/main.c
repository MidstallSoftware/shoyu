#include <shoyu-shell-gtk4/shoyu-shell-gtk4.h>
#include <gtk/gtk.h>

static void activate(GApplication* application) {
  g_application_hold(application);
}

int main(int argc, char** argv) {
  gtk_init();
  shoyu_shell_gtk_init();

  GtkApplication* app = gtk_application_new("com.midstall.shoyu.Shell", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
