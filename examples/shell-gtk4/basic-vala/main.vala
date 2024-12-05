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

      var monitors = display.get_monitors();
      monitors.items_changed.connect((pos, removed, added) => {
        scan_outputs();
      });

      scan_outputs();
    }

    private void scan_outputs() {
      var monitors = display.get_monitors();
      unowned var windows = get_windows();

      var n_monitors = monitors.get_n_items();
      for (var i = 0; i < n_monitors; i++) {
        var monitor = monitors.get_object(i) as Gdk.Monitor;
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
    private Gtk.ListView toplevels;

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
      ShoyuShellGtk.monitor_set_surface(monitor, get_surface());
      update_geom();

      var factory = new Gtk.SignalListItemFactory();
      factory.setup.connect((obj) => {
        var item = obj as Gtk.ListItem;
        var image = new Gtk.Image();
        image.set_pixel_size(monitor.geometry.width / 2);
        item.set_child(image);
        image.show();
      });

      factory.bind.connect((obj) => {
        var item = obj as Gtk.ListItem;
        var child = item.get_child() as Gtk.Image;
        var toplevel = item.get_item() as ShoyuShellGtk.Toplevel;
        child.set_from_paintable(toplevel.texture);

        toplevel.notify["texture"].connect(() => {
          child.set_from_paintable(toplevel.texture);
        });
      });

      toplevels = new Gtk.ListView(new Gtk.NoSelection(application.shell_display.get_toplevels()), factory);
      set_child(toplevels);
      toplevels.show();
    }

    private void update_geom() {
      var geom = monitor.geometry;
      set_size_request(geom.width, geom.height);
    }
  }
}
