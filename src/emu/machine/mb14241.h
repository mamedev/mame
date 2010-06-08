/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef __MB14241_H__
#define __MB14241_H__

#include "devlegcy.h"


DECLARE_LEGACY_DEVICE(MB14241, mb14241);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_MB14241_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MB14241, 0)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER ( mb14241_shift_count_w );
WRITE8_DEVICE_HANDLER ( mb14241_shift_data_w );
READ8_DEVICE_HANDLER( mb14241_shift_result_r );


#endif /* __MB14241_H__ */
