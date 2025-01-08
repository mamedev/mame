// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Printer Interface

    Dick Smith Electronics X-7320

***************************************************************************/

#ifndef MAME_BUS_VTECH_IOEXP_PRINTER_H
#define MAME_BUS_VTECH_IOEXP_PRINTER_H

#pragma once

#include "ioexp.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_printer_interface_device

class vtech_printer_interface_device : public vtech_ioexp_device
{
public:
	// construction/destruction
	vtech_printer_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;

	void busy_w(int state);
	uint8_t busy_r();
	void strobe_w(uint8_t data);

	int m_centronics_busy;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_PRINTER_INTERFACE, vtech_printer_interface_device)

#endif // MAME_BUS_VTECH_IOEXP_PRINTER_H
