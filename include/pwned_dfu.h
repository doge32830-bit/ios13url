#ifndef PWNED_DFU_H
#define PWNED_DFU_H

#include <libirecovery.h>

/* Returns 1 for a pwned device, 0 when unpwned, and -1 on error. */
int detect_pwned_dfu(irecv_client_t device);

/* Returns 1 only for the usbliter8 marker, 0 when it is another pwn. */
int detect_usbliter8_dfu(irecv_client_t device);

#endif
