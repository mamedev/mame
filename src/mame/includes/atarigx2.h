// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/atarixga.h"
#include "audio/atarijsa.h"


class atarigx2_state : public atarigen_state
{
public:
	atarigx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_xga(*this, "xga"),
			m_mo_command(*this, "mo_command"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle")
			{ }

	uint16_t          m_playfield_base;

	required_device<atari_jsa_iiis_device> m_jsa;
	required_device<atari_xga_device> m_xga;

	required_shared_ptr<uint32_t> m_mo_command;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	// LEGACY PROTECTION
	uint16_t          m_last_write;
	uint16_t          m_last_write_offset;
	uint32_t          m_protection_ram[0x1000];

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint32_t special_port2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t special_port3_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t a2d_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void latch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void mo_command_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void atarigx2_protection_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t atarigx2_protection_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t rrreveng_prot_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void init_spclords();
	void init_rrreveng();
	void init_motofren();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index atarigx2_playfield_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void machine_start_atarigx2();
	void machine_reset_atarigx2();
	void video_start_atarigx2();
	uint32_t screen_update_atarigx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atarigx2_mo_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
};
