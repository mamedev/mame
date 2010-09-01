/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "devlegcy.h"

#define BZONE_MASTER_CLOCK (XTAL_12_096MHz)
#define BZONE_CLOCK_3KHZ  (MASTER_CLOCK / 4096)

/*----------- defined in drivers/bzone.c -----------*/

extern UINT8 rb_input_select;


/*----------- defined in audio/bzone.c -----------*/


WRITE8_DEVICE_HANDLER( bzone_sounds_w );

MACHINE_CONFIG_EXTERN( bzone_audio );

/*----------- defined in audio/redbaron.c -----------*/

WRITE8_HANDLER( redbaron_sounds_w );

DECLARE_LEGACY_SOUND_DEVICE(REDBARON, redbaron_sound);
