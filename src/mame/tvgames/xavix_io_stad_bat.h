// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_STAD_BAT_H
#define MAME_TVGAMES_XAVIX_IO_STAD_BAT_H

#pragma once

#include "xavix_io_ir.h"

class xavix_stadium_bat_device : public xavix_ir_msb_device
{
public:
	xavix_stadium_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// The original bat sends an undifferentiated active-low envelope.
	virtual u32 read_report(unsigned report) override { return 0; }
	virtual bool reports_enabled() override;

private:
	required_ioport m_swing;
};

DECLARE_DEVICE_TYPE(XAVIX_STADIUM_BAT, xavix_stadium_bat_device)

#endif // MAME_TVGAMES_XAVIX_IO_STAD_BAT_H
