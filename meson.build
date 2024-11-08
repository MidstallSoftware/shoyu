project('shoyu', 'c')

wlroots = dependency('wlroots-0.18')
gobject = dependency('gobject-2.0')
libdrm = dependency('libdrm')
wayland_server = dependency('wayland-server')

gtk3 = dependency('gtk+-3.0', required: false)
gtk4 = dependency('gtk4', required: false)

libshoyu_deps = [wlroots, gobject, libdrm, wayland_server]
libshoyu_sources = [
  'src/application.c', 'src/compositor.c',
  'src/output.c', 'src/wayland-event-source.c'
]

if gtk3.found()
  libshoyu_gtk3 = shared_library('shoyu-gtk3',
    libshoyu_sources,
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    include_directories: ['include'],
    dependencies: [libshoyu_deps, gtk3],
    install: true)
  libshoyu_gtk3_dep = declare_dependency(
    link_with: [libshoyu_gtk3],
    include_directories: ['include'],
    dependencies: [libshoyu_deps, gtk3])

  exe_simple_example_gtk3 = executable('shoyu-simple-example-gtk3',
    'example/simple.c',
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    dependencies: [libshoyu_gtk3_dep])
endif

if gtk4.found()
  libshoyu_gtk4 = shared_library('shoyu-gtk4',
    libshoyu_sources,
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    include_directories: ['include'],
    dependencies: [libshoyu_deps, gtk4],
    install: true)
  libshoyu_gtk4_dep = declare_dependency(
    link_with: [libshoyu_gtk4],
    include_directories: ['include'],
    dependencies: [libshoyu_deps, gtk4])

  exe_simple_example_gtk4 = executable('shoyu-simple-example-gtk4',
    'example/simple.c',
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    dependencies: [libshoyu_gtk4_dep])
endif
