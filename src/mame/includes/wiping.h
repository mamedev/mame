#include "devlegcy.h"

/*----------- defined in audio/wiping.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(WIPING, wiping_sound);

WRITE8_DEVICE_HANDLER( wiping_sound_w );


/*----------- defined in video/wiping.c -----------*/

extern UINT8 *wiping_videoram;
extern UINT8 *wiping_colorram;

WRITE8_HANDLER( wiping_flipscreen_w );
PALETTE_INIT( wiping );
VIDEO_UPDATE( wiping );

