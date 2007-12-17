/*************************************************************************

Crazy Ballooon

*************************************************************************/

#include "sound/discrete.h"

#define CRBALOON_LAUGH_EN		NODE_01
#define CRBALOON_MUSIC_EN		NODE_02
#define CRBALOON_MUSIC_DATA		NODE_03


/*----------- defined in audio/crbaloon.c -----------*/

DISCRETE_SOUND_EXTERN( crbaloon );

/*----------- defined in video/crbaloon.c -----------*/

extern INT8 crbaloon_collision;

WRITE8_HANDLER( crbaloon_videoram_w );
WRITE8_HANDLER( crbaloon_colorram_w );
WRITE8_HANDLER( crbaloon_spritectrl_w );
WRITE8_HANDLER( crbaloon_flipscreen_w );

PALETTE_INIT( crbaloon );
VIDEO_START( crbaloon );
VIDEO_UPDATE( crbaloon );

