// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*
 * s11.h
 *
 *  Created on: 1/01/2013
 */

#ifndef MAME_INCLUDES_S11_H
#define MAME_INCLUDES_S11_H

#include "cpu/m6800/m6800.h"
#include "audio/s11c_bg.h"
#include "machine/6821pia.h"
#include "machine/genpin.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "sound/ym2151.h"

// 6802/8 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL(4'000'000)/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// IRQ pulse width is always 32 cycles
// All machines I've looked at so far have W14 present and W15 absent
// which makes the timer int fire every 0x380 E-clocks (1MHz/0x380, ~1.116KHz)
// It is possible to have W15 present and W14 absent instead,
// which makes the timer fire every 0x700 E-clocks (1MHz/0x700, ~558Hz)
// but I am unaware of any games which make use of this feature.
// define the define below to enable the W15-instead-of-W14 feature.
#undef S11_W15

class s11_state : public genpin_class
{
public:
	s11_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_piairq(*this, "piairq")
		, m_audiocpu(*this, "audiocpu")
		, m_audioirq(*this, "audioirq")
		, m_bgcpu(*this, "bgcpu")
		, m_hc55516(*this, "hc55516")
		, m_pias(*this, "pias")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia2c(*this, "pia2c")
		, m_pia30(*this, "pia30")
		, m_pia34(*this, "pia34")
		, m_pia40(*this, "pia40")
		, m_ym2151(*this, "ym2151")
		, m_bg(*this, "bgm")
		, m_digits(*this, "digit%u", 0U)
		, m_swarray(*this, "SW.%u", 0U)
		{ }

	void s11(machine_config &config);

	void init_s11();

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

	uint8_t sound_r();
	void bank_w(uint8_t data);
	void dig0_w(uint8_t data);
	void dig1_w(uint8_t data);
	void lamp0_w(uint8_t data);
	void lamp1_w(uint8_t data) { };
	void sol2_w(uint8_t data) { }; // solenoids 8-15
	void sol3_w(uint8_t data); // solenoids 0-7
	void sound_w(uint8_t data);

	void pia2c_pa_w(uint8_t data);
	void pia2c_pb_w(uint8_t data);
	void pia34_pa_w(uint8_t data);
	void pia34_pb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia34_cb2_w);
	void pia40_pb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);

	DECLARE_WRITE_LINE_MEMBER(pias_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pias_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_WRITE_LINE_MEMBER(main_irq);

	uint8_t switch_r();
	void switch_w(uint8_t data);
	uint8_t pia28_w7_r();

protected:
	DECLARE_MACHINE_RESET(s11);
	void s11_audio_map(address_map &map);
	void s11_bg_map(address_map &map);
	void s11_main_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_piairq;
	optional_device<m6802_cpu_device> m_audiocpu;
	optional_device<input_merger_device> m_audioirq;
	optional_device<cpu_device> m_bgcpu;
	optional_device<hc55516_device> m_hc55516;
	optional_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;
	optional_device<pia6821_device> m_pia40;
	optional_device<ym2151_device> m_ym2151;
	optional_device<s11c_bg_device> m_bg;
	output_finder<63> m_digits;
	required_ioport_array<8> m_swarray;

	// getters/setters
	uint8_t get_strobe() { return m_strobe; }
	void set_strobe(uint8_t s) { m_strobe = s; }
	uint8_t get_diag() { return m_diag; }
	void set_diag(uint8_t d) { m_diag = d; }
	uint32_t get_segment1() { return m_segment1; }
	void set_segment1(uint32_t s) { m_segment1 = s; }
	uint32_t get_segment2() { return m_segment2; }
	void set_segment2(uint32_t s) { m_segment2 = s; }
	void set_timer(emu_timer* t) { m_irq_timer = t; }

	static const device_timer_id TIMER_IRQ = 0;

private:
	virtual void machine_start() override { m_digits.resolve(); }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint8_t m_sound_data;
	uint8_t m_strobe;
	uint8_t m_switch_col;
	uint8_t m_diag;
	uint32_t m_segment1;
	uint32_t m_segment2;
	uint32_t m_timer_count;
	emu_timer* m_irq_timer;
	bool m_timer_irq_active;
	bool m_pia_irq_active;
};

#endif // MAME_INCLUDES_S11_H
