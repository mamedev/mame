// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7_ports.h

    Implementation of the Agat-7 ports card (decimal 3.089.106)

*********************************************************************/

#ifndef MAME_BUS_A2BUS_AGAT7_PORTS_H
#define MAME_BUS_A2BUS_AGAT7_PORTS_H

#pragma once

#include "a2bus.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/i8255.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agat7_ports_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_agat7_ports_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_agat7_ports_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_ioport m_printer_cfg;

	required_device<i8255_device> m_d9;
	required_device<i8251_device> m_d10;
	required_device<centronics_device> m_centronics;

	void write_portb(uint8_t data);
	uint8_t read_portc();
	void write_centronics_busy(int state);

private:
	bool m_centronics_busy;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_AGAT7_PORTS, a2bus_agat7_ports_device)

#endif // MAME_BUS_A2BUS_AGAT7_PORTS_H
