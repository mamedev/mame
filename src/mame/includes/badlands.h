/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _badlands_state badlands_state;
struct _badlands_state
{
	atarigen_state	atarigen;

	UINT8 			pedal_value[2];

	UINT8 *			bank_base;
	UINT8 *			bank_source_data;

	UINT8 			playfield_tile_bank;
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
VIDEO_UPDATE( badlands );
