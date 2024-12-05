namespace ShellGtk4BasicVala {
  public class Application : Gtk.Application {
    private Gdk.Display display;
    public ShoyuShellGtk.Display shell_display;

    public Application() {
      Object(application_id: "com.midstall.shoyu.Shell");
    }

    public override void activate() {
      display = Gdk.Display.get_default();
      shell_display = ShoyuShellGtk.Display.@get(display);

      display.monitor_added.connect((monitor) => {
        scan_outputs();
      });

      display.monitor_removed.connect((monitor) => {
        scan_outputs();
      });

      scan_outputs();
    }

    private void scan_outputs() {
      unowned var windows = get_windows();

      var n_monitors = display.get_n_monitors();
      for (var i = 0; i < n_monitors; i++) {
        var monitor = display.get_monitor(i);
        var has_window = false;

        foreach (var win in windows) {
          if (win is OutputWindow) {
            var owin = win as OutputWindow;
            if (owin.monitor == monitor) {
              has_window = true;
              break;
            }
          }
        }

        if (!has_window) {
          new OutputWindow(this, monitor);
        }
      }
    }

    public static int main(string[] args) {
      var app = new Application();
      return app.run(args);
    }
  }

  public class OutputWindow : Gtk.ApplicationWindow {
    public Gdk.Monitor monitor { get; set construct; }
    private Gtk.ListBox toplevels;

    public OutputWindow(Application application, Gdk.Monitor monitor) {
      Object(application: application, monitor: monitor);

      monitor.notify["geometry"].connect(() => {
        update_geom();
      });

      monitor.invalidate.connect(() => {
        unref();
      });

      decorated = false;
      present();
      ShoyuShellGtk.monitor_set_window(monitor, get_window());
      update_geom();
      
      toplevels = new Gtk.ListBox();
      toplevels.bind_model(application.shell_display.get_toplevels(), (obj) => {
        var toplevel = obj as ShoyuShellGtk.Toplevel;
        var image = new Gtk.Image();
        image.surface = toplevel.surface;

        toplevel.notify["surface"].connect(() => {
          image.clear();
          image.set_from_surface(toplevel.surface);
        });

        image.show();
        return image;
      });
      add(toplevels);
      toplevels.show();
    }

    private void update_geom() {
      var geom = monitor.geometry;
      set_size_request(geom.width, geom.height);
    }
  }
}
