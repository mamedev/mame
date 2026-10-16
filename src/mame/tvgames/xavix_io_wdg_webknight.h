// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H
#define MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H

#pragma once

#include "xavix_io_ir.h"

class xavix_webknight_device : public device_t
{
public:
	xavix_webknight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto event_callback() { return m_ir.lookup()->event_callback(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	u16 report_r();
	int report_enabled_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_identity;
	required_ioport m_mark_switch;
};

DECLARE_DEVICE_TYPE(XAVIX_WEBKNIGHT, xavix_webknight_device)

#endif // MAME_TVGAMES_XAVIX_IO_WDG_WEBKNIGHT_H
