// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_vad(*this, "vad"),
			m_extra(*this, "extra") { }

	required_device<atari_jsa_ii_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<cpu_device> m_extra;

	uint8_t           m_alpha_tile_bank;
	virtual void update_interrupts() override;
	uint16_t special_port2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_thunderj();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_thunderj();
	void machine_reset_thunderj();
	void video_start_thunderj();
	uint32_t screen_update_thunderj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
