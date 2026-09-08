/*
 * usbliter8_boot.c
 *
 * Host-side control transport for a device that has already been put into
 * usbliter8 pwned DFU by the required RP2350 hardware.  A Mac/PC USB host
 * cannot perform the SecureROM exploit itself; it can only send the raw
 * PongoOS/iBoot image after the device advertises the PWND marker.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <libusb.h>

#include "usbliter8_boot.h"

#define DFU_VID          0x05AC
#define DFU_PID          0x1227

#define DFU_DNLOAD       1
/* usbliter8's USB request handler uses CLR_STATUS (4) after boot. */
#define DFU_ABORT        4
#define CUSTOM_BOOT      8

#define TRANSFER_SIZE    0x800u
#define CTRL_TIMEOUT_MS  1000

#define REQTYPE_CLASS_OUT \
    (LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT)

static libusb_device_handle *open_usbliter8_device(libusb_context *ctx)
{
    libusb_device_handle *handle = NULL;
    libusb_device **list = NULL;
    ssize_t count;
    ssize_t i;

    count = libusb_get_device_list(ctx, &list);
    if (count < 0) {
        fprintf(stderr, "error: could not enumerate USB devices: %s\n",
                libusb_error_name((int)count));
        return NULL;
    }

    for (i = 0; i < count; ++i) {
        struct libusb_device_descriptor desc;
        unsigned char serial[256] = { 0 };
        int ret;

        ret = libusb_get_device_descriptor(list[i], &desc);
        if (ret != LIBUSB_SUCCESS || desc.idVendor != DFU_VID ||
            desc.idProduct != DFU_PID)
            continue;

        ret = libusb_open(list[i], &handle);
        if (ret != LIBUSB_SUCCESS)
            continue;

        (void)libusb_set_auto_detach_kernel_driver(handle, 1);
        if (!desc.iSerialNumber ||
            libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
                                               serial, sizeof(serial) - 1) < 0 ||
            !strstr((const char *)serial, "PWND:[usbliter8]")) {
            /* Never send a Pongo image to an ordinary or checkm8-pwned DFU
             * device. Continue searching if another device is connected. */
            libusb_close(handle);
            handle = NULL;
            continue;
        }

        printf("usbliter8: %s\n", serial);
        libusb_free_device_list(list, 1);
        return handle;
    }

    libusb_free_device_list(list, 1);
    fprintf(stderr, "error: no Apple DFU device marked PWND:[usbliter8] found\n");
    return NULL;
}

static int download_image(libusb_device_handle *handle,
                          const unsigned char *buf, size_t len)
{
    size_t offset = 0;

    while (offset < len) {
        size_t remaining = len - offset;
        uint16_t chunk = (uint16_t)(remaining < TRANSFER_SIZE
                                      ? remaining : TRANSFER_SIZE);
        int ret = libusb_control_transfer(handle, REQTYPE_CLASS_OUT,
                                           DFU_DNLOAD, 0, 0,
                                           (unsigned char *)(buf + offset),
                                           chunk, CTRL_TIMEOUT_MS);
        if (ret < 0) {
            fprintf(stderr,
                    "\nerror: DFU download failed at offset 0x%zx: %s\n",
                    offset, libusb_error_name(ret));
            return -1;
        }
        if ((uint16_t)ret != chunk) {
            fprintf(stderr,
                    "\nerror: short DFU download at offset 0x%zx (%d/%u bytes)\n",
                    offset, ret, (unsigned)chunk);
            return -1;
        }

        offset += chunk;
        printf("\rsent 0x%zx / 0x%zx", offset, len);
        fflush(stdout);
    }
    putchar('\n');

    /* End the download.  The device may already be leaving DFU, so this is
     * deliberately best-effort, matching the reference usbliter8ctl tool. */
    (void)libusb_control_transfer(handle, REQTYPE_CLASS_OUT, DFU_DNLOAD,
                                   0, 0, NULL, 0, CTRL_TIMEOUT_MS);
    return 0;
}

int usbliter8_boot(const unsigned char *buf, size_t len)
{
    libusb_context *ctx = NULL;
    libusb_device_handle *handle = NULL;
    int ret;
    int claimed = 0;

    if (!buf || len == 0) {
        fprintf(stderr, "error: cannot boot an empty image\n");
        return -1;
    }

    ret = libusb_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "error: libusb_init failed: %s\n", libusb_error_name(ret));
        return -1;
    }

    handle = open_usbliter8_device(ctx);
    if (!handle)
        goto fail;

    ret = libusb_claim_interface(handle, 0);
    if (ret < 0) {
        fprintf(stderr, "error: could not claim DFU interface 0: %s\n",
                libusb_error_name(ret));
        goto fail;
    }
    claimed = 1;

    if (download_image(handle, buf, len) != 0)
        goto fail;

    ret = libusb_control_transfer(handle, REQTYPE_CLASS_OUT, CUSTOM_BOOT,
                                  0, 0, NULL, 0, CTRL_TIMEOUT_MS);
    if (ret < 0) {
        fprintf(stderr, "error: usbliter8 boot request failed: %s\n",
                libusb_error_name(ret));
        goto fail;
    }

    /* The reference tool sends this request after CUSTOM_BOOT.  It is normal
     * for it to fail because the device has already started rebooting. */
    (void)libusb_control_transfer(handle, REQTYPE_CLASS_OUT, DFU_ABORT,
                                  0, 0, NULL, 0, CTRL_TIMEOUT_MS);
    printf("usbliter8: boot triggered\n");

    if (claimed)
        (void)libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);
    return 0;

fail:
    if (claimed)
        (void)libusb_release_interface(handle, 0);
    if (handle)
        libusb_close(handle);
    libusb_exit(ctx);
    return -1;
}

int usbliter8_boot_file(const char *path)
{
    FILE *file;
    unsigned char *buf = NULL;
    long size;
    size_t length;
    int ret = -1;

    if (!path || !*path) {
        fprintf(stderr, "error: no boot image path supplied\n");
        return -1;
    }

    file = fopen(path, "rb");
    if (!file) {
        perror(path);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot determine size of %s\n", path);
        goto done;
    }
    length = (size_t)size;
    if ((long)length != size) {
        fprintf(stderr, "error: boot image is too large: %s\n", path);
        goto done;
    }

    buf = malloc(length);
    if (!buf) {
        fprintf(stderr, "error: out of memory reading %s\n", path);
        goto done;
    }
    if (fread(buf, 1, length, file) != length) {
        fprintf(stderr, "error: short read on %s\n", path);
        goto done;
    }

    printf("image: %s (%zu bytes)\n", path, length);
    ret = usbliter8_boot(buf, length);

done:
    free(buf);
    fclose(file);
    return ret;
}
