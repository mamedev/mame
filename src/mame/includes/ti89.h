// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************
 *
 * includes/ti89.h
 *
 ****************************************************************************/

#ifndef TI89_H_
#define TI89_H_

#include "machine/intelfsh.h"

class ti68k_state : public driver_device
{
public:
	ti68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_flash(*this, "flash"),
			m_rom_base(*this, "flash"),
			m_io_bit0(*this, "BIT0"),
			m_io_bit1(*this, "BIT1"),
			m_io_bit2(*this, "BIT2"),
			m_io_bit3(*this, "BIT3"),
			m_io_bit4(*this, "BIT4"),
			m_io_bit5(*this, "BIT5"),
			m_io_bit6(*this, "BIT6"),
			m_io_bit7(*this, "BIT7")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<sharp_unk128mbit_device> m_flash;
	required_region_ptr<UINT16> m_rom_base;
	required_ioport m_io_bit0;
	required_ioport m_io_bit1;
	required_ioport m_io_bit2;
	required_ioport m_io_bit3;
	required_ioport m_io_bit4;
	required_ioport m_io_bit5;
	required_ioport m_io_bit6;
	required_ioport m_io_bit7;

	// hardware versions
	enum { HW1=1, HW2, HW3, HW4 };

	// HW specifications
	UINT8 m_hw_version;
	bool m_flash_mem;
	UINT32 m_initial_pc;

	// keyboard
	UINT16 m_kb_mask;
	UINT8 m_on_key;

	// LCD
	UINT8 m_lcd_on;
	UINT32 m_lcd_base;
	UINT16 m_lcd_width;
	UINT16 m_lcd_height;
	UINT16 m_lcd_contrast;

	// I/O
	UINT16 m_io_hw1[0x10];
	UINT16 m_io_hw2[0x80];

	// Timer
	UINT8 m_timer_on;
	UINT8 m_timer_val;
	UINT16 m_timer_mask;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 keypad_r();
	DECLARE_WRITE16_MEMBER ( ti68k_io_w );
	DECLARE_READ16_MEMBER ( ti68k_io_r );
	DECLARE_WRITE16_MEMBER ( ti68k_io2_w );
	DECLARE_READ16_MEMBER ( ti68k_io2_r );
	DECLARE_WRITE16_MEMBER ( flash_w );
	DECLARE_READ16_MEMBER ( flash_r );
	UINT64 m_timer;
	DECLARE_PALETTE_INIT(ti68k);
	DECLARE_INPUT_CHANGED_MEMBER(ti68k_on_key);
	TIMER_DEVICE_CALLBACK_MEMBER(ti68k_timer_callback);
};

#endif // TI89_H_
