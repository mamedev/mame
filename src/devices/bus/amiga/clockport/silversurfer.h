// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    SilverSurfer RS-232 card

    NS16550 decoding A2-A4 only, so the eight registers appear twice.

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CLOCKPORT_SILVERSURFER_H
#define MAME_BUS_AMIGA_CLOCKPORT_SILVERSURFER_H

#pragma once

#include "clockport.h"
#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"


class amiga_clockport_silversurfer_device : public device_t, public device_amiga_clockport_interface
{
public:
	amiga_clockport_silversurfer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_amiga_clockport_interface implementation
	virtual uint8_t spare_r(offs_t offset) override;
	virtual void spare_w(offs_t offset, uint8_t data) override;

private:
	required_device<ns16550_device> m_uart;
};

DECLARE_DEVICE_TYPE(AMIGA_CLOCKPORT_SILVERSURFER, amiga_clockport_silversurfer_device)

#endif // MAME_BUS_AMIGA_CLOCKPORT_SILVERSURFER_H
