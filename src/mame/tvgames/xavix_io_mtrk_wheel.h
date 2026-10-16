// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_MTRK_WHEEL_H
#define MAME_TVGAMES_XAVIX_IO_MTRK_WHEEL_H

#pragma once

#include "machine/timer.h"

DECLARE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device)


class xavix_mtrk_wheel_device :  public device_t
{
public:
	// construction/destruction
	xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto event_callback() { return m_event_out_cb.bind(); }

	int direction_r();
	DECLARE_INPUT_CHANGED_MEMBER(changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(event_timer);
	void update_event_rate();

	devcb_write_line m_event_out_cb;
	required_ioport m_in;
	emu_timer *m_event_timer;
	u8 m_direction;
	u8 m_rate;
};

#endif // MAME_TVGAMES_XAVIX_IO_MTRK_WHEEL_H
