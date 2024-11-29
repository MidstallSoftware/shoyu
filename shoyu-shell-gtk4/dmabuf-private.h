#pragma once

#include <gbm.h>
#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include <linux-dmabuf-v1-client-protocol.h>
#include <wayland-client.h>

typedef struct {
    uint32_t fourcc;
    uint32_t padding;
    uint64_t modifier;
} DmabufFormat;

typedef struct {
    dev_t target_device;
    guint32 flags;
    guint n_formats;
    DmabufFormat *formats;
} DmabufTranche;

typedef struct {
    dev_t main_device;
    GPtrArray *tranches;
} DmabufFormats;

typedef struct {
    GdkDisplay *display;
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1;
    guint n_formats_table;
    DmabufFormat *formats_table;
    DmabufFormats *formats;
    DmabufFormats *pending_formats;
    DmabufTranche *pending_tranche;
    int card_fd;
    struct gbm_device *gbm_device;
} DmabufFormatsInfo;

DmabufFormatsInfo *dmabuf_formats_info_new(
    GdkDisplay *display,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1);
void dmabuf_formats_info_free(DmabufFormatsInfo *self);
