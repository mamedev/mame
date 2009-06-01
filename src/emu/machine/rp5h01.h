/***************************************************************************

    RP5H01


***************************************************************************/

#ifndef __RP5H01_H__
#define __RP5H01_H__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define RP5H01		DEVICE_GET_INFO_NAME(rp5h01)

#define MDRV_RP5H01_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, RP5H01, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_RP5H01_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _rp5h01_interface rp5h01_interface;
struct _rp5h01_interface
{
	const char *region;		/* memory region where data resides */
	int offset;		/* memory offset within the above region where data resides */
};

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
