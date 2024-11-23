#include "shoyu-config.h"

#include "compositor-private.h"
#include "input-private.h"
#include "output-private.h"
#include "shell-private.h"
#include "surface-private.h"
#include "wayland-event-source.h"
#include "xdg-toplevel-private.h"

#include <glib/gi18n-lib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>

/**
 * ShoyuCompositor:
 *
 * A wlroots based Wayland compositor.
 */

enum {
  PROP_0 = 0,
  PROP_APPLICATION,
  PROP_SOCKET,
  PROP_SHELL,
#ifdef SHOYU_COLORD
  PROP_COLORD_CLIENT,
#endif
  N_PROPERTIES,

  SIG_OUTPUT_ADDED = 0,
  SIG_OUTPUT_REMOVED,
  SIG_INPUT_ADDED,
  SIG_INPUT_REMOVED,
  SIG_SURFACE_ADDED,
  SIG_SURFACE_REMOVED,
  SIG_XDG_TOPLEVEL_ADDED,
  SIG_XDG_TOPLEVEL_REMOVED,
  SIG_STARTED,
  N_SIGNALS,
};

static GParamSpec *shoyu_compositor_props[N_PROPERTIES] = {
  NULL,
};
static guint shoyu_compositor_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuCompositor, shoyu_compositor, G_TYPE_OBJECT)

static void shoyu_compositor_destroy_output(ShoyuOutput *output,
                                            ShoyuCompositor *self) {
  g_signal_emit(self, shoyu_compositor_sigs[SIG_OUTPUT_REMOVED], 0, output);

  guint len = g_list_length(self->outputs);
  self->outputs = g_list_remove(self->outputs, output);

  guint new_len = g_list_length(self->outputs);
  g_debug(_("Outputs changed (old: %u, new: %u)"), len, new_len);
  g_assert(new_len < len);

  if (self->application != NULL) {
    g_application_release(self->application);
  }

  g_debug(_("Destroyed ShoyuOutput#%p"), output);
  g_object_unref(output);
}

static void shoyu_compositor_destroy_input(ShoyuInput *input,
                                           ShoyuCompositor *self) {
  g_signal_emit(self, shoyu_compositor_sigs[SIG_INPUT_REMOVED], 0, input);

  guint len = g_list_length(self->inputs);
  self->inputs = g_list_remove(self->inputs, input);

  guint new_len = g_list_length(self->inputs);
  g_debug(_("Inputs changed (old: %u, new: %u)"), len, new_len);
  g_assert(new_len < len);

  g_debug(_("Destroyed ShoyuInput#%p"), input);
  g_object_unref(input);
}

static void shoyu_compositor_destroy_surface(ShoyuSurface *surface,
                                             ShoyuCompositor *self) {
  g_signal_emit(self, shoyu_compositor_sigs[SIG_SURFACE_REMOVED], 0, surface);

  guint len = g_list_length(self->surfaces);
  self->surfaces = g_list_remove(self->surfaces, surface);

  guint new_len = g_list_length(self->surfaces);
  g_debug(_("Surfaces changed (old: %u, new: %u)"), len, new_len);
  g_assert(new_len < len);

  g_debug(_("Destroyed ShoyuSurface#%p"), surface);
  g_object_unref(surface);
}

static void shoyu_compositor_destroy_xdg_toplevel(ShoyuXdgToplevel *toplevel,
                                                  ShoyuCompositor *self) {
  g_signal_emit(self, shoyu_compositor_sigs[SIG_XDG_TOPLEVEL_REMOVED], 0,
                toplevel);

  guint len = g_list_length(self->xdg_toplevels);
  self->xdg_toplevels = g_list_remove(self->xdg_toplevels, toplevel);

  guint new_len = g_list_length(self->xdg_toplevels);
  g_debug(_("XDG toplevels changed (old: %u, new: %u)"), len, new_len);
  g_assert(new_len < len);

  g_debug(_("Destroyed ShoyuXdgToplevel#%p"), toplevel);
  g_object_unref(toplevel);
}

