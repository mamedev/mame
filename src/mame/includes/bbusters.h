#include "video/bufsprite.h"

class bbusters_state : public driver_device
{
public:
	bbusters_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_eprom_data(*this, "eeprom"),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") { }

	UINT16 *m_videoram;
	UINT16 *m_ram;
	optional_shared_ptr<UINT16> m_eprom_data;
	int m_sound_status;
	int m_gun_select;

	tilemap_t *m_fix_tilemap;
	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	const UINT8 *m_scale_table_ptr;
	UINT8 m_scale_line_count;

	UINT16 *m_pf1_data;
	UINT16 *m_pf2_data;
	UINT16 *m_pf1_scroll_data;
	UINT16 *m_pf2_scroll_data;

	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram2;
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_status_w);
	DECLARE_WRITE16_MEMBER(sound_cpu_w);
	DECLARE_READ16_MEMBER(eprom_r);
	DECLARE_READ16_MEMBER(control_3_r);
	DECLARE_WRITE16_MEMBER(gun_select_w);
	DECLARE_WRITE16_MEMBER(two_gun_output_w);
	DECLARE_WRITE16_MEMBER(three_gun_output_w);
	DECLARE_READ16_MEMBER(kludge_r);
	DECLARE_READ16_MEMBER(mechatt_gun_r);
	DECLARE_WRITE16_MEMBER(bbusters_video_w);
	DECLARE_WRITE16_MEMBER(bbusters_pf1_w);
	DECLARE_WRITE16_MEMBER(bbusters_pf2_w);
};


/*----------- defined in video/bbusters.c -----------*/

VIDEO_START( bbuster );
VIDEO_START( mechatt );
SCREEN_UPDATE_IND16( bbuster );
SCREEN_UPDATE_IND16( mechatt );

