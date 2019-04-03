// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_CLOCK_H
#define MAME_MACHINE_CLOCK_H

#pragma once


class clock_device : public device_t
{
public:
	auto signal_handler() { return m_signal_handler.bind(); }
	DECLARE_READ_LINE_MEMBER(signal_r) { return m_signal; }

	clock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_clock_changed() override;

private:
	int m_signal;
	emu_timer *m_timer;

	devcb_write_line m_signal_handler;
};

DECLARE_DEVICE_TYPE(CLOCK, clock_device)

#endif // MAME_MACHINE_CLOCK_H
