libshell_gtk4_cargs = [
  '-DSHOYU_SHELL_GTK_COMPILATION',
  '-DG_LOG_DOMAIN="ShoyuShellGtk"',
  '-DSHOYU_SHELL_GTK_BINARY_VERSION="@0@"'.format(shoyu_binary_version),
  '-DSHOYU_SHELL_GTK_HOST="@0@"'.format(host_machine.system()),
  '-DSHOYU_SHELL_GTK_DATA_PREFIX="@0@"'.format(shoyu_prefix),
]

libshell_gtk4_private_sources = files([])

libshell_gtk4_private_h_sources = files([])

libshell_gtk4_public_sources = files([
  'version.c',
])

libshell_gtk4_public_headers = files([
  'shoyu-shell-gtk4.h',
])

install_headers(libshell_gtk4_public_headers, subdir: 'shoyu-shell-gtk4/')
libshell_gtk4_sources = libshell_gtk4_public_sources + libshell_gtk4_private_sources

libshell_gtk4_enums = gnome.mkenums_simple('shell-gtk4-enumtypes',
  sources: libshell_gtk4_public_headers,
  decorator: 'SHOYU_SHELL_GTK_AVAILABLE_IN_ALL',
  body_prefix: '#include "config.h"',
  header_prefix: '#include "version/versionmacros.h"\n',
  install_dir: shoyu_includedir / 'shoyu-shell-gtk4',
  install_header: true)

libshell_gtk4_enums_h = libshell_gtk4_enums[1]

libshell_gtk4_config_cdata = configuration_data()

libshell_gtk4_config = configure_file(
  input: 'config.h.meson',
  output: 'config.h',
  configuration: libshell_gtk4_config_cdata,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk4',
)

libshell_gtk4_version_cdata = configuration_data()
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_MAJOR_VERSION', shoyu_major_version)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_MINOR_VERSION', shoyu_minor_version)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_MICRO_VERSION', shoyu_micro_version)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_BINARY_AGE', shoyu_binary_age)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_INTERFACE_AGE', shoyu_interface_age)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_VERSION', shoyu_version)
libshell_gtk4_version_cdata.set('SHOYU_SHELL_GTK_API_VERSION', shoyu_api_version)

libshell_gtk4_version = configure_file(input: 'version.h.in',
  output: 'version.h',
  configuration: libshell_gtk4_version_cdata,
  install: true,
  install_dir: shoyu_includedir / 'shoyu-shell-gtk4',
)

libshell_gtk4_gen_headers = [
  libshell_gtk4_enums_h,
  libshell_gtk4_config,
  libshell_gtk4_version,
]

libshell_gtk4_deps = [
  gobject,
]

darwin_versions = [
  # compatibility version
  1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(),
  # current version
  '@0@.@1@'.format(1 + '@0@'.format(shoyu_binary_age - shoyu_interface_age).to_int(), shoyu_interface_age),
]

libshell_gtk4_sources += [
  libshell_gtk4_config,
  libshell_gtk4_enums,
  libshell_gtk4_version_macros_h,
  libshell_gtk4_visibility_h,
  libshell_gtk4_private_h_sources,
  libshell_gtk4_public_headers,
]

libshell_gtk4 = shared_library('shoyu-shell-gtk4',
  sources: libshell_gtk4_sources,
  c_args: libshell_gtk4_cargs + common_cflags,
  include_directories: [conf_inc, libshell_gtk4_inc],
  dependencies: libshell_gtk4_deps,
  link_args: common_ldflags,
  soversion: shoyu_soversion,
  version: shoyu_library_version,
  darwin_versions: darwin_versions,
  gnu_symbol_visibility: 'hidden',
  install: true)

libshell_gtk4_dep_sources = [libshell_gtk4_config, libshell_gtk4_version]

if build_gir
  gir_args = [
    '-DSHOYU_SHELL_GTK_COMPILATION',
    '--quiet',
  ]

  libshell_gtk4_gir_inc = [ 'GObject-2.0' ]

  libshell_gtk4_gir = gnome.generate_gir(libshell_gtk4,
    sources: [
      libshell_gtk4_enums_h,
      libshell_gtk4_public_headers,
      libshell_gtk4_public_sources,
      libshell_gtk4_version,
      libshell_gtk4_config,
    ],
    namespace: 'ShoyuShellGtk4',
    nsversion: shoyu_api_version,
    identifier_prefix: 'ShoyuShellGtk',
    symbol_prefix: 'shoyu_shell_gtk',
    includes: libshell_gtk4_gir_inc,
    header: 'shoyu-shell-gtk4/shoyu-shell-gtk4.h',
    export_packages: 'shoyu-shell-gtk4',
    install: true,
    dependencies: libshell_gtk4_deps,
    extra_args: gir_args,
    fatal_warnings: get_option('werror'),
  )
  libshell_gtk4_dep_sources += libshell_gtk4_gir
endif

if build_vapi
  libshell_gtk4_vapi = gnome.generate_vapi('shoyu-shell-gtk4',
    sources: libshell_gtk4_gir[0],
    packages: ['gobject-2.0'],
    install: true)
endif

libshell_gtk4_dep = declare_dependency(
  sources: libshell_gtk4_dep_sources,
  include_directories: [conf_inc, libshell_gtk4_inc],
  dependencies: libshell_gtk4_deps,
  link_with: libshell_gtk4,
  link_args: common_ldflags)
