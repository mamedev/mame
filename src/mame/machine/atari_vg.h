/***************************************************************************

    Atari vector hardware

***************************************************************************/

#include "devlegcy.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_ATARIVGEAROM_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, ATARIVGEAROM, 0)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( atari_vg_earom_r );
WRITE8_DEVICE_HANDLER( atari_vg_earom_w );
WRITE8_DEVICE_HANDLER( atari_vg_earom_ctrl_w );

/* ----- device interface ----- */

DECLARE_LEGACY_NVRAM_DEVICE(ATARIVGEAROM, atari_vg_earom);
