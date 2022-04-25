// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************
 *
 * includes/ti89.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_TI89_H
#define MAME_INCLUDES_TI89_H

#pragma once

#include "machine/intelfsh.h"
#include "machine/timer.h"
#include "emupal.h"

class ti68k_state : public driver_device
{
public:
	ti68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_flash(*this, "flash")
		, m_rom_base(*this, "flash")
		, m_ram_base(*this, "nvram")
		, m_io_bit0(*this, "BIT0")
		, m_io_bit1(*this, "BIT1")
		, m_io_bit2(*this, "BIT2")
		, m_io_bit3(*this, "BIT3")
		, m_io_bit4(*this, "BIT4")
		, m_io_bit5(*this, "BIT5")
		, m_io_bit6(*this, "BIT6")
		, m_io_bit7(*this, "BIT7")
	{ }

	void v200(machine_config &config);
	void ti92(machine_config &config);
	void ti89(machine_config &config);
	void ti92p(machine_config &config);
	void ti89t(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(ti68k_on_key);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// hardware versions
	enum { HW1=1, HW2, HW3, HW4 };

	required_device<cpu_device> m_maincpu;
	optional_device<intelfsh16_device> m_flash;
	required_region_ptr<uint16_t> m_rom_base;
	required_shared_ptr<uint16_t> m_ram_base;
	required_ioport m_io_bit0;
	required_ioport m_io_bit1;
	required_ioport m_io_bit2;
	required_ioport m_io_bit3;
	required_ioport m_io_bit4;
	required_ioport m_io_bit5;
	required_ioport m_io_bit6;
	required_ioport m_io_bit7;

	// HW specifications
	uint8_t m_hw_version = 0;
	bool m_ram_enabled = false;

	// keyboard
	uint16_t m_kb_mask = 0;
	uint8_t m_on_key = 0;

	// LCD
	uint8_t m_lcd_on = 0;
	uint32_t m_lcd_base = 0;
	uint16_t m_lcd_width = 0;
	uint16_t m_lcd_height = 0;
	uint16_t m_lcd_contrast = 0;

	// I/O
	uint16_t m_io_hw1[0x10]{};
	uint16_t m_io_hw2[0x80]{};

	// Timer
	uint8_t m_timer_on = 0;
	uint8_t m_timer_val = 0;
	uint16_t m_timer_mask = 0;
	uint64_t m_timer = 0;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t keypad_r();
	void ti68k_io_w(offs_t offset, uint16_t data);
	uint16_t ti68k_io_r(offs_t offset);
	void ti68k_io2_w(offs_t offset, uint16_t data);
	uint16_t ti68k_io2_r(offs_t offset);
	uint16_t rom_r(offs_t offset);
	uint16_t reset_overlay_r(offs_t offset);
	void ti68k_palette(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(ti68k_timer_callback);

	void ti89_mem(address_map &map);
	void ti89t_mem(address_map &map);
	void ti92_mem(address_map &map);
	void ti92p_mem(address_map &map);
	void v200_mem(address_map &map);
};

#endif // MAME_INCLUDES_TI89_H
