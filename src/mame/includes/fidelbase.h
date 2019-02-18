// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Fidelity Electronics chess machines base class
*  main driver is fidelz80.cpp
*
******************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_FIDELBASE_H
#define MAME_INCLUDES_FIDELBASE_H

#include "machine/bankdev.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class fidelbase_state : public driver_device
{
public:
	fidelbase_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_on(*this, "irq_on"),
		m_rombank(*this, "rombank"),
		m_mainmap(*this, "mainmap"),
		m_div_config(*this, "div_config"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_speech(*this, "speech"),
		m_speech_rom(*this, "speech"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<timer_device> m_irq_on;
	optional_memory_bank m_rombank;
	optional_device<address_map_bank_device> m_mainmap;
	optional_ioport m_div_config;
	optional_ioport_array<11> m_inp_matrix; // max 11
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;
	optional_device<s14001a_device> m_speech;
	optional_region_ptr<u8> m_speech_rom;
	optional_device<dac_bit_interface> m_dac;
	optional_device<generic_slot_device> m_cart;

	// misc common
	u16 m_inp_mux;                  // multiplexed keypad/leds mask
	u16 m_led_select;
	u32 m_7seg_data;                // data for seg leds
	u16 m_led_data;
	u8 m_speech_data;
	u8 m_speech_bank;               // speech rom higher address bits

	u16 read_inputs(int columns);

	// cross-compatible cartridges(opening book modules)
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(scc_cartridge);
	virtual DECLARE_READ8_MEMBER(cartridge_r);

	// in case reset button is directly tied to maincpu reset pin
	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// speech rom language, normally 0=English, 1=German, 2=French, 3=Spanish
	template<int Language> void init_language() { m_language = Language; }
	int m_language;

	// periodic interrupts
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(Line, ASSERT_LINE); }
	template<int Line> TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(Line, CLEAR_LINE); }

	// dynamic cpu divider
	void div_trampoline_w(offs_t offset, u8 data);
	u8 div_trampoline_r(offs_t offset);
	void div_set_cpu_freq(offs_t offset);
	void div_trampoline(address_map &map);
	u16 m_div_status;

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


INPUT_PORTS_EXTERN( fidel_cpu_div_2 );
INPUT_PORTS_EXTERN( fidel_cpu_div_4 );

INPUT_PORTS_EXTERN( fidel_cb_buttons );
INPUT_PORTS_EXTERN( fidel_cb_magnets );

#endif // MAME_INCLUDES_FIDELBASE_H
