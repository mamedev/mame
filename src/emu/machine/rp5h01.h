/***************************************************************************

    RP5H01


***************************************************************************/

#ifndef __RP5H01_H__
#define __RP5H01_H__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define RP5H01		DEVICE_GET_INFO_NAME(rp5h01)

#define MDRV_RP5H01_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, RP5H01, 0)

/*
 * Device uses memory region
 * with the same tag as the one
 * assigned to device.
 */

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( rp5h01 );

WRITE8_DEVICE_HANDLER( rp5h01_enable_w );	/* /CE */
WRITE8_DEVICE_HANDLER( rp5h01_reset_w );	/* RESET */
WRITE8_DEVICE_HANDLER( rp5h01_clock_w );	/* DATA CLOCK (active low) */
WRITE8_DEVICE_HANDLER( rp5h01_test_w );		/* TEST */
READ8_DEVICE_HANDLER( rp5h01_counter_r );	/* COUNTER OUT */
READ8_DEVICE_HANDLER( rp5h01_data_r );		/* DATA */

#endif /* __RP5H01_H__ */
