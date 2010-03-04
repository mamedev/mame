/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

class badlands_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, badlands_state(machine)); }

	badlands_state(running_machine &machine) { }

	atarigen_state	atarigen;

	UINT8			pedal_value[2];

	UINT8 *			bank_base;
	UINT8 *			bank_source_data;

	UINT8			playfield_tile_bank;
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
VIDEO_UPDATE( badlands );
