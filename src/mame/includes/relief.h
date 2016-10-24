// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"

class relief_state : public atarigen_state
{
public:
	relief_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_vad(*this, "vad"),
			m_oki(*this, "oki"),
			m_ym2413(*this, "ymsnd"),
			m_okibank(*this, "okibank")
			{ }

	required_device<atari_vad_device> m_vad;
	required_device<okim6295_device> m_oki;
	required_device<ym2413_device> m_ym2413;
	required_memory_bank m_okibank;

	uint8_t           m_ym2413_volume;
	uint8_t           m_overall_volume;
	uint8_t           m_adpcm_bank;
	virtual void update_interrupts() override;
	uint16_t special_port2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void audio_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void audio_volume_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_relief();
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_relief();
	void machine_reset_relief();
	void video_start_relief();
	uint32_t screen_update_relief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
