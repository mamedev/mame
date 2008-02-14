/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "sound/custom.h"


/*----------- defined in audio/segag80v.c -----------*/

WRITE8_HANDLER( elim1_sh_w );
WRITE8_HANDLER( elim2_sh_w );
WRITE8_HANDLER( spacfury1_sh_w );
WRITE8_HANDLER( spacfury2_sh_w );
WRITE8_HANDLER( zektor1_sh_w );
WRITE8_HANDLER( zektor2_sh_w );


/*----------- defined in video/segag80v.c -----------*/

VIDEO_START( sega );
VIDEO_UPDATE( sega );
