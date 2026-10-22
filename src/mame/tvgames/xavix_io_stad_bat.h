// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_STAD_BAT_H
#define MAME_TVGAMES_XAVIX_IO_STAD_BAT_H

#pragma once

#include "xavix_io_ir.h"

#include <utility>

class xavix_stadium_bat_device : public device_t
{
public:
	xavix_stadium_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_event_callback(T &&... args) { m_ir.lookup()->set_event_callback(std::forward<T>(args)...); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	int bat_active_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_swing;
};

DECLARE_DEVICE_TYPE(XAVIX_STADIUM_BAT, xavix_stadium_bat_device)

#endif // MAME_TVGAMES_XAVIX_IO_STAD_BAT_H
