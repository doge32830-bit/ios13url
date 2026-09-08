#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <palerain.h>
#include "usbliter8_boot.h"

bool usbliter8_booted = false;

#ifndef NO_CUSTOM_PONGO
extern const uint8_t Pongo_bin[];
extern const uint32_t Pongo_bin_len;
#endif

char *get_embedded_pongo_path(void)
{
#ifndef NO_CUSTOM_PONGO
    char *path = strdup("/tmp/usbliter8-pongo-XXXXXX");
    int fd;
    FILE *file;
    size_t written;

    if (!path)
        return NULL;

    fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        free(path);
        return NULL;
    }

    file = fdopen(fd, "wb");
    if (!file) {
        perror("fdopen");
        close(fd);
        unlink(path);
        free(path);
        return NULL;
    }

    written = fwrite(Pongo_bin, 1, Pongo_bin_len, file);
    if (fclose(file) != 0 || written != Pongo_bin_len) {
        fprintf(stderr, "error: could not write embedded PongoOS image\n");
        unlink(path);
        free(path);
        return NULL;
    }

    return path;
#else
    return NULL;
#endif
}

int boot_usbliter8(void)
{
    char *temporary_path = NULL;
    const char *path = pongo_path;
    int ret;

    if (!path) {
        temporary_path = get_embedded_pongo_path();
        path = temporary_path;
    }
    if (!path) {
        LOG(LOG_ERROR, "No PongoOS image is available");
        return -1;
    }

    ret = usbliter8_boot_file(path);
    if (temporary_path) {
        unlink(temporary_path);
        free(temporary_path);
    }
    return ret;
}
