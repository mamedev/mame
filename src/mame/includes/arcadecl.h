/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/


#include "machine/atarigen.h"

class arcadecl_state : public atarigen_state
{
public:
	arcadecl_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *		m_bitmap;
	UINT8			m_has_mo;
};

/*----------- defined in video/arcadecl.c -----------*/

VIDEO_START( arcadecl );
SCREEN_UPDATE( arcadecl );
