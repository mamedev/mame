/*************************************************************************

    Atari Shuuz hardware

*************************************************************************/

#include "machine/atarigen.h"

class shuuz_state : public atarigen_state
{
public:
	shuuz_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	int cur[2];
};


/*----------- defined in video/shuuz.c -----------*/

VIDEO_START( shuuz );
SCREEN_UPDATE( shuuz );
