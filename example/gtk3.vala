namespace ShoyuExampleGtk3 {
  public class Application : Shoyu.Application {
    private Compositor _compositor;

    public Shoyu.Compositor compositor {
      get { return _compositor; }
    }

    public Application() {
      Object(application_id: "com.midstall.shoyu.example");
    }

    public override void activate() {
      _compositor = new Compositor.with_application(this.wl_display, this);
    }

    public static int main(string[] args) {
      var app = new Application();
      return app.run(args);
    }
  }

  public class Compositor : Shoyu.Compositor {
    public Shoyu.Output primary {
      owned get { 
        return get_outputs().nth_data(0);
      }
    }

    public Compositor(void* wl_display) {
      Object(wl_display: wl_display);

      this.create_output.connect((output) => new Output(this, output));
    }

    public Compositor.with_application(void* wl_display, GLib.Application application) {
      Object(wl_display: wl_display, application: application);
    }
  }

  public class Output : Shoyu.Output {
    public Output(Shoyu.Compositor compositor, void* output) {
      Object(compositor: compositor, wlr_output: output);

      var label = new Gtk.Label("Hello");
      this.add(label);

      label.show();
      label.set_mapped(true);
    }
  }
}
