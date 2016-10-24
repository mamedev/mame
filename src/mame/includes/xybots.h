// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Xybots hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class xybots_state : public atarigen_state
{
public:
	xybots_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob") { }

	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t          m_h256;
	virtual void update_interrupts() override;
	uint16_t special_port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void init_xybots();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_xybots();
	void machine_reset_xybots();
	void video_start_xybots();
	uint32_t screen_update_xybots(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
