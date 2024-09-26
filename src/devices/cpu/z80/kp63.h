// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KP63(A) Timer/Counter

***************************************************************************/

#ifndef MAME_CPU_Z80_KP63_H
#define MAME_CPU_Z80_KP63_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class kp63_device : public device_t
{
public:
	// callback configuration
	template <int N> auto outp_callback() { return m_out_pulse_callback[N].bind(); }
	template <int N> auto outs_callback() { return m_out_strobe_callback[N].bind(); }

	// register interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// input line interface
	template <int N> void gate_w(int state) { write_gate(N, state); }

protected:
	// construction/destruction
	kp63_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 num_counters, u8 mode_mask);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const char *const s_count_modes[4];

	// timer callbacks
	template <int N> TIMER_CALLBACK_MEMBER(timer_expired);
	template <int N> TIMER_CALLBACK_MEMBER(strobe_off);
	template <int N> TIMER_CALLBACK_MEMBER(pwm_off);

	// internal helpers
	void timer_pulse(unsigned n);
	void timer_reload(unsigned n);
	void timer_resume_count(unsigned n);
	u16 timer_get_count(unsigned n) const;
	void write_gate(unsigned n, bool state);

	// callback objects
	devcb_write_line::array<4> m_out_pulse_callback;
	devcb_write_line::array<4> m_out_strobe_callback;

	// constant parameters
	const u8 c_num_counters;
	const u8 c_mode_mask;

	// internal timers
	emu_timer *m_timer[4];
	emu_timer *m_strobe_timer[4];
	emu_timer *m_pwm_timer[4];

	// internal state
	u16 m_cr[4];
	u16 m_last_count[4];
	u8 m_count_tmp[4];
	u8 m_status[4];
	u8 m_rw_seq;
	u8 m_timer_started;
	u8 m_gate_input;
};

class kp63_3channel_device : public kp63_device
{
public:
	// device type constructor
	kp63_3channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class kp63a_device : public kp63_device
{
public:
	// device type constructor
	kp63a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(KP63_3CHANNEL, kp63_3channel_device)
DECLARE_DEVICE_TYPE(KP63A, kp63a_device)

#endif // MAME_CPU_Z80_KP63_H
