// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_BUS_RS232_IE15_H
#define MAME_BUS_RS232_IE15_H

#pragma once

#include "rs232.h"
#include "machine/ie15.h"


class ie15_terminal_device : public device_t, public device_rs232_port_interface
{
public:
	ie15_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<ie15_device> m_ie15;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL_IE15, ie15_terminal_device)

#endif // MAME_BUS_RS232_IE15_H
