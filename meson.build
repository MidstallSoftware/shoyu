project('shoyu', 'c',
  license: 'AGPL-3.0-only',
  license_files: 'LICENSE',
  version: '0.1.0',
  default_options: [
    'buildtype=debugoptimized',
    'warning_level=1',
    'c_std=gnu99',
  ],
  meson_version: '>= 1.2.0')

gnome = import('gnome')

add_project_arguments('-DG_LOG_USE_STRUCTURED=1', language: 'c')
add_project_arguments('-DGLIB_DISABLE_DEPRECATION_WARNINGS', language: 'c')

shoyu_version = meson.project_version()
shoyu_major_version = shoyu_version.split('.')[0].to_int()
shoyu_minor_version = shoyu_version.split('.')[1].to_int()
shoyu_micro_version = shoyu_version.split('.')[2].to_int()

shoyu_interface_age = shoyu_minor_version.is_odd() ? 0 : shoyu_micro_version

add_project_arguments('-DSHOYU_VERSION="@0@"'.format(shoyu_version), language: 'c')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

shoyu_debug_cflags = []
debug = get_option('debug')
optimization = get_option('optimization')
if debug
  shoyu_debug_cflags += '-DG_ENABLE_DEBUG'
  if optimization in ['0', 'g']
    shoyu_debug_cflags += '-DG_ENABLE_CONSISTENCY_CHECKS'
  endif
elif optimization in ['2', '3', 's']
  shoyu_debug_cflags += ['-DG_DISABLE_CAST_CHECKS', '-DG_DISABLE_ASSERT']
endif

add_project_arguments(shoyu_debug_cflags, language: 'c')

shoyu_binary_version = '0.1.0'

shoyu_binary_age = 100 * shoyu_minor_version + shoyu_micro_version

shoyu_soversion = '0'
shoyu_library_version = '1.@0@.@1@'.format(shoyu_binary_age - shoyu_interface_age, shoyu_interface_age)

shoyu_api_version = '0.1'

os_unix = false
os_linux = false
os_win32 = false
os_darwin = false

cc = meson.get_compiler('c')

if host_machine.system() == 'darwin'
  if not cc.compiles('''
    #include <Availability.h>
    #if __MAC_OS_X_VERSION_MIN_REQUIRED < 101500L
    # error message "Minimal macOS SDK version not met"
    #endif
    ''',
    name: 'macOS SDK version >= 10.15',
  )
    error('macOS SDK 10.15 or newer is required to build GTK')
  endif

  os_darwin = true
elif host_machine.system() == 'windows'
  os_win32 = true
elif host_machine.system() == 'linux'
  os_linux = true
endif
os_unix = not os_win32

shoyu_prefix = get_option('prefix')
shoyu_includedir = join_paths(shoyu_prefix, get_option('includedir'))
shoyu_libdir = join_paths(shoyu_prefix, get_option('libdir'))
shoyu_datadir = join_paths(shoyu_prefix, get_option('datadir'))
shoyu_localedir = join_paths(shoyu_prefix, get_option('localedir'))
shoyu_sysconfdir = join_paths(shoyu_prefix, get_option('sysconfdir'))

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('SHOYU_LOCALEDIR', shoyu_localedir)
cdata.set_quoted('SHOYU_DATADIR', shoyu_datadir)
cdata.set_quoted('SHOYU_LIBDIR', shoyu_libdir)
cdata.set_quoted('SHOYU_SYSCONFDIR', shoyu_sysconfdir)
cdata.set_quoted('GETTEXT_PACKAGE', 'shoyu')

if shoyu_minor_version.is_even()
  cdata.set('GLIB_DISABLE_DEPRECATION_WARNINGS', 1)
endif

if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  test_cflags = [
    '-fno-strict-aliasing',
    '-Wno-c++11-extensions',
    '-Wno-missing-include-dirs',
    '-Wno-typedef-redefinition',
    '-Wno-tautological-constant-out-of-range-compare',
    '-Wduplicated-branches',
    '-Wduplicated-cond',
    '-Wformat=2',
    '-Wformat-nonliteral',
    '-Wformat-security',
    '-Wignored-qualifiers',
    '-Wimplicit-function-declaration',
    '-Wlogical-op',
    '-Wmisleading-indentation',
    '-Wmissing-format-attribute',
    '-Wmissing-include-dirs',
    '-Wmissing-noreturn',
    '-Wnested-externs',
    '-Wold-style-definition',
    '-Wpointer-arith',
    '-Wshadow',
    '-Wstrict-prototypes',
    '-Wswitch-default',
    '-Wswitch-enum',
    '-Wundef',
    '-Wuninitialized',
    '-Wunused',
  ]

  extra_warnings = [
    'address',
    'array-bounds',
    'empty-body',
    'enum-int-mismatch',
    'implicit',
    'implicit-fallthrough',   # For non-gcc
    'implicit-fallthrough=5', # For GCC, only recognize the attribute and no comments
    'init-self',
    'int-to-pointer-cast',
    'main',
    'missing-braces',
    'missing-declarations',
    'missing-prototypes',
    'nonnull',
    'override-init',
    'pointer-to-int-cast',
    'redundant-decls',
    'return-type',
    'sequence-point',
    'trigraphs',
    'vla',
    'write-strings',
  ]


  if get_option('buildtype').startswith('debug')
    foreach warning: extra_warnings
      test_cflags += '-Werror=@0@'.format(warning)
    endforeach
  else
    foreach warning: extra_warnings
      test_cflags += '-W@0@'.format(warning)
    endforeach
  endif

  if cc.get_id() == 'gcc'
    test_cflags += ['-Wcast-align'] # This warns too much on clang
  endif

  if not shoyu_debug_cflags.contains('-DG_DISABLE_ASSERT')
    test_cflags += ['-Wnull-dereference'] # Too noisy when assertions are disabled
  endif
