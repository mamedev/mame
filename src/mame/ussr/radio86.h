// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/radio86.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_RADIO86_H
#define MAME_USSR_RADIO86_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "video/i8275.h"

#include "emupal.h"


class radio86_state : public driver_device
{
public:
	radio86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_dma(*this, "dma")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_io_line(*this, "LINE%u", 0U)
		, m_io_cline(*this, "CLINE%u", 0U)
		, m_palette(*this, "palette")
		, m_chargen(*this, "chargen")
	{ }

	void impuls03(machine_config &config);
	void mikron2(machine_config &config);
	void rk7007(machine_config &config);
	void rk700716(machine_config &config);
	void radiorom(machine_config &config);
	void radio86(machine_config &config);
	void radio16(machine_config &config);
	void radioram(machine_config &config);
	void kr03(machine_config &config);
	void init_radioram();
	void init_radio86();

protected:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void radio86_init_keyboard();

	void impuls03_mem(address_map &map) ATTR_COLD;
	void mikron2_mem(address_map &map) ATTR_COLD;
	void radio86_16_mem(address_map &map) ATTR_COLD;
	void radio86_io(address_map &map) ATTR_COLD;
	void radio86_mem(address_map &map) ATTR_COLD;
	void radio86ram_mem(address_map &map) ATTR_COLD;
	void radio86rom_mem(address_map &map) ATTR_COLD;
	void rk7007_io(address_map &map) ATTR_COLD;

	uint8_t m_tape_value;
	uint8_t radio_cpu_state_r();
	uint8_t radio_io_r(offs_t offset);
	void radio_io_w(offs_t offset, uint8_t data);
	uint8_t radio86_8255_portb_r2();
	uint8_t radio86_8255_portc_r2();
	uint8_t rk7007_8255_portc_r();
	void radio86_8255_porta_w2(uint8_t data);
	void radio86_8255_portc_w2(uint8_t data);
	void radio86_palette(palette_device &palette) const;
	void hrq_w(int state);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	optional_device<generic_slot_device> m_cart;    // for ROMDisk - only Radio86K & Orion?
	optional_device<i8257_device> m_dma;
	required_device<i8255_device> m_ppi1;
	optional_device<i8255_device> m_ppi2;
	required_region_ptr<u8> m_rom;
	optional_shared_ptr<u8> m_ram;
	optional_memory_bank m_bank1;
	required_ioport_array<9> m_io_line;
	optional_ioport_array<8> m_io_cline;
	required_device<palette_device> m_palette;
	optional_region_ptr<uint8_t> m_chargen;

private:
	int m_keyboard_mask = 0;
	std::unique_ptr<uint8_t[]> m_radio_ram_disk{};
	uint8_t m_romdisk_lsb = 0;
	uint8_t m_romdisk_msb = 0;
	uint8_t m_disk_sel = 0;
	void radio86_pagesel(uint8_t data);
	uint8_t kr03_8255_portb_r2();
	uint8_t radio86rom_romdisk_porta_r();
	uint8_t radio86ram_romdisk_porta_r();
	void radio86_romdisk_portb_w(uint8_t data);
	void radio86_romdisk_portc_w(uint8_t data);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
};


/*----------- defined in drivers/radio86.c -----------*/

INPUT_PORTS_EXTERN( radio86 );
INPUT_PORTS_EXTERN( ms7007 );

#endif // MAME_USSR_RADIO86_H
