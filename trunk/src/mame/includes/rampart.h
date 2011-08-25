/*************************************************************************

    Atari Rampart hardware

*************************************************************************/

#include "machine/atarigen.h"

class rampart_state : public atarigen_state
{
public:
	rampart_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16 *		m_bitmap;
	UINT8			m_has_mo;
};


/*----------- defined in video/rampart.c -----------*/

VIDEO_START( rampart );
SCREEN_UPDATE( rampart );
