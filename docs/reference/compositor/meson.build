expand_content_md_files = [
]

if get_option('documentation')
  shoyu_compositor_toml = configure_file(
    input: 'shoyu-compositor.toml.in',
    output: 'shoyu-compositor.toml',
    configuration: toml_conf,
    install: true,
    install_dir: docs_dir / 'shoyu-compositor')

  custom_target('compositor-doc',
    input: [ shoyu_compositor_toml, libcompositor_gir[0] ],
    output: 'shoyu-compositor',
    command: [
      gidocgen,
      'generate',
      gidocgen_common_args,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../compositor'),
      '--config=@INPUT0@',
      '--output-dir=@OUTPUT@',
      '--content-dir=@0@'.format(meson.current_source_dir()),
      '@INPUT1@',
    ],
    depend_files: [ expand_content_md_files ],
    build_by_default: true,
    install: true,
    install_dir: docs_dir)

  test('doc-check-compositor',
    gidocgen,
    args: [
      'check',
      '--config', shoyu_compositor_toml,
      '--add-include-path=@0@'.format(meson.current_build_dir() / '../../../compositor'),
      libcompositor_gir[0],
    ],
    depends: libcompositor_gir[0],
    suite: ['docs'])
endif
