libcompositor_version_macros_h = custom_target(
  input: 'versionmacros.h.in',
  output: 'versionmacros.h',
  command: [gen_visibility_macros, meson.project_version(), 'versions-macros', 'SHOYU_COMPOSITOR', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: shoyu_includedir / 'shoyu-compositor/version',
  # FIXME: Not needed with Meson >= 0.64.0
  install_tag: 'devel')

libcompositor_visibility_h = custom_target(
  output: 'visibility.h',
  command: [gen_visibility_macros, meson.project_version(), 'visibility-macros', 'SHOYU_COMPOSITOR', '@OUTPUT@'],
  install: true,
  install_dir: shoyu_includedir / 'shoyu-compositor/version',
  # FIXME: Not needed with Meson >= 0.64.0
  install_tag: 'devel')
