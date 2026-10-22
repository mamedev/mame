// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H
#define MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H

#pragma once

#include "xavix_io_ir.h"

class xavix_webknight_device : public xavix_ir_pulse_distance_device
{
public:
	xavix_webknight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool report_enabled(unsigned report) override;

private:
	required_ioport m_identity;
	required_ioport m_mark_switch;
};

DECLARE_DEVICE_TYPE(XAVIX_WEBKNIGHT, xavix_webknight_device)

#endif // MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H
