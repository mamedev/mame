/*************************************************************************

    nmk112.h

**************************************************************************/

#ifndef __NMK112_H__
#define __NMK112_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _nmk112_interface nmk112_interface;
struct _nmk112_interface
{
	const char *rgn0, *rgn1;
	UINT8 disable_page_mask;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( nmk112 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define NMK112 DEVICE_GET_INFO_NAME( nmk112 )

#define MDRV_NMK112_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NMK112, 0) \
	MDRV_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER( nmk112_okibank_w );
WRITE16_DEVICE_HANDLER( nmk112_okibank_lsb_w );


#endif /* __NMK112_H__ */
