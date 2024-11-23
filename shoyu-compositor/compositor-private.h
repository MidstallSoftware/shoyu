#pragma once

#include "compositor.h"
#include "config.h"
#include "input.h"
#include "output.h"
#include "shell.h"
#include "surface.h"
#include "xdg-toplevel.h"

#ifdef SHOYU_COLORD
#include <colord.h>
#endif

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>

struct _ShoyuCompositor {
    GObject parent_instance;

    GApplication *application;
    ShoyuShell *shell;

    struct wl_display *wl_display;
    GSource *wl_source;

    struct wlr_backend *wlr_backend;
    struct wlr_renderer *wlr_renderer;
    struct wlr_allocator *wlr_allocator;

    struct wlr_xdg_shell *wlr_xdg_shell;
    struct wlr_output_layout *output_layout;

    struct wlr_compositor *wlr_compositor;

    GList *surfaces;
    struct wl_listener new_surface;

    GList *outputs;
    struct wl_listener new_output;

    GList *inputs;
    struct wl_listener new_input;

    GList *xdg_toplevels;
    struct wl_listener new_xdg_toplevel;

    const char *socket;

#ifdef SHOYU_COLORD
    CdClient *colord;
#endif
};

struct _ShoyuCompositorClass {
    GObjectClass parent_class;

    /**
     * ShoyuCompositor:output_type:
     *
     * The type to create when a #ShoyuOutput is being created.
     */
    GType output_type;

    /**
     * ShoyuCompositor:input_type:
     *
     * The type to create when a #ShoyuInput is being created.
     */
    GType input_type;

    /**
     * ShoyuCompositor:surface_type:
     *
     * The type to create when a #ShoyuSurface is being created.
     */
    GType surface_type;

    /**
     * ShoyuCompositor:xdg_toplevel_type:
     *
     * The type to create when a #ShoyuXdgToplevel is being created.
     */
    GType xdg_toplevel_type;

    /**
     * ShoyuCompositor:create_backend:
     * @self: (not nullable): The object instance
     * @event_loop: (not nullable): The Wayland event loop for the display
     * server.
     *
     * Creates a wlroots backend for the compositor.
     *
     * Returns: (not nullable) (transfer full): The wlroots backend to use.
     */
    struct wlr_backend *(*create_backend)(ShoyuCompositor *self,
                                          struct wl_event_loop *event_loop);

    /**
     * ShoyuCompositor:create_renderer:
     * @self: (not nullable): The object instance
     * @backend: (not nullable): The wlroots backend being used
     *
     * Creates a wlroots renderer for the compositor.
     *
     * Returns: (not nullable) (transfer full): The wlroots renderer to use.
     */
    struct wlr_renderer *(*create_renderer)(ShoyuCompositor *self,
                                            struct wlr_backend *backend);

    /**
     * ShoyuCompositor:create_allocator:
     * @self: (not nullable): The object instance
     * @backend: (not nullable): The wlroots backend being used
     * @renderer: (not nullable): The wlroots renderer being used
     *
     * Creates a wlroots allocator for the compositor.
     *
     * Returns: (not nullable) (transfer full): The wlroots allocator to use.
     */
    struct wlr_allocator *(*create_allocator)(ShoyuCompositor *self,
                                              struct wlr_backend *backend,
                                              struct wlr_renderer *renderer);

    /**
     * ShoyuOutput:create_output:
     * @self: (not nullable): The object instance
     * @output: (not nullable): The wlroots output
     *
     * Creates a #ShoyuOutput for the compositor.
     *
     * Returns: (nullable) (transfer full): A #ShoyuOutput.
     */
    ShoyuOutput *(*create_output)(ShoyuCompositor *self,
                                  struct wlr_output *output);

