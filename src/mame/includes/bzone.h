/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "sound/custom.h"


/*----------- defined in drivers/bzone.c -----------*/

extern UINT8 rb_input_select;


/*----------- defined in audio/bzone.c -----------*/

WRITE8_HANDLER( bzone_sounds_w );

void *bzone_sh_start(int clock, const custom_sound_interface *config);


/*----------- defined in audio/redbaron.c -----------*/

WRITE8_HANDLER( redbaron_sounds_w );
WRITE8_HANDLER( redbaron_pokey_w );

void *redbaron_sh_start(int clock, const custom_sound_interface *config);
