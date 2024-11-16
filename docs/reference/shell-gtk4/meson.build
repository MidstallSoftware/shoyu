expand_content_md_files = [
]

if get_option('documentation')
  shoyu_shell_gtk4_toml = configure_file(
    input: 'shoyu-shell-gtk4.toml.in',
    output: 'shoyu-shell-gtk4.toml',
    configuration: toml_conf,
    install: true,
    install_dir: docs_dir / 'shoyu-shell-gtk4')

  custom_target('shell-gtk4-doc',
    input: [ shoyu_shell_gtk4_toml, libshell_gtk4_gir[0] ],
    output: 'shoyu-shell-gtk4',
    command: [
      gidocgen,
      'generate',
      gidocgen_common_args,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../shoyu-shell-gtk4'),
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--content-dir=@0@'.format(meson.current_source_dir()),
      '@INPUT1@',
    ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: docs_dir)

  test('doc-check-shell-gtk4',
    gidocgen,
    args: [
      'check',
      '--config', shoyu_shell_gtk4_toml,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../shoyu-shell-gtk4'),
      libshell_gtk4_gir[0],
    ],
    depends: libshell_gtk4_gir[0],
    suite: ['docs'])
endif
