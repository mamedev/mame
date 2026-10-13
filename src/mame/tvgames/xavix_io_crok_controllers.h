// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H
#define MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H

#pragma once

#include "xavix_io_ir.h"

class xavix_croket_controllers_device : public device_t
{
public:
	xavix_croket_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	template <unsigned Player> u16 controller_report_r();
	int ir_enable_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_motion;
	required_ioport m_guard;
	required_ioport m_controller_power;
};

DECLARE_DEVICE_TYPE(XAVIX_CROKET_CONTROLLERS, xavix_croket_controllers_device)

#endif // MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H
