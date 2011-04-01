/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			m_alpha_tile_bank;
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
SCREEN_UPDATE( thunderj );
