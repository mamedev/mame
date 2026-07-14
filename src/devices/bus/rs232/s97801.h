// license:BSD-3-Clause
// copyright-holders:Dave Rand
#ifndef MAME_BUS_RS232_S97801_H
#define MAME_BUS_RS232_S97801_H

#pragma once

#include "rs232.h"
#include "machine/s97801.h"

// Siemens 97801 terminal as an RS-232 peripheral (plugs into any host serial port, e.g. the
// PC-MX2 SERAD port).  Note: the firmware drives the link at a fixed 38400 7O1 XON/XOFF.
class s97801_terminal_device : public device_t, public device_rs232_port_interface
{
public:
	s97801_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void input_txd(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void term_txd_w(int state);

	required_device<s97801_device> m_term;
};

DECLARE_DEVICE_TYPE(SERIAL_TERMINAL_S97801, s97801_terminal_device)

#endif // MAME_BUS_RS232_S97801_H
