/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, thunderj_state(machine)); }

	thunderj_state(running_machine &machine)
		: atarigen_state(machine) { }

	UINT8			alpha_tile_bank;
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
VIDEO_UPDATE( thunderj );

void thunderj_mark_high_palette(bitmap_t *bitmap, UINT16 *pf, UINT16 *mo, int x, int y);
