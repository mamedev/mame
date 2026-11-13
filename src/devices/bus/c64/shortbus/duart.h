// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    DUART64 ShortBus card

    XR68C681 at 3.6864 MHz, A0-A3 drive RS1-RS4. JP1 selects /CSEL0 or
    /CSEL1 and JP2 routes the interrupt output to /NMI.

***************************************************************************/

#ifndef MAME_BUS_C64_SHORTBUS_DUART_H
#define MAME_BUS_C64_SHORTBUS_DUART_H

#pragma once

#include "shortbus.h"
#include "bus/rs232/rs232.h"
#include "machine/mc68681.h"


class ide64_shortbus_duart_device : public device_t, public device_ide64_shortbus_interface
{
public:
	ide64_shortbus_duart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

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
	void outport_w(uint8_t data);

	required_device<xr68c681_device> m_duart;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_ioport m_jp1;
	required_ioport m_jp2;
};

DECLARE_DEVICE_TYPE(IDE64_SHORTBUS_DUART, ide64_shortbus_duart_device)

#endif // MAME_BUS_C64_SHORTBUS_DUART_H
