/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class offtwall_state : public atarigen_state
{
public:
	offtwall_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *bankswitch_base;
	UINT16 *bankrom_base;
	UINT32 bank_offset;

	UINT16 *spritecache_count;
	UINT16 *unknown_verify_base;
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
VIDEO_UPDATE( offtwall );
