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
	eprom_state(const machine_config &mconfig, device_type type, const char *tag)
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
	uint16_t          m_sync_data;
	int         m_last_offset;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint16_t special_port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t adc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void eprom_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sync_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sync_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_klaxp();
	void init_guts();
	void init_eprom();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void guts_get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_eprom();
	void machine_reset_eprom();
	void video_start_eprom();
	void video_start_guts();
	uint32_t screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette();
	optional_device<cpu_device> m_extra;
	required_device<palette_device> m_palette;
	static const atari_motion_objects_config s_mob_config;
	static const atari_motion_objects_config s_guts_mob_config;
};
