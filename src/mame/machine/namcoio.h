#ifndef __NAMCOIO_H__
#define __NAMCOIO_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _namcoio_interface namcoio_interface;
struct _namcoio_interface
{
	devcb_read8 in[4];
	devcb_write8 out[2];

	const device_config *device;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( namcoio );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define NAMCO56XX DEVICE_GET_INFO_NAME( namcoio )
#define NAMCO58XX DEVICE_GET_INFO_NAME( namcoio )
#define NAMCO59XX DEVICE_GET_INFO_NAME( namcoio )

#define MDRV_NAMCO56XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO56XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_NAMCO58XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO58XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_NAMCO59XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO59XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ8_DEVICE_HANDLER( namcoio_r );
WRITE8_DEVICE_HANDLER( namcoio_w );

WRITE_LINE_DEVICE_HANDLER( namcoio_set_reset_line );
READ_LINE_DEVICE_HANDLER( namcoio_read_reset_line );


/* these must be used in the single drivers, inside a timer */
void namco_customio_56xx_run(const device_config *device);
void namco_customio_58xx_run(const device_config *device);
void namco_customio_59xx_run(const device_config *device);


#endif	/* __NAMCOIO_H__ */
