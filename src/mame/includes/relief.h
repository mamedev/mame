/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _relief_state relief_state;
struct _relief_state
{
	atarigen_state	atarigen;

	UINT8 			ym2413_volume;
	UINT8 			overall_volume;
	UINT32 			adpcm_bank_base;
};


/*----------- defined in video/relief.c -----------*/

VIDEO_START( relief );
VIDEO_UPDATE( relief );
