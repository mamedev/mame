/*************************************************************************

    Atari Klax hardware

*************************************************************************/

#include "machine/atarigen.h"

class klax_state : public atarigen_state
{
public:
	klax_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }
};


/*----------- defined in video/klax.c -----------*/

WRITE16_HANDLER( klax_latch_w );

VIDEO_START( klax );
VIDEO_UPDATE( klax );
