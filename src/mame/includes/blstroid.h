/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"

class blstroid_state : public atarigen_state
{
public:
	blstroid_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16 *		m_priorityram;
};


/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
SCREEN_UPDATE( blstroid );

void blstroid_scanline_update(screen_device &screen, int scanline);
