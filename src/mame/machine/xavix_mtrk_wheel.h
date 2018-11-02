// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_XAVIX_MTRK_WHEEL_H
#define MAME_MACHINE_XAVIX_MTRK_WHEEL_H

#pragma once

DECLARE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device)


class xavix_mtrk_wheel_device :  public device_t
{
public:
	// construction/destruction
	xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto event_out_cb() { return m_event_out_cb.bind(); }

	int read_direction();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_event_out_cb;
};

#endif // MAME_MACHINE_XAVIX_MTRK_WHEEL_H
