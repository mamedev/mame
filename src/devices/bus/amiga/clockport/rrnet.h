// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    RR-Net Ethernet card

    CS8900A in 8-bit mode, IRQ not wired.

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CLOCKPORT_RRNET_H
#define MAME_BUS_AMIGA_CLOCKPORT_RRNET_H

#pragma once

#include "clockport.h"
#include "machine/cs8900a.h"


class amiga_clockport_rrnet_device : public device_t, public device_amiga_clockport_interface
{
public:
	amiga_clockport_rrnet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_amiga_clockport_interface implementation
	virtual uint8_t spare_r(offs_t offset) override;
	virtual void spare_w(offs_t offset, uint8_t data) override;

private:
	required_device<cs8900a_device> m_cs8900a;
};

DECLARE_DEVICE_TYPE(AMIGA_CLOCKPORT_RRNET, amiga_clockport_rrnet_device)

#endif // MAME_BUS_AMIGA_CLOCKPORT_RRNET_H
