// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*
 * s11.h
 *
 *  Created on: 1/01/2013
 */

#ifndef MAME_INCLUDES_S11_H
#define MAME_INCLUDES_S11_H

#include "audio/s11c_bg.h"
#include "machine/6821pia.h"
#include "machine/genpin.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "sound/ym2151.h"

// 6802/8 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL(4'000'000)/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

class s11_state : public genpin_class
{
public:
	s11_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
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
		, m_ym(*this, "ym2151")
		, m_bg(*this, "bgm")
		, m_digits(*this, "digit%u", 0U)
		{ }

	void s11(machine_config &config);

	void init_s11();

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w); // solenoids 0-7
	DECLARE_WRITE8_MEMBER(sound_w);

	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE8_MEMBER(pia34_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia34_cb2_w);
	DECLARE_WRITE8_MEMBER(pia40_pb_w);
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

	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ8_MEMBER(pia28_w7_r);

protected:
	DECLARE_MACHINE_RESET(s11);
	void s11_audio_map(address_map &map);
	void s11_bg_map(address_map &map);
	void s11_main_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
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
	optional_device<ym2151_device> m_ym;
	optional_device<s11c_bg_device> m_bg;
	output_finder<63> m_digits;

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
	uint8_t m_kbdrow;
	uint8_t m_diag;
	uint32_t m_segment1;
	uint32_t m_segment2;
	emu_timer* m_irq_timer;
	bool m_irq_active;
};

#endif // MAME_INCLUDES_S11_H
