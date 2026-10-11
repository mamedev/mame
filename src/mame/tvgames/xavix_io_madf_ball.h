// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_MADF_BALL_H
#define MAME_TVGAMES_XAVIX_IO_MADF_BALL_H

#pragma once

#include "machine/timer.h"

DECLARE_DEVICE_TYPE(XAVIX_MADFB_BALL, xavix_madfb_ball_device)


class xavix_madfb_ball_device :  public device_t
{
public:
	// construction/destruction
	xavix_madfb_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_event_out_cb.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 PULSE_PERIODS[4] = { 0, 40, 30, 20 };

	TIMER_CALLBACK_MEMBER(event_timer);
	void update_event_rate();

	devcb_write_line m_event_out_cb;
	required_ioport m_in;
	emu_timer *m_event_timer;
};

#endif // MAME_TVGAMES_XAVIX_IO_MADF_BALL_H
