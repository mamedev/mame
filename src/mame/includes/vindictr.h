// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class vindictr_state : public atarigen_state
{
public:
	vindictr_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_jsa(*this, "jsa") { }

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_i_device> m_jsa;
	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint16_t port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_vindictr();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_vindictr();
	void machine_reset_vindictr();
	void video_start_vindictr();
	uint32_t screen_update_vindictr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vindictr_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	static const atari_motion_objects_config s_mob_config;
};
