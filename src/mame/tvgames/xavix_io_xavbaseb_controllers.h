// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_XAVBASEB_CONTROLLERS_H
#define MAME_TVGAMES_XAVIX_IO_XAVBASEB_CONTROLLERS_H

#pragma once

#include "xavix_io_ir.h"

class xavix_baseball_controllers_device : public xavix_ir_framed_device
{
public:
	enum class controller_type
	{
		BASEBALL,
		STADIUM_DX
	};

	xavix_baseball_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_controller_type(controller_type type) { m_controller_type = type; }

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool report_enabled(unsigned report) override;

private:
	static constexpr u8 BAT_PAYLOADS[3] = { 0x43, 0x4a, 0x51 };
	static u16 baseball_bat_report(bool bunt, bool swing);

	required_ioport m_ball_throw;
	required_ioport m_ball_buttons;
	required_ioport m_bat;
	optional_ioport m_controller_power;
	controller_type m_controller_type;
};

DECLARE_DEVICE_TYPE(XAVIX_BASEBALL_CONTROLLERS, xavix_baseball_controllers_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVBASEB_CONTROLLERS_H
