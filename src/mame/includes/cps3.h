/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "sound/custom.h"

/*----------- defined in audio/cps3.c -----------*/

CUSTOM_START( cps3_sh_start );

WRITE32_HANDLER( cps3_sound_w );
READ32_HANDLER( cps3_sound_r );