    /**
     * ShoyuOutput:create_input:
     * @self: (not nullable): The object instance
     * @output: (not nullable): The wlroots input
     *
     * Creates a #ShoyuInput for the compositor.
     *
     * Returns: (nullable) (transfer full): A #ShoyuInput.
     */
    ShoyuInput *(*create_input)(ShoyuCompositor *self,
                                struct wlr_input_device *device);

    /**
     * ShoyuOutput:create_surface:
     * @self: (not nullable): The object instance
     * @surface: (not nullable): The wlroots surface
     *
     * Creates a #ShoyuSurface for the compositor.
     *
     * Returns: (nullable) (transfer full): A #ShoyuSurface.
     */
    ShoyuSurface *(*create_surface)(ShoyuCompositor *self,
                                    struct wlr_surface *surface);

    /**
     * ShoyuOutput:create_xdg_toplevel:
     * @self: (not nullable): The object instance
     * @toplevel: (not nullable): The wlroots XDG toplevel
     *
     * Creates a #ShoyuXdgToplevel for the compositor.
     *
     * Returns: (nullable) (transfer full): A #ShoyuXdgToplevel.
     */
    ShoyuXdgToplevel *(*create_xdg_toplevel)(ShoyuCompositor *self,
                                             struct wlr_xdg_toplevel *toplevel);

    /**
     * ShoyuCompositor:output_added:
     * @self: (not nullable): The object instance
     * @output: (not nullable): The output which was added
     */
    void (*output_added)(ShoyuCompositor *self, ShoyuOutput *output);

    /**
     * ShoyuCompositor:output_removed:
     * @self: (not nullable): The object instance
     * @output: (not nullable): The output which was removed
     */
    void (*output_removed)(ShoyuCompositor *self, ShoyuOutput *output);

    /**
     * ShoyuCompositor:input_added:
     * @self: (not nullable): The object instance
     * @input: (not nullable): The input which was added
     */
    void (*input_added)(ShoyuCompositor *self, ShoyuInput *input);

    /**
     * ShoyuCompositor:input_removed:
     * @self: (not nullable): The object instance
     * @input: (not nullable): The input which was removed
     */
    void (*input_removed)(ShoyuCompositor *self, ShoyuInput *input);

    /**
     * ShoyuCompositor:surface_added:
     * @self: (not nullable): The object instance
     * @surface: (not nullable): The surface which was added
     */
    void (*surface_added)(ShoyuCompositor *self, ShoyuSurface *surface);

    /**
     * ShoyuCompositor:surface_removed:
     * @self: (not nullable): The object instance
     * @surface: (not nullable): The surface which was removed
     */
    void (*surface_removed)(ShoyuCompositor *self, ShoyuSurface *surface);

    /**
     * ShoyuCompositor:input_added:
     * @self: (not nullable): The object instance
     * @toplevel: (not nullable): The toplevel which was added
     */
    void (*xdg_toplevel_added)(ShoyuCompositor *self,
                               ShoyuXdgToplevel *toplevel);

    /**
     * ShoyuCompositor:output_removed:
     * @self: (not nullable): The object instance
     * @toplevel: (not nullable): The toplevel which was removed
     */
    void (*xdg_toplevel_removed)(ShoyuCompositor *self,
                                 ShoyuXdgToplevel *toplevel);

    /**
     * ShoyuCompositor::started:
     * @self: (not nullable): The object instance
     */
    void (*started)(ShoyuCompositor *self);
};

ShoyuOutput *shoyu_compositor_get_output(ShoyuCompositor *self,
                                         struct wlr_output *wlr_output);
ShoyuSurface *shoyu_compositor_get_surface(ShoyuCompositor *self,
                                           struct wlr_surface *wlr_surface);

gboolean
shoyu_compositor_is_xdg_toplevel_claimed(ShoyuCompositor *self,
                                         struct xdg_toplevel *xdg_toplevel);
ShoyuOutput *shoyu_compositor_get_xdg_toplevel_claimed_output(
    ShoyuCompositor *self, struct xdg_toplevel *xdg_toplevel);
