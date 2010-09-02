/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class relief_state : public atarigen_state
{
public:
	relief_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			ym2413_volume;
	UINT8			overall_volume;
	UINT32			adpcm_bank_base;
};


/*----------- defined in video/relief.c -----------*/

VIDEO_START( relief );
VIDEO_UPDATE( relief );
