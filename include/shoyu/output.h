#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS;

#if GTK_MAJOR_VERSION == 3
G_DECLARE_DERIVABLE_TYPE(ShoyuOutput, shoyu_output, SHOYU, OUTPUT, GtkBin);
#elif GTK_MAJOR_VERSION == 4
G_DECLARE_DERIVABLE_TYPE(ShoyuOutput, shoyu_output, SHOYU, OUTPUT, GtkWidget);
#endif

struct _ShoyuCompositor;

struct _ShoyuOutputClass {
#if GTK_MAJOR_VERSION == 3
  GtkBinClass parent_class;
#elif GTK_MAJOR_VERSION == 4
  GtkWidgetClass parent_class;
#endif

  gboolean (*request_state)(ShoyuOutput* self, struct wlr_output_state* state);

  gpointer padding[12];
};

ShoyuOutput* shoyu_output_new(struct _ShoyuCompositor* compositor, struct wlr_output* wlr_output);

G_END_DECLS;