static void shoyu_compositor_new_output(struct wl_listener *listener,
                                        void *data) {
  ShoyuCompositor *self = wl_container_of(listener, self, new_output);
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  struct wlr_output *wlr_output = data;

  wlr_output_init_render(wlr_output, self->wlr_allocator, self->wlr_renderer);

  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);

  struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
  if (mode != NULL) {
    wlr_output_state_set_mode(&state, mode);
  }

  wlr_output_commit_state(wlr_output, &state);
  wlr_output_state_finish(&state);

  g_return_if_fail(class->create_output != NULL);

  ShoyuOutput *output = class->create_output(self, wlr_output);
  if (output == NULL)
    return;

  g_signal_connect(output, "destroy",
                   G_CALLBACK(shoyu_compositor_destroy_output), self);

  shoyu_output_realize(output, wlr_output);

  guint len = g_list_length(self->outputs);
  self->outputs = g_list_append(self->outputs, output);

  guint new_len = g_list_length(self->outputs);
  g_debug(_("Outputs changed (old: %u, new: %u)"), len, new_len);
  g_assert(new_len > len);

  g_debug(_("Created ShoyuOutput#%p"), output);

  if (self->application != NULL) {
    g_application_hold(self->application);
  }

  g_signal_emit(self, shoyu_compositor_sigs[SIG_OUTPUT_ADDED], 0, output);
}

static void shoyu_compositor_new_input(struct wl_listener *listener,
                                       void *data) {
  ShoyuCompositor *self = wl_container_of(listener, self, new_input);
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  struct wlr_input_device *wlr_input_device = data;

  g_return_if_fail(class->create_input != NULL);

  ShoyuInput *input = class->create_input(self, wlr_input_device);
  if (input == NULL)
    return;

  g_signal_connect(input, "destroy", G_CALLBACK(shoyu_compositor_destroy_input),
                   self);

  shoyu_input_realize(input, wlr_input_device);

  guint len = g_list_length(self->inputs);
  self->inputs = g_list_append(self->inputs, input);

  guint new_len = g_list_length(self->inputs);
  g_debug("Inputs changed (old: %u, new: %u)", len, new_len);
  g_assert(new_len > len);

  g_debug("Created ShoyuInput#%p", input);
  g_signal_emit(self, shoyu_compositor_sigs[SIG_INPUT_ADDED], 0, input);
}

static void shoyu_compositor_new_surface(struct wl_listener *listener,
                                         void *data) {
  ShoyuCompositor *self = wl_container_of(listener, self, new_surface);
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  struct wlr_surface *wlr_surface = data;

  g_return_if_fail(class->create_surface != NULL);

  ShoyuSurface *surface = class->create_surface(self, wlr_surface);
  if (surface == NULL)
    return;

  g_signal_connect(surface, "destroy",
                   G_CALLBACK(shoyu_compositor_destroy_surface), self);

  shoyu_surface_realize(surface, wlr_surface);

  guint len = g_list_length(self->surfaces);
  self->surfaces = g_list_append(self->surfaces, surface);

  guint new_len = g_list_length(self->surfaces);
  g_debug("Surfaces changed (old: %u, new: %u)", len, new_len);
  g_assert(new_len > len);

  g_debug("Created ShoyuSurface#%p", surface);
  g_signal_emit(self, shoyu_compositor_sigs[SIG_SURFACE_ADDED], 0, surface);
}

static void shoyu_compositor_new_xdg_toplevel(struct wl_listener *listener,
                                              void *data) {
  ShoyuCompositor *self = wl_container_of(listener, self, new_xdg_toplevel);
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  struct wlr_xdg_toplevel *wlr_xdg_toplevel = data;

  g_return_if_fail(class->create_xdg_toplevel != NULL);

  ShoyuXdgToplevel *toplevel =
      class->create_xdg_toplevel(self, wlr_xdg_toplevel);
  if (toplevel == NULL)
    return;

  g_signal_connect(toplevel, "destroy",
                   G_CALLBACK(shoyu_compositor_destroy_xdg_toplevel), self);

  shoyu_xdg_toplevel_realize(toplevel, wlr_xdg_toplevel);

  guint len = g_list_length(self->xdg_toplevels);
  self->xdg_toplevels = g_list_append(self->xdg_toplevels, toplevel);

  guint new_len = g_list_length(self->xdg_toplevels);
  g_debug("XDG toplevels changed (old: %u, new: %u)", len, new_len);
  g_assert(new_len > len);

  g_debug("Created ShoyuXdgToplevel#%p", toplevel);
  g_signal_emit(self, shoyu_compositor_sigs[SIG_XDG_TOPLEVEL_ADDED], 0,
                toplevel);
}

