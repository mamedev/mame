// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_IR_MOTION_H
#define MAME_TVGAMES_XAVIX_IO_IR_MOTION_H

#pragma once

#include "xavix_io_ir.h"

#include <utility>

class xavix_ir_motion_device : public device_t
{
public:
	enum class controller_type
	{
		PADDLES,
		PING_PONG_RACKETS,
		POWERED_PING_PONG_RACKETS,
		KICK_SENSORS,
		TENNIS_RACKETS,
		SHAKERS,
		SWORDS,
		XAVIX_RACKETS
	};

	xavix_ir_motion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_controller_type(controller_type type) { m_controller_type = type; }
	template <typename... T> void set_event_callback(T &&... args) { m_ir.lookup()->set_event_callback(std::forward<T>(args)...); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	template <unsigned Player> u16 ir_report_r();
	template <unsigned Player> int ir_report_enabled_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport_array<2> m_ir_strength;
	required_ioport_array<2> m_ir_button;
	optional_ioport m_ir_power;
	controller_type m_controller_type;
};

DECLARE_DEVICE_TYPE(XAVIX_IR_MOTION, xavix_ir_motion_device)

#endif // MAME_TVGAMES_XAVIX_IO_IR_MOTION_H
