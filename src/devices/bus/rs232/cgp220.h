// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack Color Graphics Printer CGP-220, serial interface

**********************************************************************/

#ifndef MAME_BUS_RS232_CGP220_H
#define MAME_BUS_RS232_CGP220_H

#pragma once

#include "rs232.h"
#include "machine/cgp220.h"


class rs232_cgp220_device : public device_t, public device_rs232_port_interface
{
public:
	rs232_cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void input_txd(int state) override { m_printer->rxd_w(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void busy_w(int state);

	required_device<cgp220_device> m_printer;
};

DECLARE_DEVICE_TYPE(RS232_CGP220, rs232_cgp220_device)

#endif // MAME_BUS_RS232_CGP220_H