static void shoyu_compositor_constructed(GObject *object) {
  G_OBJECT_CLASS(shoyu_compositor_parent_class)->constructed(object);

  ShoyuCompositor *self = SHOYU_COMPOSITOR(object);
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

#ifdef SHOYU_COLORD
  self->colord = cd_client_new();

  GError *error = NULL;
  if (!cd_client_connect_sync(self->colord, NULL, &error)) {
    g_warning("Cannot connect to colord: %s", error->message);
    g_clear_object(&self->colord);
  }
#endif

  g_assert(class->create_backend != NULL);
  self->wlr_backend =
      class->create_backend(self, wl_display_get_event_loop(self->wl_display));
  g_assert(self->wlr_backend != NULL);

  g_assert(class->create_renderer != NULL);
  self->wlr_renderer = class->create_renderer(self, self->wlr_backend);
  g_assert(self->wlr_renderer != NULL);

  wlr_renderer_init_wl_display(self->wlr_renderer, self->wl_display);

  g_assert(class->create_allocator != NULL);
  self->wlr_allocator =
      class->create_allocator(self, self->wlr_backend, self->wlr_renderer);
  g_assert(self->wlr_allocator != NULL);

  self->wlr_compositor =
      wlr_compositor_create(self->wl_display, 5, self->wlr_renderer);
  g_assert(self->wlr_compositor != NULL);

  self->new_surface.notify = shoyu_compositor_new_surface;
  wl_signal_add(&self->wlr_compositor->events.new_surface, &self->new_surface);

  self->new_output.notify = shoyu_compositor_new_output;
  wl_signal_add(&self->wlr_backend->events.new_output, &self->new_output);

  self->new_input.notify = shoyu_compositor_new_input;
  wl_signal_add(&self->wlr_backend->events.new_input, &self->new_input);

  self->new_xdg_toplevel.notify = shoyu_compositor_new_xdg_toplevel;
  wl_signal_add(&self->wlr_xdg_shell->events.new_toplevel,
                &self->new_xdg_toplevel);
}

static void shoyu_compositor_finalize(GObject *object) {
  ShoyuCompositor *self = SHOYU_COMPOSITOR(object);

  g_clear_list(&self->outputs, (GDestroyNotify)g_object_unref);
  g_clear_list(&self->inputs, (GDestroyNotify)g_object_unref);
  g_clear_list(&self->surfaces, (GDestroyNotify)g_object_unref);
  g_clear_list(&self->xdg_toplevels, (GDestroyNotify)g_object_unref);

  g_clear_object(&self->shell);

#ifdef SHOYU_COLORD
  g_clear_object(&self->colord);
#endif

  g_clear_pointer(&self->wlr_allocator, (GDestroyNotify)wlr_allocator_destroy);
  g_clear_pointer(&self->wlr_renderer, (GDestroyNotify)wlr_renderer_destroy);
  g_clear_pointer(&self->wlr_backend, (GDestroyNotify)wlr_backend_destroy);
  g_clear_pointer(&self->wl_source, (GDestroyNotify)g_source_destroy);
  g_clear_pointer(&self->wl_display, (GDestroyNotify)wl_display_destroy);

  g_clear_object(&self->application);

  G_OBJECT_CLASS(shoyu_compositor_parent_class)->finalize(object);
}

