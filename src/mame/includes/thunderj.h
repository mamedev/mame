/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			alpha_tile_bank;
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
VIDEO_UPDATE( thunderj );

void thunderj_mark_high_palette(bitmap_t *bitmap, UINT16 *pf, UINT16 *mo, int x, int y);
