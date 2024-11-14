// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*
 * s11.h
 *
 *  Created on: 2013-01-01
 */

#ifndef MAME_PINBALL_S11_H
#define MAME_PINBALL_S11_H

#include "cpu/m6800/m6800.h"
#include "pinsnd88.h"
#include "s11c_bg.h"
#include "machine/6821pia.h"
#include "genpin.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "sound/hc55516.h"
#include "sound/ymopm.h"

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
		, m_hc55516(*this, "hc55516")
		, m_cvsd_filter(*this, "cvsd_filter")
		, m_cvsd_filter2(*this, "cvsd_filter2")
		, m_dac(*this, "dac")
		, m_pias(*this, "pias")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia2c(*this, "pia2c")
		, m_pia30(*this, "pia30")
		, m_pia34(*this, "pia34")
		, m_bg(*this, "bg")
		, m_ps88(*this, "ps88")
		, m_digits(*this, "digit%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void s11(machine_config &config);
	void s11_only(machine_config &config);
	void s11_bgs(machine_config &config);
	void s11_bgm(machine_config &config);

	void init_s11();

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void s11_main_map(address_map &map) ATTR_COLD;
	void s11_audio_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer);

	u8 sound_r();
	void bank_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol2_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[8U+i] = BIT(data, i); }; // solenoids 8-15
	void sol3_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[i] = BIT(data, i); }; // solenoids 0-7
	void sound_w(u8 data);

	void pia2c_pa_w(u8 data);
	void pia2c_pb_w(u8 data);
	void pia34_pa_w(u8 data);
	void pia34_pb_w(u8 data);
	void pia34_cb2_w(int state);

	void pias_ca2_w(int state);
	void pias_cb2_w(int state);
	void pia21_ca2_w(int state);
	void pia21_cb2_w(int state) { } // enable solenoids
	void pia24_ca2_w(int state) { m_io_outputs[20] = state; } // E
	void pia24_cb2_w(int state) { m_io_outputs[21] = state; } // F
	void pia28_ca2_w(int state) { } // comma3&4 (not used)
	void pia28_cb2_w(int state) { } // comma1&2 (not used)
	void pia2c_ca2_w(int state) { m_io_outputs[17] = state; } // B
	void pia2c_cb2_w(int state) { m_io_outputs[18] = state; } // C
	void pia30_ca2_w(int state) { m_io_outputs[16] = state; } // D
	void pia30_cb2_w(int state) { m_io_outputs[19] = state; } // A
	void pia_irq(int state);
	void main_irq(int state);

	u8 switch_r();
	void switch_w(u8 data);
	u8 pia28_w7_r();

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_piairq;
	// the following devices are optional because certain board variants (i.e. system 11c)
	//  do not have the audio section on the mainboard populated
	optional_device<m6802_cpu_device> m_audiocpu;
	optional_device<input_merger_device> m_audioirq;
	optional_device<hc55516_device> m_hc55516;
	optional_device<filter_biquad_device> m_cvsd_filter;
	optional_device<filter_biquad_device> m_cvsd_filter2;
	optional_device<mc1408_device> m_dac;
	optional_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;
	optional_device<s11c_bg_device> m_bg;
	optional_device<pinsnd88_device> m_ps88;
	output_finder<63> m_digits;
	required_ioport_array<8> m_io_keyboard;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps

	// getters/setters
	u8 get_strobe() { return m_strobe; }
	void set_strobe(u8 s) { m_strobe = s; }
	u8 get_diag() { return m_diag; }
	void set_diag(u8 d) { m_diag = d; }
	u32 get_segment1() { return m_segment1; }
	void set_segment1(u32 s) { m_segment1 = s; }
	u32 get_segment2() { return m_segment2; }
	void set_segment2(u32 s) { m_segment2 = s; }

	u8 m_sound_data = 0U;
	u8 m_strobe = 0U;
	u8 m_row = 0U;
	u8 m_diag = 0U;
	u8 m_lamp_data = 0U;
	u32 m_segment1 = 0U;
	u32 m_segment2 = 0U;
	u32 m_timer_count = 0U;
	emu_timer* m_irq_timer = nullptr;
	bool m_timer_irq_active = false;
	bool m_pia_irq_active = false;
	u8 m_lock1 = 0U;
	u8 m_lock2 = 0U;
	void set_lock1(u8 x) { m_lock1 = x; }
	u8 get_lock1() { return m_lock1; }
	void set_lock2(u8 x) { m_lock2 = x; }
	u8 get_lock2() { return m_lock2; }

private:
	void dig0_w(u8 data);
};


class s11a_state : public s11_state
{
public:
	s11a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11_state(mconfig, type, tag)
	{ }

	void s11a_base(machine_config &config);
	void s11a(machine_config &config);
	void s11a_obg(machine_config &config);

	void init_s11a();

protected:
	void s11a_dig0_w(u8 data);
};


class s11b_state : public s11a_state
{
public:
	s11b_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11a_state(mconfig, type, tag)
	{ }

	void s11b_base(machine_config &config);
	void s11b(machine_config &config);
	void s11b_jokerz(machine_config &config);

	void init_s11bnn();  // normal
	void init_s11bin();  // invert
	void init_s11bn7();  // 7seg34
	void init_s11bi7();  // invert and 7seg34

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	bool m_invert = false;  // later System 11B games expect inverted data to the display LED segments.
	void set_invert(bool i) { m_invert = i; }
	bool m_is7seg34 = false;  // some games use 7-segment displays for players 3 and 4
	void set_7seg(bool i) { m_is7seg34 = i; }

	void s11b_dig1_w(u8 data);
	void s11b_pia2c_pa_w(u8 data);
	void s11b_pia2c_pb_w(u8 data);
	void s11b_pia34_pa_w(u8 data);
};


class s11c_state : public s11b_state
{
public:
	s11c_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11b_state(mconfig, type, tag)
	{ }

	void s11c(machine_config &config);

	void init_s11c();
	void init_s11c7();

protected:
	virtual void machine_reset() override ATTR_COLD;
};


#endif // MAME_PINBALL_S11_H
