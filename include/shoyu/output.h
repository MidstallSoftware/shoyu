#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

#include "compositor.h"

G_BEGIN_DECLS;

G_DECLARE_DERIVABLE_TYPE(ShoyuOutput, shoyu_output, SHOYU, OUTPUT, GtkBin);

struct _ShoyuCompositor;

struct _ShoyuOutputClass {
  GtkBinClass parent_class;

  gboolean (*request_state)(ShoyuOutput* self, struct wlr_output_state* state);

  gpointer padding[12];
};

ShoyuOutput* shoyu_output_new(struct _ShoyuCompositor* compositor, struct wlr_output* wlr_output);
void shoyu_output_invalidate_view(ShoyuOutput* self);

G_END_DECLS;
