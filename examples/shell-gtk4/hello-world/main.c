#include <gtk/gtk.h>
#include <shoyu-shell-gtk4/shoyu-shell-gtk4.h>

static void realize(GtkWidget *widget) {
  GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(widget));
  GdkDisplay *display = gdk_surface_get_display(surface);
  GListModel *monitors = gdk_display_get_monitors(display);

  shoyu_shell_gtk_monitor_set_surface(
      GDK_MONITOR(g_list_model_get_object(monitors, 0)), surface);
}

static void activate(GApplication *application) {
  GtkApplicationWindow *win =
      gtk_application_window_new(GTK_APPLICATION(application));
  gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
  g_signal_connect(win, "realize", G_CALLBACK(realize), NULL);
  gtk_window_present(GTK_WINDOW(win));

  gtk_window_set_child(GTK_WINDOW(win), gtk_label_new("Hello, world"));
}

int main(int argc, char **argv) {
  gtk_init();
  shoyu_shell_gtk_init();

  GtkApplication *app =
      gtk_application_new("com.midstall.shoyu.Shell", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
