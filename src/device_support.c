#include <stddef.h>

#include <palerain.h>
#include "device_support.h"

bool is_usbliter8_cpid(unsigned int cpid)
{
    return usbliter8_cpid_supported(cpid);
}

const char *usbliter8_cpid_name(unsigned int cpid)
{
    switch (cpid) {
    case USBLITER8_CPID_S4_S5:
        return "S4/S5";
    case USBLITER8_CPID_A12:
        return "A12";
    case USBLITER8_CPID_A13:
        return "A13";
    default:
        return NULL;
    }
}

bool is_a12_a13(struct irecv_device_info *info)
{
    return info != NULL && is_usbliter8_cpid(info->cpid);
}

device_path_t get_device_path(struct irecv_device_info *info)
{
    if (info == NULL)
        return DEVICE_PATH_UNSUPPORTED;

    if (is_usbliter8_cpid(info->cpid))
        return DEVICE_PATH_USBLITER8;

    if (cpid_is_arm64(info->cpid))
        return DEVICE_PATH_CHECKRA1N;

    return DEVICE_PATH_UNSUPPORTED;
}
