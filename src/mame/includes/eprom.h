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
	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_READ16_MEMBER(adc_r);
	DECLARE_WRITE16_MEMBER(eprom_latch_w);
	DECLARE_READ16_MEMBER(sync_r);
	DECLARE_WRITE16_MEMBER(sync_w);
	DECLARE_DRIVER_INIT(klaxp);
	DECLARE_DRIVER_INIT(guts);
	DECLARE_DRIVER_INIT(eprom);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(guts_get_playfield_tile_info);
	DECLARE_MACHINE_START(eprom);
	DECLARE_MACHINE_RESET(eprom);
	DECLARE_VIDEO_START(eprom);
	DECLARE_VIDEO_START(guts);
	UINT32 screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in video/eprom.c -----------*/
void eprom_scanline_update(screen_device &screen, int scanline);
