// license:BSD-3-Clause
// copyright-holders:smf, hap
/*

  Generic clock signal device

*/

#ifndef MAME_MACHINE_CLOCK_H
#define MAME_MACHINE_CLOCK_H

#pragma once

class clock_device : public device_t
{
public:
	clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto signal_handler() { return m_signal_handler.bind(); }
	auto &set_period(attotime period) { m_period = period; reinit(); return *this; }
	auto &set_pulse_width(attotime pw) { assert(!pw.is_never()); m_pw = pw; reinit(); return *this; }
	auto &set_duty_cycle(double duty) { assert(duty >= 0.0 && duty <= 1.0); m_duty = duty; m_pw = attotime::never; reinit(); return *this; }

	DECLARE_READ_LINE_MEMBER(signal_r) { return m_signal; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override { output(); }
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_clock_changed() override { reinit(); }

private:
	static constexpr device_timer_id TID_CLOCK_INIT = 0;
	static constexpr device_timer_id TID_CLOCK_TICK = 1;

	void reinit();
	void output();

	int m_signal;
	int m_output;
	double m_duty;
	attotime m_period;
	attotime m_pw;
	attotime m_thigh;
	attotime m_tlow;

	emu_timer *m_timer_init;
	emu_timer *m_timer_tick;

	devcb_write_line m_signal_handler;
};

DECLARE_DEVICE_TYPE(CLOCK, clock_device)

#endif // MAME_MACHINE_CLOCK_H
