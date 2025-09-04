// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_IBM_RTPC_MOUSE_H
#define MAME_IBM_RTPC_MOUSE_H

#pragma once

#include "bus/rs232/rs232.h"

class rtpc_mouse_device : public buffered_rs232_device<4>
{
public:
	rtpc_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// buffered_rs232_device
	virtual void received_byte(u8 data) override;

	// helpers
	void set_sample_rate(u8 data);
	void set_data_mode(u8 data);
	void set_resolution(u8 data);

	void status_report();
	void data_report(s32 param);
	void pon_report(bool error);

private:
	required_ioport m_buttons;
	required_ioport m_x_axis;
	required_ioport m_y_axis;

	emu_timer *m_timer;

	// programmable state
	bool m_linear;
	bool m_disable;
	bool m_remote;
	bool m_wrap;

	u8 m_resolution;
	u8 m_sample_rate;

	// internal state
	u8 m_b;
	u16 m_x;
	u16 m_y;

	// command parameter handler
	void (rtpc_mouse_device::*m_cmd_param)(u8);
};

DECLARE_DEVICE_TYPE(RTPC_MOUSE, rtpc_mouse_device)

#endif // MAME_IBM_RTPC_MOUSE_H
