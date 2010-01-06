#ifndef __TAITOSND_H__
#define __TAITOSND_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _tc0140syt_interface tc0140syt_interface;
struct _tc0140syt_interface
{
	const char         *master;
	const char         *slave;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( tc0140syt );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define TC0140SYT DEVICE_GET_INFO_NAME( tc0140syt )

#define MDRV_TC0140SYT_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0140SYT, 0) \
	MDRV_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/* MASTER (8bit bus) control functions */
WRITE8_DEVICE_HANDLER( tc0140syt_port_w );
WRITE8_DEVICE_HANDLER( tc0140syt_comm_w );
READ8_DEVICE_HANDLER( tc0140syt_comm_r );


/* SLAVE (8bit bus) control functions ONLY */
WRITE8_DEVICE_HANDLER( tc0140syt_slave_port_w );
READ8_DEVICE_HANDLER( tc0140syt_slave_comm_r );
WRITE8_DEVICE_HANDLER( tc0140syt_slave_comm_w );


#endif /*__TAITOSND_H__*/
