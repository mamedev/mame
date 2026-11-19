// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETH64 II Ethernet card

    LAN91C96 in 8-bit mode. INTR0 reaches /INT6 through jumper JP1.

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CLOCKPORT_ETH64II_H
#define MAME_BUS_AMIGA_CLOCKPORT_ETH64II_H

#pragma once

#include "clockport.h"
#include "machine/smc91c9x.h"


class amiga_clockport_eth64ii_device : public device_t, public device_amiga_clockport_interface
{
public:
	amiga_clockport_eth64ii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_amiga_clockport_interface implementation
	virtual uint8_t spare_r(offs_t offset) override;
	virtual void spare_w(offs_t offset, uint8_t data) override;

private:
	void irq_w(int state);

	required_device<smc91c96_device> m_smc;
	required_ioport m_jp1;
};

DECLARE_DEVICE_TYPE(AMIGA_CLOCKPORT_ETH64II, amiga_clockport_eth64ii_device)

#endif // MAME_BUS_AMIGA_CLOCKPORT_ETH64II_H
