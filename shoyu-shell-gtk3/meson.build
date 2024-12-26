libshell_gtk3_cargs = [
  '-DSHOYU_SHELL_GTK_COMPILATION',
  '-DG_LOG_DOMAIN="ShoyuShellGtk"',
  '-DSHOYU_SHELL_GTK_BINARY_VERSION="@0@"'.format(shoyu_binary_version),
  '-DSHOYU_SHELL_GTK_HOST="@0@"'.format(host_machine.system()),
  '-DSHOYU_SHELL_GTK_DATA_PREFIX="@0@"'.format(shoyu_prefix),
]

libshell_gtk3_private_sources = files([
  'dmabuf.c',
])

libshell_gtk3_private_h_sources = files([
  'display-private.h',
  'dmabuf-private.h',
  'main-private.h',
  'toplevel-private.h',
])

libshell_gtk3_public_sources = files([
  'display.c',
  'main.c',
  'toplevel.c',
  'version.c',
])

libshell_gtk3_public_headers = files([
  'display.h',
  'main.h',
  'shoyu-shell-gtk3.h',
  'toplevel.h',
])

install_headers(libshell_gtk3_public_headers, subdir: 'shoyu-shell-gtk3/')
libshell_gtk3_sources = libshell_gtk3_public_sources + libshell_gtk3_private_sources

libshell_gtk3_enums = gnome.mkenums_simple('shell-gtk3-enumtypes',
  sources: libshell_gtk3_public_headers,
  decorator: 'SHOYU_SHELL_GTK_AVAILABLE_IN_ALL',
  body_prefix: '#include "config.h"',
  header_prefix: '#include "version/versionmacros.h"\n',
  install_dir: shoyu_includedir / 'shoyu-shell-gtk3',
  install_header: true)

libshell_gtk3_enums_h = libshell_gtk3_enums[1]

libshell_gtk3_config_cdata = configuration_data()

libshell_gtk3_config = configure_file(
  input: 'config.h.meson',
  output: 'config.h',
  configuration: libshell_gtk3_config_cdata,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk3',
)

libshell_gtk3_version_cdata = configuration_data()
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_MAJOR_VERSION', shoyu_major_version)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_MINOR_VERSION', shoyu_minor_version)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_MICRO_VERSION', shoyu_micro_version)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_BINARY_AGE', shoyu_binary_age)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_INTERFACE_AGE', shoyu_interface_age)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_VERSION', shoyu_version)
libshell_gtk3_version_cdata.set('SHOYU_SHELL_GTK_API_VERSION', shoyu_api_version)

libshell_gtk3_version = configure_file(input: 'version.h.in',
  output: 'version.h',
  configuration: libshell_gtk3_version_cdata,
  install: true,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk3',
)

libshell_gtk3_gen_headers = [
  libshell_gtk3_enums_h,
  libshell_gtk3_config,
  libshell_gtk3_version,
]

libshell_gtk3_deps_public = [
  gobject,
  gtk3,
  cairo,
]

libshell_gtk3_deps_private = [
  drm,
  gbm,
  libudev,
  gdk3_wayland,
  wayland_client,
  epoxy,
]

libshell_gtk3_deps = libshell_gtk3_deps_public + libshell_gtk3_deps_private

darwin_versions = [
  # compatibility version
  1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(),
  # current version
  '@0@.@1@'.format(1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(), shoyu_interface_age),
]

libshell_gtk3_sources += [
  libshell_gtk3_config,
  libshell_gtk3_enums,
  libshell_gtk3_version_macros_h,
  libshell_gtk3_visibility_h,
  libshell_gtk3_private_h_sources,
  libshell_gtk3_public_headers,
]

libshell_gtk3 = shared_library('shoyu-shell-gtk3',
  sources: libshell_gtk3_sources,
  c_args: libshell_gtk3_cargs + common_cflags,
  include_directories: [conf_inc, libshell_gtk3_inc, proto_inc],
  dependencies: libshell_gtk3_deps,
  link_whole: [libwayland_client],
  link_args: common_ldflags,
  soversion: shoyu_soversion,
  version: shoyu_library_version,
  darwin_versions: darwin_versions,
  gnu_symbol_visibility: 'hidden',
  install: true)

libshell_gtk3_dep_sources = [libshell_gtk3_config, libshell_gtk3_version]

if build_gir
  gir_args = [
    '-DSHOYU_SHELL_GTK_COMPILATION',
    '--quiet',
  ]

  libshell_gtk3_gir_inc = [ 'GObject-2.0', 'Gtk-3.0' ]

  libshell_gtk3_gir = gnome.generate_gir(libshell_gtk3,
    sources: [
      libshell_gtk3_enums_h,
      libshell_gtk3_public_headers,
      libshell_gtk3_public_sources,
      libshell_gtk3_version,
      libshell_gtk3_config,
    ],
    namespace: 'ShoyuShellGtk3',
    nsversion: shoyu_api_version,
    identifier_prefix: 'ShoyuShellGtk',
    symbol_prefix: 'shoyu_shell_gtk',
    includes: libshell_gtk3_gir_inc,
    header: 'shoyu-shell-gtk3/shoyu-shell-gtk3.h',
    export_packages: 'shoyu-shell-gtk3',
    install: true,
    dependencies: libshell_gtk3_deps,
    extra_args: gir_args,
    fatal_warnings: get_option('werror'),
  )
  libshell_gtk3_dep_sources += libshell_gtk3_gir
endif

if build_vapi
  libshell_gtk3_vapi = gnome.generate_vapi('shoyu-shell-gtk3',
    sources: libshell_gtk3_gir[0],
    packages: ['gobject-2.0', 'gtk+-3.0', 'cairo'],
    install: true)
endif

libshell_gtk3_dep = declare_dependency(
  sources: libshell_gtk3_dep_sources,
  include_directories: [conf_inc, libshell_gtk3_inc],
  dependencies: libshell_gtk3_deps_public,
  link_with: libshell_gtk3,
  link_args: common_ldflags)

pkg_config.generate(libshell_gtk3,
  name: 'shoyu-shell-gtk3',
  description: 'Shoyu Shell client API for GTK 3',
  requires: ['gobject-2.0', 'gio-2.0', 'gtk+-3.0'])
