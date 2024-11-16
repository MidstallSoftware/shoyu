#include "config.h"
#include "version.h"

/**
 * shoyu_shell_gtk_get_major_version:
 *
 * Returns the major version number of the Shoyu Compositor library.
 *
 * Returns: the major version number of the Shoyu Compositor library
 */
guint shoyu_shell_gtk_get_major_version(void) {
  return SHOYU_SHELL_GTK_MAJOR_VERSION;
}

/**
 * shoyu_shell_gtk_get_minor_version:
 *
 * Returns the minor version number of the Shoyu Compositor library.
 *
 * Returns: the minor version number of the Shoyu Compositor library
 */
guint shoyu_shell_gtk_get_minor_version(void) {
  return SHOYU_SHELL_GTK_MINOR_VERSION;
}

/**
 * shoyu_shell_gtk_get_micro_version:
 *
 * Returns the micro version number of the Shoyu Compositor library.
 *
 * Returns: the micro version number of the Shoyu Compositor library
 */
guint shoyu_shell_gtk_get_micro_version(void) {
  return SHOYU_SHELL_GTK_MICRO_VERSION;
}

/**
 * shoyu_shell_gtk_get_binary_age:
 *
 * Returns the binary age as passed to `libtool`.
 *
 * If `libtool` means nothing to you, don't worry about it.
 *
 * Returns: the binary age of the Shoyu Shell GTK 3 library
 */
guint shoyu_shell_gtk_get_binary_age(void) {
  return SHOYU_SHELL_GTK_MICRO_VERSION;
}

/**
 * shoyu_shell_gtk_get_interface_age:
 *
 * Returns the interface age as passed to `libtool`.
 *
 * If `libtool` means nothing to you, don't worry about it.
 *
 * Returns: the interface age of the Shoyu Shell GTK 3 library
 */
guint shoyu_shell_gtk_get_interface_age(void) {
  return SHOYU_SHELL_GTK_MICRO_VERSION;
}

/**
 * shoyu_shell_gtk_check_version:
 * @required_major: the required major version
 * @required_minor: the required minor version
 * @required_micro: the required micro version
 *
 * Checks that the Shoyu Shell GTK 3 library in use is compatible with the
 * given version.
 *
 * Returns: (nullable): %NULL if the Shoyu Compositor library is compatible with the
 *   given version, or a string describing the version mismatch.
 *   The returned string is owned by Shoyu Compositor and should not be modified
 *   or freed.
 */
const char* shoyu_shell_gtk_check_version(guint required_major, guint required_minor, guint required_micro) {
  int shoyu_shell_gtk_effective_micro = 100 * SHOYU_SHELL_GTK_MINOR_VERSION + SHOYU_SHELL_GTK_MICRO_VERSION;
  int required_effective_micro = 100 * required_minor + required_micro;

  if (required_major > SHOYU_SHELL_GTK_MAJOR_VERSION)
    return "Shoyu Compositor version too old (major mismatch)";
  if (required_major < SHOYU_SHELL_GTK_MAJOR_VERSION)
    return "Shoyu Compositor version too new (major mismatch)";
  if (required_effective_micro < shoyu_shell_gtk_effective_micro - SHOYU_SHELL_GTK_BINARY_AGE)
    return "Shoyu Compositor version too new (micro mismatch)";
  if (required_effective_micro > shoyu_shell_gtk_effective_micro)
    return "Shoyu Compositor version too old (micro mismatch)";
  return NULL;
}
