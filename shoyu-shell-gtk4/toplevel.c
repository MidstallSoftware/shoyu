#include "display-private.h"
#include "toplevel-private.h"

enum {
  PROP_0 = 0,
  PROP_DISPLAY,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_REALIZED,
  SIG_UNREALIZED,
  N_SIGNALS,
};

static GParamSpec *shoyu_shell_gtk_toplevel_props[N_PROPERTIES] = {
  NULL,
};
static guint shoyu_shell_gtk_toplevel_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuShellGtkToplevel, shoyu_shell_gtk_toplevel, G_TYPE_OBJECT)

static void shoyu_shell_gtk_toplevel_drm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t drm_format) {}

static void shoyu_shell_gtk_toplevel_shm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t shm_format) {}

static void shoyu_shell_gtk_toplevel_frame(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t width, uint32_t height) {}

static void shoyu_shell_gtk_toplevel_destroy(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel) {
  ShoyuShellGtkToplevel *toplevel = SHOYU_SHELL_GTK_TOPLEVEL(data);

  if (!toplevel->is_invalidated) {
    shoyu_shell_gtk_toplevel_unrealize(toplevel);
    g_signal_emit(toplevel, shoyu_shell_gtk_toplevel_sigs[SIG_DESTROY], 0);
  }
}

static const struct shoyu_shell_toplevel_listener
    shoyu_shell_toplevel_listener = {
      .drm_format = shoyu_shell_gtk_toplevel_drm_format,
      .shm_format = shoyu_shell_gtk_toplevel_shm_format,
      .frame = shoyu_shell_gtk_toplevel_frame,
      .destroy = shoyu_shell_gtk_toplevel_destroy,
};

static void shoyu_shell_gtk_toplevel_finalize(GObject *object) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  g_clear_object(&self->display);

  G_OBJECT_CLASS(shoyu_shell_gtk_toplevel_parent_class)->finalize(object);
}

static void shoyu_shell_gtk_toplevel_set_property(GObject *object,
                                                  guint prop_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      self->display = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_shell_gtk_toplevel_get_property(GObject *object,
                                                  guint prop_id, GValue *value,
                                                  GParamSpec *pspec) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      g_value_set_object(value, G_OBJECT(self->display));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
shoyu_shell_gtk_toplevel_class_init(ShoyuShellGtkToplevelClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_shell_gtk_toplevel_finalize;
  object_class->set_property = shoyu_shell_gtk_toplevel_set_property;
  object_class->get_property = shoyu_shell_gtk_toplevel_get_property;

  shoyu_shell_gtk_toplevel_props[PROP_DISPLAY] = g_param_spec_object(
      "display", "Shoyu Shell Display",
      "The display the shell is connected to.", SHOYU_SHELL_GTK_TYPE_DISPLAY,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_shell_gtk_toplevel_props);

  /**
   * ShoyuShellGtkToplevel::destroy:
   * @toplevel: the object which received the signal
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_DESTROY] =
      g_signal_new("destroy", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuShellGtkToplevel::realized:
   * @toplevel: a #ShoyuShellGtkToplevel
   * @shoyu_shell_toplevel: A Shoyu Shell toplevel
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_REALIZED] =
      g_signal_new("realized", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuShellGtkToplevelClass, realized), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuShellGtkToplevel::unrealized:
   * @toplevel: a #ShoyuShellGtkToplevel
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_UNREALIZED] = g_signal_new(
      "unrealized", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuShellGtkToplevelClass, unrealized), NULL, NULL, NULL,
      G_TYPE_NONE, 0);
}

static void shoyu_shell_gtk_toplevel_init(ShoyuShellGtkToplevel *self) {
  self->is_invalidated = TRUE;
}

void shoyu_shell_gtk_toplevel_realize(
    ShoyuShellGtkToplevel *self,
    struct shoyu_shell_toplevel *shoyu_shell_toplevel) {
  g_return_if_fail(SHOYU_SHELL_GTK_IS_TOPLEVEL(self));
  g_return_if_fail(self->shoyu_shell_toplevel == NULL && self->is_invalidated);

  self->shoyu_shell_toplevel = shoyu_shell_toplevel;
  self->is_invalidated = FALSE;

  shoyu_shell_toplevel_add_listener(self->shoyu_shell_toplevel,
                                    &shoyu_shell_toplevel_listener, self);

  g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_REALIZED], 0,
                shoyu_shell_toplevel);
}

void shoyu_shell_gtk_toplevel_unrealize(ShoyuShellGtkToplevel *self) {
  g_return_if_fail(SHOYU_SHELL_GTK_IS_TOPLEVEL(self));
  g_return_if_fail(self->shoyu_shell_toplevel != NULL && !self->is_invalidated);

  self->shoyu_shell_toplevel = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_UNREALIZED], 0);
}