static void shoyu_compositor_set_property(GObject *object, guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  ShoyuCompositor *self = SHOYU_COMPOSITOR(object);

  switch (prop_id) {
    case PROP_APPLICATION:
      self->application = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_compositor_get_property(GObject *object, guint prop_id,
                                          GValue *value, GParamSpec *pspec) {
  ShoyuCompositor *self = SHOYU_COMPOSITOR(object);

  switch (prop_id) {
    case PROP_APPLICATION:
      g_value_set_object(value, G_OBJECT(self->application));
      break;
    case PROP_SOCKET:
      g_value_set_string(value, self->socket);
      break;
    case PROP_SHELL:
      g_value_set_object(value, G_OBJECT(self->shell));
      break;
#ifdef SHOYU_COLORD
    case PROP_COLORD_CLIENT:
      g_value_set_object(value, G_OBJECT(self->colord));
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static struct wlr_backend *
shoyu_compositor_real_create_backend(ShoyuCompositor *self,
                                     struct wl_event_loop *event_loop) {
  return wlr_backend_autocreate(event_loop, NULL);
}

static struct wlr_renderer *
shoyu_compositor_real_create_renderer(ShoyuCompositor *self,
                                      struct wlr_backend *backend) {
  return wlr_renderer_autocreate(backend);
}

static struct wlr_allocator *
shoyu_compositor_real_create_allocator(ShoyuCompositor *self,
                                       struct wlr_backend *backend,
                                       struct wlr_renderer *renderer) {
  return wlr_allocator_autocreate(backend, renderer);
}

static ShoyuOutput *
shoyu_compositor_real_create_output(ShoyuCompositor *self,
                                    struct wlr_output *wlr_output) {
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  ShoyuOutput *output =
      g_object_new(class->output_type, "compositor", self, NULL);
  g_return_val_if_fail(output != NULL, NULL);
  return output;
}

static ShoyuInput *
shoyu_compositor_real_create_input(ShoyuCompositor *self,
                                   struct wlr_input_device *input_device) {
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  ShoyuInput *input = g_object_new(class->input_type, "compositor", self, NULL);
  g_return_val_if_fail(input != NULL, NULL);
  return input;
}

static ShoyuSurface *
shoyu_compositor_real_create_surface(ShoyuCompositor *self,
                                     struct wlr_surface *wlr_surface) {
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  ShoyuSurface *surface =
      g_object_new(class->surface_type, "compositor", self, NULL);
  g_return_val_if_fail(surface != NULL, NULL);
  return surface;
}

static ShoyuXdgToplevel *shoyu_compositor_real_create_xdg_toplevel(
    ShoyuCompositor *self, struct wlr_xdg_toplevel *wlr_xdg_toplevel) {
  ShoyuCompositorClass *class = SHOYU_COMPOSITOR_GET_CLASS(self);

  ShoyuXdgToplevel *toplevel =
      g_object_new(class->xdg_toplevel_type, "compositor", self, NULL);
  g_return_val_if_fail(toplevel != NULL, NULL);
  return toplevel;
}

static void shoyu_compositor_class_init(ShoyuCompositorClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->constructed = shoyu_compositor_constructed;
  object_class->finalize = shoyu_compositor_finalize;
  object_class->set_property = shoyu_compositor_set_property;
  object_class->get_property = shoyu_compositor_get_property;

  class->output_type = SHOYU_TYPE_OUTPUT;
  class->input_type = SHOYU_TYPE_INPUT;
  class->surface_type = SHOYU_TYPE_SURFACE;
  class->xdg_toplevel_type = SHOYU_TYPE_XDG_TOPLEVEL;

  class->create_backend = shoyu_compositor_real_create_backend;
  class->create_renderer = shoyu_compositor_real_create_renderer;
  class->create_allocator = shoyu_compositor_real_create_allocator;
  class->create_output = shoyu_compositor_real_create_output;
  class->create_input = shoyu_compositor_real_create_input;
  class->create_surface = shoyu_compositor_real_create_surface;
  class->create_xdg_toplevel = shoyu_compositor_real_create_xdg_toplevel;

  shoyu_compositor_props[PROP_APPLICATION] = g_param_spec_object(
      "application", "Gio Application",
      "The application to run the compositor on.", G_TYPE_APPLICATION,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  shoyu_compositor_props[PROP_SOCKET] = g_param_spec_string(
      "socket", "Wayland Socket", "The name of the Wayland display socket.",
      NULL, G_PARAM_READABLE);

  shoyu_compositor_props[PROP_SHELL] = g_param_spec_object(
      "shell", "Shoyu Shell", "The shell to run with the compositor.",
      SHOYU_TYPE_SHELL, G_PARAM_READABLE);

#ifdef SHOYU_COLORD
  shoyu_compositor_props[PROP_COLORD_CLIENT] = g_param_spec_object(
      "colord-client", "Colord Client", "The colord client for the compositor",
      CD_TYPE_CLIENT, G_PARAM_READABLE);
#endif

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_compositor_props);

  /**
   * ShoyuCompositor::output-added:
   * @compositor: the object which received the signal
   * @output: a #ShoyuOutput
   */
  shoyu_compositor_sigs[SIG_OUTPUT_ADDED] =
      g_signal_new("output-added", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, output_added), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_OUTPUT);

  /**
   * ShoyuCompositor::output-removed:
   * @compositor: the object which received the signal
   * @output: a #ShoyuOutput
   */
  shoyu_compositor_sigs[SIG_OUTPUT_REMOVED] =
      g_signal_new("output-removed", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, output_removed), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_OUTPUT);

  /**
   * ShoyuCompositor::input-added:
   * @compositor: the object which received the signal
   * @output: a #ShoyuInput
   */
  shoyu_compositor_sigs[SIG_INPUT_ADDED] =
      g_signal_new("input-added", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, input_added), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_INPUT);

  /**
   * ShoyuCompositor::input-removed:
   * @compositor: the object which received the signal
   * @output: a #ShoyuInput
   */
  shoyu_compositor_sigs[SIG_INPUT_REMOVED] =
      g_signal_new("input-removed", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, input_removed), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_INPUT);

  /**
   * ShoyuCompositor::surface-added:
   * @compositor: the object which received the signal
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_compositor_sigs[SIG_SURFACE_ADDED] =
      g_signal_new("surface-added", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, surface_added), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_SURFACE);

  /**
   * ShoyuCompositor::surface-removed:
   * @compositor: the object which received the signal
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_compositor_sigs[SIG_SURFACE_REMOVED] =
      g_signal_new("surface-removed", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, surface_removed), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, SHOYU_TYPE_SURFACE);

  /**
   * ShoyuCompositor::xdg-toplevel-added:
   * @compositor: the object which received the signal
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_compositor_sigs[SIG_XDG_TOPLEVEL_ADDED] = g_signal_new(
      "xdg-toplevel-added", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuCompositorClass, xdg_toplevel_added), NULL, NULL,
      NULL, G_TYPE_NONE, 1, SHOYU_TYPE_XDG_TOPLEVEL);

  /**
   * ShoyuCompositor::xdg-toplevel-removed:
   * @compositor: the object which received the signal
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_compositor_sigs[SIG_XDG_TOPLEVEL_REMOVED] = g_signal_new(
      "xdg-toplevel-removed", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuCompositorClass, xdg_toplevel_removed), NULL, NULL,
      NULL, G_TYPE_NONE, 1, SHOYU_TYPE_XDG_TOPLEVEL);

  /**
   * ShoyuCompositor::started:
   * @compositor: the object which received the signal
   */
  shoyu_compositor_sigs[SIG_STARTED] =
      g_signal_new("started", SHOYU_TYPE_COMPOSITOR, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuCompositorClass, started), NULL, NULL,
                   NULL, G_TYPE_NONE, 0);
}

