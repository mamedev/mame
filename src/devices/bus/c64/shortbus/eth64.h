// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETH64 ShortBus card

    LAN91C96 in 8-bit mode, software compatible with ETH64 II. JP1 selects
    /CSEL0 or /CSEL1 and JP2 routes the interrupt output to /NMI.

***************************************************************************/

#ifndef MAME_BUS_C64_SHORTBUS_ETH64_H
#define MAME_BUS_C64_SHORTBUS_ETH64_H

#pragma once

#include "shortbus.h"
#include "machine/smc91c9x.h"


class ide64_shortbus_eth64_device : public device_t, public device_ide64_shortbus_interface
{
public:
	ide64_shortbus_eth64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ide64_shortbus_interface implementation
	virtual uint8_t shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel) override;
	virtual void shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel) override;

private:
	void irq_w(int state);

	required_device<smc91c96_device> m_smc;
	required_ioport m_jp1;
	required_ioport m_jp2;
};

DECLARE_DEVICE_TYPE(IDE64_SHORTBUS_ETH64, ide64_shortbus_eth64_device)

#endif // MAME_BUS_C64_SHORTBUS_ETH64_H
