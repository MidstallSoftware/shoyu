protocols = [
  wayland.find_protocol('xdg-shell'),
  wayland.find_protocol('linux-dmabuf', version: 1),
  join_paths(meson.current_source_dir(), 'shoyu-shell.xml'),
]

libwayland_client_sources = []
libwayland_server_sources = []

foreach protocol : protocols
  generated = wayland.scan_xml(protocol,
    client: true,
    server: true,
    public: true)

  libwayland_client_sources += [
    generated[0],
    generated[1]
  ]

  libwayland_server_sources += [
    generated[0],
    generated[2]
  ]
endforeach

libwayland_client = static_library(
  'shoyu-wayland-client',
  sources: libwayland_client_sources)

libwayland_server = static_library(
  'shoyu-wayland-server',
  sources: libwayland_server_sources)
