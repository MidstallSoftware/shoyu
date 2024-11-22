#include "shoyu-config.h"
#include "main.h"

#include <glib/gi18n-lib.h>
#include <wlr/util/log.h>

static gboolean shoyu_initialized = FALSE;

static void log_handler(enum wlr_log_importance importance, const char* fmt, va_list args) {
  switch (importance) {
    case WLR_SILENT:
    case WLR_LOG_IMPORTANCE_LAST:
      break;
    case WLR_ERROR:
      g_logv("wlroots", G_LOG_LEVEL_ERROR, fmt, args);
      break;
    case WLR_INFO:
      g_logv("wlroots", G_LOG_LEVEL_INFO, fmt, args);
      break;
    case WLR_DEBUG:
      g_logv("wlroots", G_LOG_LEVEL_DEBUG, fmt, args);
      break;
  }
}

static void setlocale_initialization(void) {
  static gboolean initialized = FALSE;

  if (initialized)
    return;
  initialized = TRUE;

  if (!setlocale(LC_ALL, "")) {
    g_warning("Locale not supported by C library.\n\tUsing the fallback 'C' locale.");
  }
}

static void gettext_initialization(void) {
  setlocale_initialization();

  bindtextdomain(GETTEXT_PACKAGE, SHOYU_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
}

void shoyu_init(void) {
  if (!shoyu_init_check()) {
    g_warning("Failed to initialize");
    exit(1);
  }
}

gboolean shoyu_init_check(void) {
  if (shoyu_initialized) return TRUE;

  gettext_initialization();
  wlr_log_init(WLR_DEBUG, log_handler);

  shoyu_initialized = TRUE;
  return TRUE;
}

gboolean shoyu_is_initialized(void) {
  return shoyu_initialized;
}
