// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "cpu/m68000/m68000.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_mo_command(*this, "mo_command") { }

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	bool            m_is_pitfight;

	uint8_t           m_which_input;
	required_shared_ptr<uint16_t> m_mo_command;

	uint16_t *        m_bslapstic_base;
	std::unique_ptr<uint8_t[]>          m_bslapstic_bank0;
	uint8_t           m_bslapstic_bank;
	bool            m_bslapstic_primed;

	int             m_pfscroll_xoffset;
	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	virtual void device_post_load() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	void mo_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t special_port0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void a2d_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t a2d_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pitfightb_cheap_slapstic_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void update_bank(int bank);
	void init_hydrap();
	void init_hydra();
	void init_pitfight();
	void init_pitfightb();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_atarig1();
	void machine_reset_atarig1();
	void video_start_atarig1();
	uint32_t screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	void pitfightb_cheap_slapstic_init();
};
