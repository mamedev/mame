// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Versatile Interface Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_UIB.html

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_VIB_H
#define MAME_BUS_ACORN_SYSTEM_VIB_H

#pragma once

#include "bus/acorn/bus.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "machine/i8255.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/input_merger.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_vib_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_vib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template<mc14411_device::timer_id T> void write_acia_clock(int state);
	void irq_w(int state);

	required_device<i8255_device> m_ppi8255;
	required_device<via6522_device> m_via6522;
	required_device<acia6850_device> m_acia;
	required_device<mc14411_device> m_mc14411;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<input_merger_device> m_irqs;
	required_ioport m_txc;
	required_ioport m_rxc;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_VIB, acorn_vib_device)


#endif // MAME_BUS_ACORN_SYSTEM_VIB_H
