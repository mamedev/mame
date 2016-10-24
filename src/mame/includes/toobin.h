// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class toobin_state : public atarigen_state
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_interrupt_scan(*this, "interrupt_scan") { }

	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	required_shared_ptr<uint16_t> m_interrupt_scan;

	double          m_brightness;
	bitmap_ind16 m_pfbitmap;

	virtual void update_interrupts() override;

	void interrupt_scan_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void intensity_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void slip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void machine_start_toobin();
	void machine_reset_toobin();
	void video_start_toobin();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
