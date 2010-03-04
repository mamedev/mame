/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class relief_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, relief_state(machine)); }

	relief_state(running_machine &machine) { }

	atarigen_state	atarigen;

	UINT8			ym2413_volume;
	UINT8			overall_volume;
	UINT32			adpcm_bank_base;
};


/*----------- defined in video/relief.c -----------*/

VIDEO_START( relief );
VIDEO_UPDATE( relief );
