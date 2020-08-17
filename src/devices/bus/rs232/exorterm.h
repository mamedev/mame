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

	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) override;

	DECLARE_INPUT_CHANGED_MEMBER(flow_control);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<exorterm155_device> m_exorterm155;
	required_ioport m_flow_control;

	DECLARE_WRITE_LINE_MEMBER(route_term_rts);
	DECLARE_WRITE_LINE_MEMBER(route_term_dtr);

	int m_dtr;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL_EXORTERM155, exorterm155_terminal_device)

#endif // MAME_BUS_RS232_EXORTERM_H
