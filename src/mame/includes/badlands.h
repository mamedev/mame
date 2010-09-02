/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

class badlands_state : public atarigen_state
{
public:
	badlands_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			pedal_value[2];

	UINT8 *			bank_base;
	UINT8 *			bank_source_data;

	UINT8			playfield_tile_bank;
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
VIDEO_UPDATE( badlands );
