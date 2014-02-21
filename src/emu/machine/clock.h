#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma once

#include "emu.h"

#define MCFG_CLOCK_SIGNAL_HANDLER(_devcb) \
	devcb = &clock_device::set_signal_handler(*device, DEVCB2_##_devcb);

class clock_device : public device_t
{
public:
	clock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_signal_handler(device_t &device, _Object object) { return downcast<clock_device &>(device).m_signal_handler.set_callback(object); }

protected:
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_clock_changed();

private:
	void update_timer();
	attotime period();

	int m_signal;
	emu_timer *m_timer;

	devcb2_write_line m_signal_handler;
};

extern const device_type CLOCK;

#endif
