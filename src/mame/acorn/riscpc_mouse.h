// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_ACORN_RISCPC_MOUSE_H
#define MAME_ACORN_RISCPC_MOUSE_H

#pragma once

#include "machine/quadmouse.h"

class riscpc_mouse_device
	: public quadmouse_device
{
public:
	riscpc_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	ioport_value buttons_r();

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_buttons;
};

DECLARE_DEVICE_TYPE(RISCPC_MOUSE, riscpc_mouse_device)

#endif // MAME_ACORN_RISCPC_MOUSE_H
