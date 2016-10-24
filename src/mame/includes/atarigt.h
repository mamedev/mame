// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GT hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/cage.h"

#define CRAM_ENTRIES        0x4000
#define TRAM_ENTRIES        0x4000
#define MRAM_ENTRIES        0x8000

#define ADDRSEQ_COUNT   4

class atarigt_state : public atarigen_state
{
public:
	atarigt_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_colorram(*this, "colorram", 32),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_mo_command(*this, "mo_command"),
			m_cage(*this, "cage") { }

	uint8_t           m_is_primrage;
	required_shared_ptr<uint16_t> m_colorram;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	bitmap_ind16    m_pf_bitmap;
	bitmap_ind16    m_an_bitmap;

	uint8_t           m_playfield_tile_bank;
	uint8_t           m_playfield_color_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

	uint32_t          m_tram_checksum;

	uint32_t          m_expanded_mram[MRAM_ENTRIES * 3];

	required_shared_ptr<uint32_t> m_mo_command;
	optional_device<atari_cage_device> m_cage;

	void            (atarigt_state::*m_protection_w)(address_space &space, offs_t offset, uint16_t data);
	void            (atarigt_state::*m_protection_r)(address_space &space, offs_t offset, uint16_t *data);

	bool            m_ignore_writes;
	offs_t          m_protaddr[ADDRSEQ_COUNT];
	uint8_t           m_protmode;
	uint16_t          m_protresult;
	uint8_t           m_protdata[0x800];

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	uint32_t special_port2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t special_port3_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t analog_port0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t analog_port1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void latch_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void mo_command_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void led_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t sound_data_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sound_data_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t colorram_protection_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void colorram_protection_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tmek_pf_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void cage_irq_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void atarigt_colorram_w(offs_t address, uint16_t data, uint16_t mem_mask);
	uint16_t atarigt_colorram_r(offs_t address);
	void init_primrage();
	void init_tmek();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index atarigt_playfield_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void machine_start_atarigt();
	void machine_reset_atarigt();
	void video_start_atarigt();
	uint32_t screen_update_atarigt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
private:
	void tmek_update_mode(offs_t offset);
	void tmek_protection_w(address_space &space, offs_t offset, uint16_t data);
	void tmek_protection_r(address_space &space, offs_t offset, uint16_t *data);
	void primrage_update_mode(offs_t offset);
	void primrage_protection_w(address_space &space, offs_t offset, uint16_t data);
	void primrage_protection_r(address_space &space, offs_t offset, uint16_t *data);
	void compute_fake_pots(int *pots);
};