else
  test_cflags = []
endif

if get_option('default_library') != 'static'
  if os_win32
    cdata.set('DLL_EXPORT', true)
  else
    test_cflags += ['-fvisibility=hidden']
  endif
endif

common_cflags = cc.get_supported_arguments(test_cflags)

common_ldflags = cc.get_supported_link_arguments([
  '-Wl,-Bsymbolic',
  '-Wl,-z,relro',
  '-Wl,-z,now',
])

conf_inc = include_directories('.')
libcompositor_inc = include_directories('shoyu-compositor')
libshell_gtk3_inc = include_directories('shoyu-shell-gtk3')
libshell_gtk4_inc = include_directories('shoyu-shell-gtk4')

gen_visibility_macros = find_program('build-aux/meson/gen-visibility-macros.py')

# GObject Introspection
gir = find_program('g-ir-scanner', required : get_option('introspection'))

if not gir.found() and get_option('introspection').enabled()
  error('Introspection enabled, but g-ir-scanner not found.')
endif

build_gir = gir.found() and (get_option('introspection').enabled() or
  (get_option('introspection').allowed() and get_option('documentation')))

# Vala API Generation
vapigen = find_program('vapigen', required: get_option('vapigen'))

if not vapigen.found() and get_option('vapigen').enabled()
  error('Vala API generation enabled, but vapigen not found.')
endif

build_vapi = vapigen.found() and (get_option('introspection').enabled() or
  (get_option('introspection').allowed() and build_gir))

# Base dependencies
gobject = dependency('gobject-2.0', version: '>= 2.81.0')
gio = dependency('gio-2.0', version: '>= 2.81.0')
wlroots = dependency('wlroots-0.18', version: '>= 0.18.0')
wayland_server = dependency('wayland-server', version: '>= 1.23.0')

shoyu_shells = []

# GTK 3
gtk3 = dependency('gtk+-3.0', required: get_option('gtk3'))

if not gtk3.found() and get_option('gtk3').enabled()
  error('GTK 3 enabled, but the library was not found.')
endif

build_gtk3 = gtk3.found() and (get_option('gtk3').enabled() or get_option('gtk3').allowed())

# GTK 4
if build_gtk3
  shoyu_shells += 'gtk3'
endif

gtk4 = dependency('gtk4', required: get_option('gtk4'))

if not gtk4.found() and get_option('gtk4').enabled()
  error('GTK 3 enabled, but the library was not found.')
endif

build_gtk4 = gtk4.found() and (get_option('gtk4').enabled() or get_option('gtk4').allowed())

if build_gtk4
  shoyu_shells += 'gtk4'
endif

configure_file(output: 'shoyu-config.h',
  configuration: cdata)

# Add subprojects
subdir('shoyu-compositor/version')
subdir('shoyu-compositor')
subdir('shoyu-compositor-runner')

foreach shell : shoyu_shells
  subdir('shoyu-shell-@0@/version'.format(shell))
  subdir('shoyu-shell-@0@'.format(shell))
endforeach

subdir('po')

# Documentation
subdir('docs/reference')

# Summary
summary('Shells', shoyu_shells, section: 'Components')

summary('Compiler', cc.get_id(), section: 'Toolchain')
summary('Linker', cc.get_linker_id(), section: 'Toolchain')

summary('Debugging', get_option('debug'), section: 'Build')
summary('Optimization', get_option('optimization'), section: 'Build')
summary('Introspection', build_gir, section: 'Build')
summary('Vala API Generation', build_vapi, section: 'Build')
summary('Documentation', get_option('documentation'), section: 'Build')

summary('prefix', shoyu_prefix, section: 'Directories')
summary('includedir', shoyu_includedir, section: 'Directories')
summary('libdir', shoyu_libdir, section: 'Directories')
summary('datadir', shoyu_datadir, section: 'Directories')
