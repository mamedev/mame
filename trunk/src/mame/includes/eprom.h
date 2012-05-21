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
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_READ16_MEMBER(adc_r);
	DECLARE_WRITE16_MEMBER(eprom_latch_w);
	DECLARE_READ16_MEMBER(sync_r);
	DECLARE_WRITE16_MEMBER(sync_w);
};


/*----------- defined in video/eprom.c -----------*/

VIDEO_START( eprom );
SCREEN_UPDATE_IND16( eprom );

VIDEO_START( guts );
SCREEN_UPDATE_IND16( guts );

void eprom_scanline_update(screen_device &screen, int scanline);
