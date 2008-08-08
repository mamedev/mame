/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "sound/custom.h"

/*----------- defined in audio/cps3.c -----------*/

void *cps3_sh_start(int clock, const custom_sound_interface *config);

WRITE32_HANDLER( cps3_sound_w );
READ32_HANDLER( cps3_sound_r );

