// license:BSD-3-Clause
// copyright-holders:68bit

#ifndef MAME_BUS_RS232_EXORTERM_H
#define MAME_BUS_RS232_EXORTERM_H

#pragma once

#include "rs232.h"
#include "machine/exorterm.h"


class exorterm155_terminal_device : public device_t, public device_rs232_port_interface
{
public:
	exorterm155_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER(flow_control);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<exorterm155_device> m_exorterm155;
	required_ioport m_flow_control;

	void route_term_rts(int state);
	void route_term_dtr(int state);

	int m_dtr;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL_EXORTERM155, exorterm155_terminal_device)

#endif // MAME_BUS_RS232_EXORTERM_H
