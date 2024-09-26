// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_MTRK_WHEEL_H
#define MAME_TVGAMES_XAVIX_MTRK_WHEEL_H

#pragma once

#include "machine/timer.h"

DECLARE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device)


class xavix_mtrk_wheel_device :  public device_t
{
public:
	// construction/destruction
	xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	int m_direction;
	void check_wheel();
	int m_is_running;
};

#endif // MAME_TVGAMES_XAVIX_MTRK_WHEEL_H
