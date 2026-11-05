// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_MADFB_BALL_H
#define MAME_TVGAMES_XAVIX_IO_MADFB_BALL_H

#pragma once

#include "machine/timer.h"

#include <utility>

DECLARE_DEVICE_TYPE(XAVIX_MADFB_BALL, xavix_madfb_ball_device)


class xavix_madfb_ball_device :  public device_t
{
public:
	// construction/destruction
	xavix_madfb_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }

	DECLARE_INPUT_CHANGED_MEMBER(changed);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 PULSE_PERIODS[4] = { 0, 40, 30, 20 };

	TIMER_CALLBACK_MEMBER(event_timer);
	void update_event_rate();

	device_delegate<void ()> m_event_out_cb;
	required_ioport m_in;
	emu_timer *m_event_timer;
};

#endif // MAME_TVGAMES_XAVIX_IO_MADFB_BALL_H