static void shoyu_compositor_init(ShoyuCompositor *self) {
  self->wl_display = wl_display_create();
  g_assert(self->wl_display != NULL);

  self->socket = wl_display_add_socket_auto(self->wl_display);
  g_assert(self->socket != NULL);

  g_debug("ShoyuCompositor#%p has socket %s", self, self->socket);

  self->wl_source = shoyu_wayland_event_source_new(
      self->wl_display, wl_display_get_event_loop(self->wl_display));
  g_assert(self->wl_source != NULL);

  self->outputs = NULL;

  self->wlr_xdg_shell = wlr_xdg_shell_create(self->wl_display, 3);
  g_assert(self->wlr_xdg_shell != NULL);

  self->output_layout = wlr_output_layout_create(self->wl_display);
  g_assert(self->output_layout != NULL);

  self->shell = shoyu_shell_new(self);
  g_assert(self->shell != NULL);

  wlr_subcompositor_create(self->wl_display);
  wlr_data_device_manager_create(self->wl_display);
}

/**
 * shoyu_compositor_new: (constructor)
 *
 * Creates a #ShoyuCompositor
 *
 * Returns: (transfer full): A #ShoyuCompositor
 */
ShoyuCompositor *shoyu_compositor_new(void) {
  return SHOYU_COMPOSITOR(g_object_new(SHOYU_TYPE_COMPOSITOR, NULL));
}

