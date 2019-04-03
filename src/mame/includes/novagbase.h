// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Novag chess computers base driver
*  implementation is in machine/novagbase.cpp
*
******************************************************************************/

#ifndef MAME_INCLUDES_NOVAGBASE_H
#define MAME_INCLUDES_NOVAGBASE_H

#pragma once

#include "includes/chessbase.h"

#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/beep.h"
#include "video/hd44780.h"
#include "emupal.h"

class novagbase_state : public chessbase_state
{
public:
	novagbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_rombank(*this, "rombank"),
		m_beeper(*this, "beeper"),
		m_dac(*this, "dac"),
		m_lcd(*this, "hd44780")
	{ }

	// in case reset button is directly tied to maincpu reset pin
	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<timer_device> m_irq_on;
	optional_memory_bank m_rombank;
	optional_device<beep_device> m_beeper;
	optional_device<dac_bit_interface> m_dac;
	optional_device<hd44780_device> m_lcd;

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// lcd common
	HD44780_PIXEL_UPDATE(novag_lcd_pixel_update);
	void novag_lcd_palette(palette_device &palette) const;
	u8 m_lcd_control;
	u8 m_lcd_data;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


#endif // MAME_INCLUDES_NOVAGBASE_H
