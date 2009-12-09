/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _thunderj_state thunderj_state;
struct _thunderj_state
{
	atarigen_state	atarigen;
	UINT8			alpha_tile_bank;
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
VIDEO_UPDATE( thunderj );

void thunderj_mark_high_palette(bitmap_t *bitmap, UINT16 *pf, UINT16 *mo, int x, int y);
