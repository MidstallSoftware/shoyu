libshell_gtk4_version_macros_h = custom_target(
  input: 'versionmacros.h.in',
  output: 'versionmacros.h',
  command: [gen_visibility_macros, meson.project_version(), 'versions-macros', 'SHOYU_SHELL_GTK', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk4/version',
  # FIXME: Not needed with Meson >= 0.64.0
  install_tag: 'devel')

libshell_gtk4_visibility_h = custom_target(
  output: 'visibility.h',
  command: [gen_visibility_macros, meson.project_version(), 'visibility-macros', 'SHOYU_SHELL_GTK', '@OUTPUT@'],
  install: true,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk4/version',
  # FIXME: Not needed with Meson >= 0.64.0
  install_tag: 'devel')
