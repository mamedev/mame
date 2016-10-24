// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class batman_state : public atarigen_state
{
public:
	batman_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_vad(*this, "vad") { }

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;

	uint16_t          m_latch_data;
	uint8_t           m_alpha_tile_bank;

	virtual void update_interrupts() override;
	void latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_batman();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_batman();
	void machine_reset_batman();
	void video_start_batman();
	uint32_t screen_update_batman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
