expand_content_md_files = [
]

if get_option('documentation')
  shoyu_shell_gtk3_toml = configure_file(
    input: 'shoyu-shell-gtk3.toml.in',
    output: 'shoyu-shell-gtk3.toml',
    configuration: toml_conf,
    install: true,
    install_dir: docs_dir / 'shoyu-shell-gtk3')

  custom_target('shell-gtk3-doc',
    input: [ shoyu_shell_gtk3_toml, libshell_gtk3_gir[0] ],
    output: 'shoyu-shell-gtk3',
    command: [
      gidocgen,
      'generate',
      gidocgen_common_args,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../shoyu-shell-gtk3'),
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--content-dir=@0@'.format(meson.current_source_dir()),
      '@INPUT1@',
    ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: docs_dir)

  test('doc-check-shell-gtk3',
    gidocgen,
    args: [
      'check',
      '--config', shoyu_shell_gtk3_toml,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../shoyu-shell-gtk3'),
      libshell_gtk3_gir[0],
    ],
    depends: libshell_gtk3_gir[0],
    suite: ['docs'])
endif
