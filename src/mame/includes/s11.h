// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11.h
 *
 *  Created on: 1/01/2013
 */

#ifndef S11_H_
#define S11_H_

// 6802/8 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL_4MHz/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

class s11_state : public genpin_class
{
public:
	s11_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_bgcpu(*this, "bgcpu"),
	m_dac(*this, "dac"),
	m_dac1(*this, "dac1"),
	m_hc55516(*this, "hc55516"),
	m_pias(*this, "pias"),
	m_pia21(*this, "pia21"),
	m_pia24(*this, "pia24"),
	m_pia28(*this, "pia28"),
	m_pia2c(*this, "pia2c"),
	m_pia30(*this, "pia30"),
	m_pia34(*this, "pia34"),
	m_pia40(*this, "pia40"),
	m_ym(*this, "ym2151"),
	m_bg(*this, "bgm")
	{ }

	DECLARE_READ8_MEMBER(dac_r);
	DECLARE_WRITE8_MEMBER(dac_w);
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
	DECLARE_WRITE8_MEMBER(pia40_pa_w);
	DECLARE_WRITE8_MEMBER(pia40_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ8_MEMBER(pia28_w7_r);
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
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);
	DECLARE_MACHINE_RESET(s11);
	DECLARE_DRIVER_INIT(s11);
protected:
	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_bgcpu;
	optional_device<dac_device> m_dac;
	optional_device<dac_device> m_dac1;
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

	// getters/setters
	UINT8 get_strobe() { return m_strobe; }
	void set_strobe(UINT8 s) { m_strobe = s; }
	UINT8 get_diag() { return m_diag; }
	void set_diag(UINT8 d) { m_diag = d; }
	UINT32 get_segment1() { return m_segment1; }
	void set_segment1(UINT32 s) { m_segment1 = s; }
	UINT32 get_segment2() { return m_segment2; }
	void set_segment2(UINT32 s) { m_segment2 = s; }
	void set_timer(emu_timer* t) { m_irq_timer = t; }

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 0;
private:
	UINT8 m_sound_data;
	UINT8 m_strobe;
	UINT8 m_kbdrow;
	UINT8 m_diag;
	UINT32 m_segment1;
	UINT32 m_segment2;
	emu_timer* m_irq_timer;
	bool m_irq_active;
};

class s11a_state : public s11_state
{
public:
	s11a_state(const machine_config &mconfig, device_type type, std::string tag)
		: s11_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(bgbank_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_MACHINE_RESET(s11a);
	DECLARE_DRIVER_INIT(s11a);

protected:

private:

};

class s11b_state : public s11a_state
{
public:
	s11b_state(const machine_config &mconfig, device_type type, std::string tag)
		: s11a_state(mconfig, type, tag),
		m_bg_hc55516(*this, "hc55516_bg")

	{ }

	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_ca2_w);

	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);

	DECLARE_MACHINE_RESET(s11b);
	DECLARE_DRIVER_INIT(s11b);
	DECLARE_DRIVER_INIT(s11b_invert);

protected:
	optional_device<hc55516_device> m_bg_hc55516;

	void set_invert(bool inv) { m_invert = inv; }

private:
	bool m_invert;  // later System 11B games start expecting inverted data to the display LED segments.


};

class s11c_state : public s11b_state
{
public:
	s11c_state(const machine_config &mconfig, device_type type, std::string tag)
		: s11b_state(mconfig, type, tag)
	{ }

	DECLARE_MACHINE_RESET(s11c);
	DECLARE_DRIVER_INIT(s11c);

protected:

private:


};


#endif /* S11_H_ */
