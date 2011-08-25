/*************************************************************************

    Atari Escape hardware

*************************************************************************/

#include "machine/atarigen.h"

class eprom_state : public atarigen_state
{
public:
	eprom_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	int 			m_screen_intensity;
	int 			m_video_disable;
	UINT16 *		m_sync_data;
	int			m_last_offset;
};


/*----------- defined in video/eprom.c -----------*/

VIDEO_START( eprom );
SCREEN_UPDATE( eprom );

VIDEO_START( guts );
SCREEN_UPDATE( guts );

void eprom_scanline_update(screen_device &screen, int scanline);
