// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H
#define MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H

#pragma once

#include "xavix_io_ir_motion.h"

class xavix_croket_controllers_device : public xavix_ir_motion_base_device
{
public:
	xavix_croket_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;

private:
	required_ioport m_motion;
	required_ioport_array<2> m_motion_strength;
	required_ioport m_guard;
	required_ioport m_controller_power;
};

DECLARE_DEVICE_TYPE(XAVIX_CROKET_CONTROLLERS, xavix_croket_controllers_device)

#endif // MAME_TVGAMES_XAVIX_IO_CROK_CONTROLLERS_H
