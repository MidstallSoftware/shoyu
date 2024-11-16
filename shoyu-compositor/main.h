#pragma once

#if !defined (__SHOYU_COMPOSITOR_H_INSIDE__) && !defined (SHOYU_COMPILATION)
#error "Only <shoyu-compositor/shoyu-compositor.h> can be included directly."
#endif

#include <glib-object.h>
#include <shoyu-compositor/version.h>

G_BEGIN_DECLS

SHOYU_AVAILABLE_IN_ALL
void shoyu_init(void);

SHOYU_AVAILABLE_IN_ALL
gboolean shoyu_init_check(void);

SHOYU_AVAILABLE_IN_ALL
gboolean shoyu_is_initialized(void);

G_END_DECLS
