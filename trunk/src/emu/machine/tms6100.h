#pragma once

#ifndef __TMS6100_H__
#define __TMS6100_H__

#include "devlegcy.h"

/* TMS 6100 memory controller */

WRITE_LINE_DEVICE_HANDLER( tms6100_m0_w );
WRITE_LINE_DEVICE_HANDLER( tms6100_m1_w );
WRITE_LINE_DEVICE_HANDLER( tms6100_romclock_w );
WRITE8_DEVICE_HANDLER( tms6100_addr_w );

READ_LINE_DEVICE_HANDLER( tms6100_data_r );

DECLARE_LEGACY_DEVICE(TMS6100, tms6100);
DECLARE_LEGACY_DEVICE(M58819, m58819);

#endif /* __TMS6100_H__ */
