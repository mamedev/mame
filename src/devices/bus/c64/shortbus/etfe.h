// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETFE ShortBus card

    CS8900A in 8-bit mode with the TFE register layout and no interrupt.
    JP7 selects $DExx (/IO1) or $DFxx (/IO2). In the $DExx space JP5
    selects /CSEL0 or /CSEL1, in the $DFxx space JP8 selects the offset.

***************************************************************************/

#ifndef MAME_BUS_C64_SHORTBUS_ETFE_H
#define MAME_BUS_C64_SHORTBUS_ETFE_H

#pragma once

#include "shortbus.h"
#include "machine/cs8900a.h"


class ide64_shortbus_etfe_device : public device_t, public device_ide64_shortbus_interface
{
public:
	ide64_shortbus_etfe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ide64_shortbus_interface implementation
	virtual uint8_t shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel) override;
	virtual void shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel) override;

private:
	bool selected(offs_t offset, int io1, int io2, int csel);

	required_device<cs8900a_device> m_cs8900a;
	required_ioport m_base;
};

DECLARE_DEVICE_TYPE(IDE64_SHORTBUS_ETFE, ide64_shortbus_etfe_device)

#endif // MAME_BUS_C64_SHORTBUS_ETFE_H
