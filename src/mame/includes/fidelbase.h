// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Fidelity Electronics chess computers base driver
*  implementation is in machine/fidelbase.cpp
*
******************************************************************************/

#ifndef MAME_INCLUDES_FIDELBASE_H
#define MAME_INCLUDES_FIDELBASE_H

#pragma once

#include "includes/chessbase.h"

#include "machine/bankdev.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class fidelbase_state : public chessbase_state
{
public:
	fidelbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_rombank(*this, "rombank"),
		m_mainmap(*this, "mainmap"),
		m_div_config(*this, "div_config"),
		m_speech(*this, "speech"),
		m_speech_rom(*this, "speech"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot")
	{ }

	// in case reset button is directly tied to maincpu reset pin
	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// speech rom language, normally 0=English, 1=German, 2=French, 3=Spanish
	template<int Language> void init_language() { m_language = Language; }

protected:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<timer_device> m_irq_on;
	optional_memory_bank m_rombank;
	optional_device<address_map_bank_device> m_mainmap;
	optional_ioport m_div_config;
	optional_device<s14001a_device> m_speech;
	optional_region_ptr<u8> m_speech_rom;
	optional_device<dac_bit_interface> m_dac;
	optional_device<generic_slot_device> m_cart;

	int m_language;

	u8 m_speech_data;
	u8 m_speech_bank; // speech rom higher address bits

	// cross-compatible cartridges(opening book modules)
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(scc_cartridge);
	virtual DECLARE_READ8_MEMBER(cartridge_r);

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// dynamic cpu divider
	void div_trampoline_w(offs_t offset, u8 data);
	u8 div_trampoline_r(offs_t offset);
	void div_set_cpu_freq(offs_t offset);
	void div_trampoline(address_map &map);
	u16 m_div_status;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


INPUT_PORTS_EXTERN( fidel_cpu_div_2 );
INPUT_PORTS_EXTERN( fidel_cpu_div_4 );

#endif // MAME_INCLUDES_FIDELBASE_H
