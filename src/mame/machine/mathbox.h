/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */

#include "devlegcy.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_MATHBOX_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MATHBOX, 0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

WRITE8_DEVICE_HANDLER( mathbox_go_w );
READ8_DEVICE_HANDLER( mathbox_status_r );
READ8_DEVICE_HANDLER( mathbox_lo_r );
READ8_DEVICE_HANDLER( mathbox_hi_r );

/* ----- device interface ----- */
DECLARE_LEGACY_DEVICE(MATHBOX, mathbox);
