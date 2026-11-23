// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BB_BAT_H
#define MAME_TVGAMES_XAVIX_IO_BB_BAT_H

#pragma once

#include "xavix_io_ir.h"

class xavix_baseball_bat_device : public xavix_optical_pulse_device_base
{
public:
	xavix_baseball_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void vblank_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static constexpr u8 EVENT_COUNTS[4] = { 0, 2, 4, 8 };

	required_ioport m_strength;
};

DECLARE_DEVICE_TYPE(XAVIX_BASEBALL_BAT, xavix_baseball_bat_device)

#endif // MAME_TVGAMES_XAVIX_IO_BB_BAT_H
