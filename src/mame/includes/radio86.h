// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/radio86.h
 *
 ****************************************************************************/

#ifndef radio86_H_
#define radio86_H_

#include "machine/i8255.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "imagedev/cassette.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class radio86_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	radio86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_dma8257(*this, "dma8257"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_ppi8255_2(*this, "ppi8255_2"),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_line8(*this, "LINE8"),
		m_io_cline0(*this, "CLINE0"),
		m_io_cline1(*this, "CLINE1"),
		m_io_cline2(*this, "CLINE2"),
		m_io_cline3(*this, "CLINE3"),
		m_io_cline4(*this, "CLINE4"),
		m_io_cline5(*this, "CLINE5"),
		m_io_cline6(*this, "CLINE6"),
		m_io_cline7(*this, "CLINE7"),
		m_palette(*this, "palette") { }

	virtual void video_start() override;

	uint8_t m_tape_value;
	uint8_t m_mikrosha_font_page;
	int m_keyboard_mask;
	std::unique_ptr<uint8_t[]> m_radio_ram_disk;
	uint8_t m_romdisk_lsb;
	uint8_t m_romdisk_msb;
	uint8_t m_disk_sel;
	const uint8_t *m_charmap;
	uint8_t radio_cpu_state_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t radio_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void radio_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void radio86_pagesel(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_radioram();
	void init_radio86();
	void machine_reset_radio86();
	void palette_init_radio86(palette_device &palette);
	uint8_t radio86_8255_portb_r2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t radio86_8255_portc_r2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void radio86_8255_porta_w2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void radio86_8255_portc_w2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rk7007_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hrq_w(int state);
	uint8_t radio86rom_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t radio86ram_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void radio86_romdisk_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void radio86_romdisk_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mikrosha_8255_font_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t memory_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void memory_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	required_device<cpu_device> m_maincpu;

protected:
	required_device<cassette_image_device> m_cassette;
	optional_device<generic_slot_device> m_cart;    // for ROMDisk - only Radio86K & Orion?
	optional_device<i8257_device> m_dma8257;
	required_device<i8255_device> m_ppi8255_1;
	optional_device<i8255_device> m_ppi8255_2;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	optional_ioport m_io_cline0;
	optional_ioport m_io_cline1;
	optional_ioport m_io_cline2;
	optional_ioport m_io_cline3;
	optional_ioport m_io_cline4;
	optional_ioport m_io_cline5;
	optional_ioport m_io_cline6;
	optional_ioport m_io_cline7;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void radio86_init_keyboard();
public:
	required_device<palette_device> m_palette;
};


/*----------- defined in drivers/radio86.c -----------*/

INPUT_PORTS_EXTERN( radio86 );
INPUT_PORTS_EXTERN( ms7007 );

#endif /* radio86_H_ */
