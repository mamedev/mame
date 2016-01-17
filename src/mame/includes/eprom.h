// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Escape hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class eprom_state : public atarigen_state
{
public:
	eprom_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_jsa(*this, "jsa"),
			m_extra(*this, "extra"),
			m_palette(*this, "palette") { }

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_base_device> m_jsa;
	int             m_screen_intensity;
	int             m_video_disable;
	UINT16 *        m_sync_data;
	int         m_last_offset;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
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
	void update_palette();
	optional_device<cpu_device> m_extra;
	required_device<palette_device> m_palette;
	static const atari_motion_objects_config s_mob_config;
	static const atari_motion_objects_config s_guts_mob_config;
};