/**
 * shoyu_compositor_new_with_application: (constructor)
 * @application: A #GApplication
 *
 * Creates a #ShoyuCompositor which binds to a #GApplication.
 *
 * By binding to a #GApplication, the compositor will hold the
 * application open as long as there are monitors. Once the last
 * monitor is removed, the compositor will then shutdown the
 * application.
 *
 * Returns: (transfer full): A #ShoyuCompositor
 */
ShoyuCompositor *
shoyu_compositor_new_with_application(GApplication *application) {
  return SHOYU_COMPOSITOR(
      g_object_new(SHOYU_TYPE_COMPOSITOR, "application", application, NULL));
}

/**
 * shoyu_compositor_get_application:
 * @self: A #ShoyuCompositor
 *
 * Gets the #GApplication associated with the compositor.
 *
 * Returns: (transfer none) (nullable): A #GApplication
 */
GApplication *shoyu_compositor_get_application(ShoyuCompositor *self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);
  return self->application;
}

/**
 * shoyu_compositor_get_socket:
 * @self: A #ShoyuCompositor
 *
 * Gets the Wayland display socket name for the compositor.
 *
 * Returns: (transfer none): A string
 */
const char *shoyu_compositor_get_socket(ShoyuCompositor *self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);
  return self->socket;
}

/**
 * shoyu_compositor_start:
 * @self: A #ShoyuCompositor
 *
 * Starts processing events for the Wayland server.
 *
 * Returns: %TRUE if the server has started, %FALSE if
 * it failed to start.
 */
gboolean shoyu_compositor_start(ShoyuCompositor *self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), FALSE);

  if (!wlr_backend_start(self->wlr_backend))
    return FALSE;

  g_signal_emit(self, shoyu_compositor_sigs[SIG_STARTED], 0);
  return TRUE;
}

/**
 * shoyu_compositor_get_shell:
 * @self: A #ShoyuCompositor
 *
 * Returns: (transfer none): A #ShoyuShell
 */
ShoyuShell *shoyu_compositor_get_shell(ShoyuCompositor *self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), FALSE);
  return self->shell;
}

ShoyuOutput *shoyu_compositor_get_output(ShoyuCompositor *self,
                                         struct wlr_output *wlr_output) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);

  for (GList *item = self->outputs; item != NULL; item = item->next) {
    ShoyuOutput *output = SHOYU_OUTPUT(item->data);

    if (output->is_invalidated)
      continue;
    if (output->wlr_output == wlr_output)
      return output;
  }

  return NULL;
}

ShoyuSurface *shoyu_compositor_get_surface(ShoyuCompositor *self,
                                           struct wlr_surface *wlr_surface) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);

  for (GList *item = self->surfaces; item != NULL; item = item->next) {
    ShoyuSurface *surface = SHOYU_SURFACE(item->data);

    if (surface->is_invalidated)
      continue;
    if (surface->wlr_surface == wlr_surface)
      return surface;
  }

  return NULL;
}

gboolean
shoyu_compositor_is_xdg_toplevel_claimed(ShoyuCompositor *self,
                                         struct xdg_toplevel *xdg_toplevel) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), FALSE);

  for (GList *item = self->outputs; item != NULL; item = item->next) {
    ShoyuOutput *output = SHOYU_OUTPUT(item->data);

    if (output->is_invalidated)
      continue;
    if (output->wlr_surface != NULL) {
      struct wlr_xdg_toplevel *output_xdg_toplevel =
          wlr_xdg_toplevel_try_from_wlr_surface(output->wlr_surface);
      if (output_xdg_toplevel == xdg_toplevel)
        return TRUE;
    }
  }

  return FALSE;
}

ShoyuOutput *shoyu_compositor_get_xdg_toplevel_claimed_output(
    ShoyuCompositor *self, struct xdg_toplevel *xdg_toplevel) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), FALSE);

  for (GList *item = self->outputs; item != NULL; item = item->next) {
    ShoyuOutput *output = SHOYU_OUTPUT(item->data);

    if (output->is_invalidated)
      continue;
    if (output->wlr_surface != NULL) {
      struct wlr_xdg_toplevel *output_xdg_toplevel =
          wlr_xdg_toplevel_try_from_wlr_surface(output->wlr_surface);
      if (output_xdg_toplevel == xdg_toplevel)
        return output;
    }
  }

  return FALSE;
}
