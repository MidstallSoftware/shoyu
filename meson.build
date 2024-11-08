project('shoyu', 'c')

wlroots = dependency('wlroots-0.18')
gobject = dependency('gobject-2.0')
gtk3 = dependency('gtk+-3.0')
libdrm = dependency('libdrm')
wayland_server = dependency('wayland-server')

libshoyu_deps = [wlroots, gobject, gtk3, libdrm, wayland_server]
libshoyu = shared_library('shoyu',
  ['src/application.c', 'src/compositor.c',
    'src/output.c', 'src/wayland-event-source.c'],
  c_args: ['-DWLR_USE_UNSTABLE=1'],
  include_directories: ['include'],
  dependencies: libshoyu_deps,
  install: true)
libshoyu_dep = declare_dependency(
  link_with: [libshoyu],
  include_directories: ['include'],
  dependencies: libshoyu_deps)

exe_simple_example = executable('shoyu-simple-example',
  'example/simple.c',
  c_args: ['-DWLR_USE_UNSTABLE=1'],
  dependencies: [libshoyu_dep])
