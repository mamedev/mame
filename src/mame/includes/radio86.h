// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/radio86.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_RADIO86_H
#define MAME_INCLUDES_RADIO86_H

#pragma once

#include "machine/i8255.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "imagedev/cassette.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"


class radio86_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	radio86_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_dma8257(*this, "dma8257"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_ppi8255_2(*this, "ppi8255_2"),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_io_line(*this, "LINE%u", 0),
		m_io_cline(*this, "CLINE%u", 0),
		m_palette(*this, "palette"),
		m_charmap(*this, "gfx1")
	{ }

	uint8_t m_tape_value;
	uint8_t m_mikrosha_font_page;
	int m_keyboard_mask;
	std::unique_ptr<uint8_t[]> m_radio_ram_disk;
	uint8_t m_romdisk_lsb;
	uint8_t m_romdisk_msb;
	uint8_t m_disk_sel;
	uint8_t radio_cpu_state_r();
	uint8_t radio_io_r(offs_t offset);
	void radio_io_w(offs_t offset, uint8_t data);
	void radio86_pagesel(uint8_t data);
	void init_radioram();
	void init_radio86();
	void radio86_palette(palette_device &palette) const;
	uint8_t radio86_8255_portb_r2();
	uint8_t radio86_8255_portc_r2();
	void radio86_8255_porta_w2(uint8_t data);
	void radio86_8255_portc_w2(uint8_t data);
	uint8_t rk7007_8255_portc_r();
	uint8_t kr03_8255_portb_r2();
	void hrq_w(int state);
	DECLARE_READ8_MEMBER(radio86rom_romdisk_porta_r);
	uint8_t radio86ram_romdisk_porta_r();
	void radio86_romdisk_portb_w(uint8_t data);
	void radio86_romdisk_portc_w(uint8_t data);
	void mikrosha_8255_font_page_w(uint8_t data);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	required_device<cpu_device> m_maincpu;

	void impuls03(machine_config &config);
	void mikron2(machine_config &config);
	void rk7007(machine_config &config);
	void rk700716(machine_config &config);
	void radiorom(machine_config &config);
	void radio86(machine_config &config);
	void radio16(machine_config &config);
	void radioram(machine_config &config);
	void kr03(machine_config &config);

protected:
	required_device<cassette_image_device> m_cassette;
	optional_device<generic_slot_device> m_cart;    // for ROMDisk - only Radio86K & Orion?
	optional_device<i8257_device> m_dma8257;
	required_device<i8255_device> m_ppi8255_1;
	optional_device<i8255_device> m_ppi8255_2;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_ioport_array<9> m_io_line;
	optional_ioport_array<8> m_io_cline;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_reset() override;

	void radio86_init_keyboard();

	void impuls03_mem(address_map &map);
	void mikron2_mem(address_map &map);
	void radio86_16_mem(address_map &map);
	void radio86_io(address_map &map);
	void radio86_mem(address_map &map);
	void radio86ram_mem(address_map &map);
	void radio86rom_mem(address_map &map);
	void rk7007_io(address_map &map);

	required_device<palette_device> m_palette;
	optional_region_ptr<uint8_t> m_charmap;
};


/*----------- defined in drivers/radio86.c -----------*/

INPUT_PORTS_EXTERN( radio86 );
INPUT_PORTS_EXTERN( ms7007 );

#endif // MAME_INCLUDES_RADIO86_H
