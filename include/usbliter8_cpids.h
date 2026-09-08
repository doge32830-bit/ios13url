#ifndef USBLITER8_CPIDS_H
#define USBLITER8_CPIDS_H

#include <stdbool.h>

/* These are the CPIDs implemented by the upstream usbliter8 firmware. */
#define USBLITER8_CPID_S4_S5 0x8006
#define USBLITER8_CPID_A12   0x8020
#define USBLITER8_CPID_A13   0x8030

static inline bool usbliter8_cpid_supported(unsigned int cpid)
{
    switch (cpid) {
    case USBLITER8_CPID_S4_S5:
    case USBLITER8_CPID_A12:
    case USBLITER8_CPID_A13:
        return true;
    default:
        return false;
    }
}

#endif
