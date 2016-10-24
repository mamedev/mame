// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "cpu/m68000/m68000.h"
#include "machine/asic65.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_asic65(*this, "asic65"),
			m_mo_command(*this, "mo_command") { }

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;
	required_device<asic65_device> m_asic65;

	uint16_t          m_playfield_base;

	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	uint8_t           m_analog_data;
	required_shared_ptr<uint16_t> m_mo_command;

	int             m_sloop_bank;
	int             m_sloop_next_bank;
	int             m_sloop_offset;
	int             m_sloop_state;
	uint16_t *        m_sloop_base;

	uint32_t          m_last_accesses[8];
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint16_t special_port2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void a2d_select_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t a2d_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void io_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mo_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t roadriot_sloop_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void roadriot_sloop_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t guardians_sloop_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void guardians_sloop_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roadriot_sloop_tweak(int offset);
	void guardians_sloop_tweak(int offset);
	void init_roadriot();
	void init_guardian();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index atarig42_playfield_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void machine_start_atarig42();
	void machine_reset_atarig42();
	void video_start_atarig42();
	uint32_t screen_update_atarig42(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
