#ifndef USBLITER8_BOOT_H
#define USBLITER8_BOOT_H

#include <stddef.h>

/* Send a raw image to a device already pwned by usbliter8. */
int usbliter8_boot(const unsigned char *buf, size_t len);
int usbliter8_boot_file(const char *path);

#endif
