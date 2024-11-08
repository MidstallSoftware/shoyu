project('shoyu', 'c',
  license: 'AGPL-3.0-only',
  license_files: 'LICENSE',
  version: '0.1.0')

gnome = import('gnome')

libdrm = dependency('libdrm')
wayland_server = dependency('wayland-server')
wlroots = dependency('wlroots-0.18')

gobject = dependency('gobject-2.0')
gtk3 = dependency('gtk+-3.0', required: false)
gtk4 = dependency('gtk4', required: false)

gobject_introspection = dependency('gobject-introspection-1.0', required: false)
vapigen = find_program('vapigen', required: false)
valac = find_program('valac', required: false)

if valac.found()
  add_languages('vala')
  add_project_arguments(['--vapidir', join_paths(meson.current_source_dir(), 'vapi')], language: 'vala')
endif

libshoyu_deps = [wlroots, gobject, libdrm, wayland_server]
libshoyu_sources = [
  'src/application.c', 'src/compositor.c',
  'src/output.c', 'src/wayland-event-source.c'
]
libshoyu_headers = [
  'include/shoyu/application.h', 'include/shoyu/compositor.h',
  'include/shoyu/output.h', 'include/shoyu/shoyu.h',
  'include/shoyu/wayland-event-source.h'
]

if gtk3.found()
  libshoyu_gtk3_deps = libshoyu_deps + [gtk3]
  libshoyu_gtk3 = shared_library('shoyu-gtk3',
    libshoyu_sources,
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    include_directories: ['include'],
    dependencies: libshoyu_gtk3_deps,
    install: true)

  if gobject_introspection.found()
    libshoyu_gtk3_gir = gnome.generate_gir(
      libshoyu_gtk3,
      dependencies: [libshoyu_deps, gtk3],
      sources: libshoyu_sources + libshoyu_headers,
      extra_args: ['-DWLR_USE_UNSTABLE=1'],
      namespace: 'ShoyuGtk3',
      nsversion: '0.1',
      identifier_prefix: 'Shoyu',
      symbol_prefix: 'shoyu',
      export_packages: 'shoyu-gtk3',
      includes: ['Gtk-3.0'],
      header: 'shoyu/shoyu.h',
      install: true)
  endif

  if vapigen.found()
    libshoyu_gtk3_vapi = gnome.generate_vapi(
      'shoyu-gtk3',
      sources: libshoyu_gtk3_gir[0],
      packages: 'gtk+-3.0',
      install: true)
  endif

  libshoyu_gtk3_dep = declare_dependency(
    link_with: [libshoyu_gtk3],
    include_directories: ['include'],
    dependencies: libshoyu_gtk3_deps)

  exe_simple_example_gtk3 = executable('shoyu-simple-example-gtk3',
    'example/simple.c',
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    dependencies: [libshoyu_gtk3_dep])

  if valac.found()
    exe_vala_example_gtk3 = executable('shoyu-vala-example-gtk3',
      'example/gtk3.vala',
      c_args: ['-DWLR_USE_UNSTABLE=1'],
      dependencies: [libshoyu_gtk3_dep, libshoyu_gtk3_vapi])
  endif
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

  if gobject_introspection.found()
    libshoyu_gtk4_gir = gnome.generate_gir(
      libshoyu_gtk4,
      dependencies: [libshoyu_deps, gtk4],
      sources: libshoyu_sources + libshoyu_headers,
      extra_args: ['-DWLR_USE_UNSTABLE=1'],
      namespace: 'ShoyuGtk4',
      nsversion: '0.1',
      identifier_prefix: 'Shoyu',
      symbol_prefix: 'shoyu',
      export_packages: 'shoyu-gtk4',
      includes: ['Gtk-4.0'],
      header: 'shoyu/shoyu.h',
      install: true)
  endif

  if vapigen.found()
    libshoyu_gtk4_vapi = gnome.generate_vapi(
      'shoyu-gtk4',
      sources: libshoyu_gtk4_gir[0],
      packages: 'gtk4',
      install: true)
  endif

  exe_simple_example_gtk4 = executable('shoyu-simple-example-gtk4',
    'example/simple.c',
    c_args: ['-DWLR_USE_UNSTABLE=1'],
    dependencies: [libshoyu_gtk4_dep])
endif
