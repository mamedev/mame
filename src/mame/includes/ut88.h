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
#include "sound/dac.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"
#include "emupal.h"

class ut88_base_state : public driver_device
{
public:
	ut88_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cassette(*this, "cassette")
		, m_region_maincpu(*this, "maincpu")
		, m_io_line0(*this, "LINE0")
		, m_io_line1(*this, "LINE1")
		, m_io_line2(*this, "LINE2")
		, m_maincpu(*this, "maincpu")
	{
	}

protected:
	uint8_t tape_r();

	required_device<cassette_image_device> m_cassette;
	required_memory_region m_region_maincpu;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_device<cpu_device> m_maincpu;
};

class ut88_state : public ut88_base_state
{
public:
	ut88_state(const machine_config &mconfig, device_type type, const char *tag)
		: ut88_base_state(mconfig, type, tag)
		, m_ppi(*this, "ppi8255")
		, m_dac(*this, "dac")
		, m_p_videoram(*this, "videoram")
		, m_bank1(*this, "bank1")
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
	virtual void driver_init() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	enum
	{
		TIMER_RESET,
		TIMER_UPDATE_DISPLAY
	};

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

	required_device<i8255_device> m_ppi;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_memory_bank m_bank1;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

class ut88mini_state : public ut88_base_state
{
public:
	ut88mini_state(const machine_config &mconfig, device_type type, const char *tag)
		: ut88_base_state(mconfig, type, tag)
		, m_region_proms(*this, "proms")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void ut88mini(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void io_map(address_map &map);
	void mem_map(address_map &map);

	enum
	{
		TIMER_RESET,
		TIMER_UPDATE_DISPLAY
	};

	uint8_t keyboard_r();
	void led_w(offs_t offset, uint8_t data);

	required_memory_region m_region_proms;

	int m_lcd_digit[6];
	output_finder<6> m_digits;
};

/*----------- defined in video/ut88.c -----------*/

extern const gfx_layout ut88_charlayout;


#endif // MAME_INCLUDES_UT88_H
