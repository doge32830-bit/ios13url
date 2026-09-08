#include <string.h>

#include <libirecovery.h>
#include <palerain.h>
#include <pwned_dfu.h>

static const char *dfu_serial(irecv_client_t device)
{
    const struct irecv_device_info *info;

    if (!device)
        return NULL;

    info = irecv_get_device_info(device);
    return info ? info->serial_string : NULL;
}

int detect_pwned_dfu(irecv_client_t device)
{
    const char *serial = dfu_serial(device);

    if (!serial) {
        LOG(LOG_ERROR, "Unable to query DFU device serial string");
        return -1;
    }

    if (strstr(serial, "PWND:")) {
        LOG(LOG_INFO, "Pwned DFU marker detected: %s", serial);
        return 1;
    }

    return 0;
}

int detect_usbliter8_dfu(irecv_client_t device)
{
    const char *serial = dfu_serial(device);

    if (!serial)
        return -1;

    return strstr(serial, "PWND:[usbliter8]") != NULL;
}
