#pragma once

#include <glib-object.h>
#include <gio/gio.h>

#include "compositor.h"

G_BEGIN_DECLS;

G_DECLARE_DERIVABLE_TYPE(ShoyuApplication, shoyu_application, SHOYU, APPLICATION, GApplication);

struct _ShoyuApplicationClass {
  GApplicationClass parent_class;
};

ShoyuApplication* shoyu_application_new(const gchar* application_id, GApplicationFlags flags);
struct wl_display* shoyu_application_get_wl_display(ShoyuApplication* self);

G_END_DECLS;
