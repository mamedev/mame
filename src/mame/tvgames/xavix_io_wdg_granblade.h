// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_WDG_GRANBLADE_H
#define MAME_TVGAMES_XAVIX_IO_WDG_GRANBLADE_H

#pragma once

#include "xavix_io_ir.h"

class xavix_granblade_device : public xavix_ir_pulse_distance_device
{
public:
	xavix_granblade_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool report_enabled(unsigned report) override;

private:
	static constexpr u8 GRANBLADE_REPORTS[2][2] =
	{
		{ 0x13, 0x12 }, // Blade: transform, swing
		{ 0x14, 0x11 }  // Gran Magnum: transform, trigger
	};

	required_ioport m_granblade_action;
	required_ioport m_granblade_mode;
	required_ioport m_granblade_power;
	u8 m_granblade_last_mode;
};

DECLARE_DEVICE_TYPE(XAVIX_GRANBLADE, xavix_granblade_device)

#endif // MAME_TVGAMES_XAVIX_IO_WDG_GRANBLADE_H
