#include <gtk/gtk.h>
#include <shoyu-shell-gtk3/shoyu-shell-gtk3.h>

static void realize(GtkWidget *widget) {
  GdkWindow *window = gtk_widget_get_window(widget);
  GdkDisplay *display = gdk_window_get_display(window);

  shoyu_shell_gtk_monitor_set_window(gdk_display_get_monitor(display, 0),
                                     window);
}

static void activate(GApplication *application) {
  GtkApplicationWindow *win = GTK_APPLICATION_WINDOW(
      gtk_application_window_new(GTK_APPLICATION(application)));
  gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
  g_signal_connect(win, "realize", G_CALLBACK(realize), NULL);

  gtk_container_add(GTK_CONTAINER(win), gtk_label_new("Hello, world"));
  gtk_widget_show_all(GTK_WIDGET(win));
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);
  shoyu_shell_gtk_init();

  GtkApplication *app =
      gtk_application_new("com.midstall.shoyu.Shell", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
