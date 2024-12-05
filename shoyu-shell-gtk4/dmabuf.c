#include "dmabuf-private.h"
#include <fcntl.h>
#include <libudev.h>
#include <sys/mman.h>
#include <xf86drm.h>

static char *get_most_appropriate_node(const char *drm_node,
                                       gboolean is_scanout_device) {
  int n_devices = drmGetDevices2(0, NULL, 0);
  g_return_val_if_fail(n_devices > 0, NULL);

  drmDevice **devices = g_new0(drmDevice *, n_devices);
  g_return_val_if_fail(devices != NULL, NULL);

  n_devices = drmGetDevices2(0, devices, n_devices);
  g_return_val_if_fail(n_devices > 0, NULL);

  drmDevice *match = NULL;
  for (int i = 0; i < n_devices && match == NULL; i++) {
    for (int j = 0; j < DRM_NODE_MAX; j++) {
      if (!(devices[i]->available_nodes & (1 << j)))
        continue;
      if (g_strcmp0(devices[i]->nodes[j], drm_node) == 0) {
        match = devices[i];
        break;
      }
    }
  }

  g_return_val_if_fail(match != NULL, NULL);

  char *appropriate_node = NULL;
  if (is_scanout_device) {
    appropriate_node = g_strdup(match->nodes[DRM_NODE_PRIMARY]);
  } else {
    if (match->available_nodes & (1 << DRM_NODE_RENDER))
      appropriate_node = g_strdup(match->nodes[DRM_NODE_RENDER]);
    else
      appropriate_node = g_strdup(match->nodes[DRM_NODE_PRIMARY]);
  }

  for (int i = 0; i < n_devices; i++)
    drmFreeDevice(&devices[i]);
  g_free(devices);

  return appropriate_node;
}

static char *get_drm_node(dev_t device, gboolean is_scanout_device) {
  struct udev *udev = udev_new();
  g_return_val_if_fail(udev != NULL, NULL);

  struct udev_device *udev_device =
      udev_device_new_from_devnum(udev, 'c', device);
  g_return_val_if_fail(udev_device != NULL, NULL);

  const char *node = udev_device_get_devnode(udev_device);
  g_return_val_if_fail(node != NULL, NULL);

  udev_unref(udev);
  return get_most_appropriate_node(node, is_scanout_device);
}

static void dmabuf_tranche_free(DmabufTranche *self) {
  g_free(self->formats);
  g_free(self);
}

static void dmabuf_formats_free(DmabufFormats *self) {
  g_ptr_array_unref(self->tranches);
  g_free(self);
}

static void zwp_linux_dmabuf_feedback_v1_done(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1) {
  DmabufFormatsInfo *self = data;

  g_clear_pointer(&self->formats, (GDestroyNotify)dmabuf_formats_free);
  self->formats = self->pending_formats;
  self->pending_formats = NULL;

  char *drm_node = get_drm_node(self->formats->main_device, FALSE);
  if (drm_node != NULL) {
    if (self->card_fd < 1)
      self->card_fd = open(drm_node, O_RDWR | O_CLOEXEC);
    g_free(drm_node);
  }

  if (self->card_fd > 0) {
    self->gbm_device = gbm_create_device(self->card_fd);
  }
}

static void zwp_linux_dmabuf_feedback_v1_format_table(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1,
    int32_t fd, uint32_t size) {
  DmabufFormatsInfo *self = data;

  if (self->formats_table != NULL) {
    munmap(self->formats_table, sizeof(DmabufFormat) * self->n_formats_table);
  }

  self->n_formats_table = size / 16;
  self->formats_table = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
}

static void zwp_linux_dmabuf_feedback_v1_main_device(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1,
    struct wl_array *device) {
  DmabufFormatsInfo *self = data;

  self->pending_formats = g_new0(DmabufFormats, 1);
  memcpy(&self->pending_formats->main_device, device->data, sizeof(dev_t));

  self->pending_formats->tranches =
      g_ptr_array_new_with_free_func((GDestroyNotify)dmabuf_tranche_free);
}

