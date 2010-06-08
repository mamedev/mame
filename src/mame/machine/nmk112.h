/*************************************************************************

    nmk112.h

**************************************************************************/

#ifndef __NMK112_H__
#define __NMK112_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _nmk112_interface nmk112_interface;
struct _nmk112_interface
{
	const char *rgn0, *rgn1;
	UINT8 disable_page_mask;
};

DECLARE_LEGACY_DEVICE(NMK112, nmk112);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_NMK112_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NMK112, 0) \
	MDRV_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER( nmk112_okibank_w );
WRITE16_DEVICE_HANDLER( nmk112_okibank_lsb_w );


#endif /* __NMK112_H__ */
