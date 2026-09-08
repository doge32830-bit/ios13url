#include <assert.h>
#include <stdio.h>

#include "usbliter8_cpids.h"

int main(void)
{
    static const unsigned int supported[] = {
        USBLITER8_CPID_S4_S5,
        USBLITER8_CPID_A12,
        USBLITER8_CPID_A13,
    };
    static const unsigned int unsupported[] = {
        0x8015, /* A11 */
        0x8027, /* A12X/Z: no upstream usbliter8 implementation */
        0x8033, /* A14 */
    };
    size_t i;

    for (i = 0; i < sizeof(supported) / sizeof(supported[0]); ++i)
        assert(usbliter8_cpid_supported(supported[i]));
    for (i = 0; i < sizeof(unsupported) / sizeof(unsupported[0]); ++i)
        assert(!usbliter8_cpid_supported(unsupported[i]));

    puts("usbliter8 CPID table: OK");
    return 0;
}
