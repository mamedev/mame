/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "sound/custom.h"


/*----------- defined in drivers/bzone.c -----------*/

extern UINT8 rb_input_select;


/*----------- defined in audio/bzone.c -----------*/

WRITE8_HANDLER( bzone_sounds_w );

CUSTOM_START( bzone_sh_start );


/*----------- defined in audio/redbaron.c -----------*/

WRITE8_HANDLER( redbaron_sounds_w );
WRITE8_HANDLER( redbaron_pokey_w );

CUSTOM_START( redbaron_sh_start );
