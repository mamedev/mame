// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ut88.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_UT88_H
#define MAME_INCLUDES_UT88_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "emupal.h"


class ut88_common : public driver_device
{
public:
	ut88_common(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_io_line0(*this, "LINE0")
		, m_io_line1(*this, "LINE1")
		, m_io_line2(*this, "LINE2")
	{
	}

protected:
	uint8_t tape_r();

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
};

class ut88_state : public ut88_common
{
public:
	ut88_state(const machine_config &mconfig, device_type type, const char *tag)
		: ut88_common(mconfig, type, tag)
		, m_ppi(*this, "ppi")
		, m_dac(*this, "dac")
		, m_vram(*this, "videoram")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_io_line3(*this, "LINE3")
		, m_io_line4(*this, "LINE4")
		, m_io_line5(*this, "LINE5")
		, m_io_line6(*this, "LINE6")
		, m_io_line7(*this, "LINE7")
		, m_io_line8(*this, "LINE8")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void ut88(machine_config &config);

private:
	void machine_start() override;
	void machine_reset() override;
	uint8_t keyboard_r(offs_t offset);
	void keyboard_w(offs_t offset, uint8_t data);
	void sound_w(uint8_t data);
	uint8_t ppi_portb_r();
	uint8_t ppi_portc_r();
	void ppi_porta_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	int m_keyboard_mask;

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<i8255_device> m_ppi;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

class ut88mini_state : public ut88_common
{
public:
	ut88mini_state(const machine_config &mconfig, device_type type, const char *tag)
		: ut88_common(mconfig, type, tag)
		, m_proms(*this, "proms")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void ut88mini(machine_config &config);

private:
	void machine_start() override;
	void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(display_timer);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	uint8_t keyboard_r();
	void led_w(offs_t offset, uint8_t data);

	required_memory_region m_proms;

	int m_lcd_digit[6];
	output_finder<6> m_digits;
};


#endif // MAME_INCLUDES_UT88_H