static void zwp_linux_dmabuf_feedback_v1_tranche_done(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1) {
  DmabufFormatsInfo *self = data;

  g_ptr_array_add(self->pending_formats->tranches, self->pending_tranche);
  self->pending_tranche = NULL;
}

static void zwp_linux_dmabuf_feedback_v1_tranche_target_device(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1,
    struct wl_array *device) {
  DmabufFormatsInfo *self = data;

  g_assert(self->pending_tranche == NULL);

  DmabufTranche *tranche = g_new0(DmabufTranche, 1);
  memcpy(&tranche->target_device, device->data, sizeof(dev_t));

  self->pending_tranche = tranche;
}

static void zwp_linux_dmabuf_feedback_v1_tranche_formats(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1,
    struct wl_array *indices) {
  DmabufFormatsInfo *self = data;

  g_assert(self->pending_tranche != NULL);
  DmabufTranche *tranche = self->pending_tranche;

  tranche->n_formats = indices->size / sizeof(guint16);
  tranche->formats = g_new(DmabufFormat, tranche->n_formats);

  guint i = 0;
  guint16 *pos;
  wl_array_for_each(pos, indices) {
    tranche->formats[i++] = self->formats_table[*pos];
  }
}

static void zwp_linux_dmabuf_feedback_v1_tranche_flags(
    void *data,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1,
    uint32_t flags) {
  DmabufFormatsInfo *self = data;

  g_assert(self->pending_tranche != NULL);
  DmabufTranche *tranche = self->pending_tranche;
  tranche->flags = flags;
}

static const struct zwp_linux_dmabuf_feedback_v1_listener
    zwp_linux_dmabuf_feedback_v1_listener = {
      .done = zwp_linux_dmabuf_feedback_v1_done,
      .format_table = zwp_linux_dmabuf_feedback_v1_format_table,
      .main_device = zwp_linux_dmabuf_feedback_v1_main_device,
      .tranche_done = zwp_linux_dmabuf_feedback_v1_tranche_done,
      .tranche_target_device =
          zwp_linux_dmabuf_feedback_v1_tranche_target_device,
      .tranche_formats = zwp_linux_dmabuf_feedback_v1_tranche_formats,
      .tranche_flags = zwp_linux_dmabuf_feedback_v1_tranche_flags,
};

DmabufFormatsInfo *dmabuf_formats_info_new(
    GdkDisplay *display,
    struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1) {
  g_return_val_if_fail(GDK_IS_DISPLAY(display), NULL);
  g_return_val_if_fail(zwp_linux_dmabuf_feedback_v1 != NULL, NULL);

  DmabufFormatsInfo *self = g_new0(DmabufFormatsInfo, 1);
  g_assert(self != NULL);

  self->display = GDK_DISPLAY(g_object_ref(display));
  self->zwp_linux_dmabuf_feedback_v1 = zwp_linux_dmabuf_feedback_v1;

  zwp_linux_dmabuf_feedback_v1_add_listener(
      self->zwp_linux_dmabuf_feedback_v1,
      &zwp_linux_dmabuf_feedback_v1_listener, self);
  return self;
}

void dmabuf_formats_info_free(DmabufFormatsInfo *self) {
  if (self->formats_table != NULL) {
    munmap(self->formats_table, sizeof(DmabufFormat) * self->n_formats_table);
    self->formats_table = NULL;
  }

  g_clear_pointer(&self->zwp_linux_dmabuf_feedback_v1,
                  zwp_linux_dmabuf_feedback_v1_destroy);
  g_clear_pointer(&self->formats, (GDestroyNotify)dmabuf_formats_free);
  g_clear_pointer(&self->pending_formats, (GDestroyNotify)dmabuf_formats_free);
  g_clear_pointer(&self->pending_tranche, (GDestroyNotify)dmabuf_tranche_free);
  g_clear_pointer(&self->gbm_device, (GDestroyNotify)gbm_device_destroy);

  g_free(self);
}
