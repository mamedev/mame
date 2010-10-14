/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "devlegcy.h"

/*----------- defined in drivers/cps3.c -----------*/

extern UINT8* cps3_user5region;


/*----------- defined in audio/cps3.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(CPS3, cps3_sound);

WRITE32_DEVICE_HANDLER( cps3_sound_w );
READ32_DEVICE_HANDLER( cps3_sound_r );
