/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"

class rampart_state : public atarigen_state
{
public:
	rampart_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *		bitmap;
	UINT8			has_mo;
};


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
SCREEN_UPDATE( rampart );
