libcompositor_cargs = [
  '-DSHOYU_COMPILATION',
  '-DG_LOG_DOMAIN="ShoyuCompositor"',
  '-DSHOYU_COMPOSITOR_BINARY_VERSION="@0@"'.format(shoyu_binary_version),
  '-DSHOYU_COMPOSITOR_HOST="@0@"'.format(host_machine.system()),
  '-DSHOYU_COMPOSITOR_DATA_PREFIX="@0@"'.format(shoyu_prefix),
  '-DWLR_USE_UNSTABLE',
]

libcompositor_private_sources = files([
  'shell-output.c',
  'shell-toplevel.c',
])

libcompositor_private_h_sources = files([
  'compositor-private.h',
  'input-private.h',
  'output-private.h',
  'shell-private.h',
  'shell-output-private.h',
  'shell-toplevel-private.h',
  'surface-private.h',
  'wayland-event-source-private.h',
  'xdg-toplevel-private.h',
])

libcompositor_public_sources = files([
  'compositor.c',
  'input.c',
  'main.c',
  'output.c',
  'shell.c',
  'surface.c',
  'version.c',
  'wayland-event-source.c',
  'xdg-toplevel.c',
])

libcompositor_public_headers = files([
  'compositor.h',
  'input.h',
  'main.h',
  'output.h',
  'shell.h',
  'shoyu-compositor.h',
  'surface.h',
  'wayland-event-source.h',
  'xdg-toplevel.h',
])

install_headers(libcompositor_public_headers, subdir: 'shoyu-compositor/')
libcompositor_sources = libcompositor_public_sources + libcompositor_private_sources

libcompositor_enums = gnome.mkenums_simple('compositorenumtypes',
  sources: libcompositor_public_headers,
  decorator: 'SHOYU_AVAILABLE_IN_ALL',
  body_prefix: '#include "config.h"',
  header_prefix: '#include "version/versionmacros.h"\n',
  install_dir: shoyu_includedir / 'shoyu-compositor',
  install_header: true)

libcompositor_enums_h = libcompositor_enums[1]

libcompositor_config_cdata = configuration_data()

libcompositor_config = configure_file(
  input: 'config.h.meson',
  output: 'config.h',
  configuration: libcompositor_config_cdata,
  install_dir: shoyu_includedir / 'shoyu-compositor',
)

libcompositor_version_cdata = configuration_data()
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_MAJOR_VERSION', shoyu_major_version)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_MINOR_VERSION', shoyu_minor_version)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_MICRO_VERSION', shoyu_micro_version)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_BINARY_AGE', shoyu_binary_age)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_INTERFACE_AGE', shoyu_interface_age)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_VERSION', shoyu_version)
libcompositor_version_cdata.set('SHOYU_COMPOSITOR_API_VERSION', shoyu_api_version)

libcompositor_version = configure_file(input: 'version.h.in',
  output: 'version.h',
  configuration: libcompositor_version_cdata,
  install: true,
  install_dir: shoyu_includedir / 'shoyu-compositor')

libcompositor_gen_headers = [
  libcompositor_enums_h,
  libcompositor_config,
  libcompositor_version,
]

libcompositor_deps = [
  gobject,
  gio,
  wlroots,
  wayland_server,
  pixman,
]

darwin_versions = [
  # compatibility version
  1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(),
  # current version
  '@0@.@1@'.format(1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(), shoyu_interface_age),
]

libcompositor_sources += [
  libcompositor_config,
  libcompositor_enums,
  libcompositor_version_macros_h,
  libcompositor_visibility_h,
  libcompositor_private_h_sources,
  libcompositor_public_headers,
]

libcompositor = shared_library('shoyu-compositor',
  sources: libcompositor_sources,
  c_args: libcompositor_cargs + common_cflags,
  include_directories: [conf_inc, proto_inc, libcompositor_inc],
  link_whole: [libwayland_server],
  dependencies: libcompositor_deps,
  link_args: common_ldflags,
  soversion: shoyu_soversion,
  version: shoyu_library_version,
  darwin_versions: darwin_versions,
  gnu_symbol_visibility: 'hidden',
  install: true)

libcompositor_dep_sources = [libcompositor_config, libcompositor_version]

if build_gir
  gir_args = [
    '-DSHOYU_COMPILATION',
    '--quiet',
  ]

  libcompositor_gir_inc = [ 'GObject-2.0', 'Gio-2.0' ]

  libcompositor_gir = gnome.generate_gir(libcompositor,
    sources: [
      libcompositor_enums_h,
      libcompositor_public_headers,
      libcompositor_public_sources,
      libcompositor_version,
      libcompositor_config,
    ],
    namespace: 'Shoyu',
    nsversion: shoyu_api_version,
    identifier_prefix: 'Shoyu',
    symbol_prefix: 'shoyu',
    includes: libcompositor_gir_inc,
    header: 'shoyu-compositor/shoyu-compositor.h',
    install: true,
    dependencies: libcompositor_deps,
    extra_args: gir_args,
    fatal_warnings: get_option('werror'),
  )
  libcompositor_dep_sources += libcompositor_gir
endif

if build_vapi
  libcompositor_vapi = gnome.generate_vapi('shoyu-compositor',
    sources: libcompositor_gir[0],
    packages: ['gobject-2.0', 'gio-2.0'],
    install: true)
endif

libcompositor_dep = declare_dependency(
  sources: libcompositor_dep_sources,
  include_directories: [conf_inc, libcompositor_inc],
  dependencies: libcompositor_deps,
  link_with: libcompositor,
  link_args: common_ldflags)

pkg_config.generate(libcompositor,
  name: 'shoyu-compositor',
  description: 'Compositor API for Shoyu',
  requires: ['gobject-2.0', 'gio-2.0'])
