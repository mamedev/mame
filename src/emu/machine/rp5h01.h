/***************************************************************************

    RP5H01


***************************************************************************/

#ifndef __RP5H01_H__
#define __RP5H01_H__

#include "devlegcy.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(RP5H01, rp5h01);

#define MCFG_RP5H01_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, RP5H01, 0)

/*
 * Device uses memory region
 * with the same tag as the one
 * assigned to device.
 */

/***************************************************************************
    PROTOTYPES
***************************************************************************/

WRITE8_DEVICE_HANDLER( rp5h01_enable_w );	/* /CE */
WRITE8_DEVICE_HANDLER( rp5h01_reset_w );	/* RESET */
WRITE8_DEVICE_HANDLER( rp5h01_cs_w );	/* CS */
WRITE8_DEVICE_HANDLER( rp5h01_clock_w );	/* DATA CLOCK (active low) */
WRITE8_DEVICE_HANDLER( rp5h01_test_w );		/* TEST */
READ8_DEVICE_HANDLER( rp5h01_counter_r );	/* COUNTER OUT */
READ8_DEVICE_HANDLER( rp5h01_data_r );		/* DATA */

#endif /* __RP5H01_H__ */
