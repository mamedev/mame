// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Skull & Crossbones hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class skullxbo_state : public atarigen_state
{
public:
	skullxbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_scanline_timer(*this, "scan_timer"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_playfield_latch(-1) { }

	required_device<atari_jsa_ii_device> m_jsa;
	required_device<timer_device> m_scanline_timer;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	int m_playfield_latch;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	void skullxbo_halt_until_hblank_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void skullxbo_mobwr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_skullxbo();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void playfield_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	void playfield_latched_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	void machine_start_skullxbo();
	void machine_reset_skullxbo();
	void video_start_skullxbo();
	uint32_t screen_update_skullxbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void scanline_timer(timer_device &timer, void *ptr, int32_t param);
	void skullxbo_scanline_update(int scanline);
	void skullxbo_xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void skullxbo_yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void skullxbo_mobmsb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	static const atari_motion_objects_config s_mob_config;
};
