// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_MADFB_BALL_H
#define MAME_TVGAMES_XAVIX_MADFB_BALL_H

#pragma once

#include "machine/timer.h"

DECLARE_DEVICE_TYPE(XAVIX_MADFB_BALL, xavix_madfb_ball_device)


class xavix_madfb_ball_device :  public device_t
{
public:
	// construction/destruction
	xavix_madfb_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto event_out_cb() { return m_event_out_cb.bind(); }

	int read_direction();
	DECLARE_INPUT_CHANGED_MEMBER( changed );

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	devcb_write_line m_event_out_cb;
	required_ioport m_in;
	TIMER_CALLBACK_MEMBER(event_timer);
	emu_timer *m_event_timer;
	void check_ball();
	int m_is_running;
};

#endif // MAME_TVGAMES_XAVIX_MADFB_BALL_H
