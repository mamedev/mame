// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "sound/ym2151.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"

class gauntlet_state : public atarigen_state
{
public:
	gauntlet_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_soundcomm(*this, "soundcomm"),
			m_ym2151(*this, "ymsnd"),
			m_pokey(*this, "pokey"),
			m_tms5220(*this, "tms"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob")  { }

	required_device<cpu_device> m_audiocpu;
	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<ym2151_device> m_ym2151;
	required_device<pokey_device> m_pokey;
	required_device<tms5220_device> m_tms5220;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	uint16_t          m_sound_reset_val;
	uint8_t           m_vindctr2_screen_refresh;
	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	void sound_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t switch_6502_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_ctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mixer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void swap_memory(void *ptr1, void *ptr2, int bytes);
	void common_init(int vindctr2);
	void init_gauntlet();
	void init_vindctr2();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_gauntlet();
	void machine_reset_gauntlet();
	void video_start_gauntlet();
	uint32_t screen_update_gauntlet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gauntlet_xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gauntlet_yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	static const atari_motion_objects_config s_mob_config;
};
