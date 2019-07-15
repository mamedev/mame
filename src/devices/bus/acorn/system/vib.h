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
#include "machine/clock.h"
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
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

	required_device<i8255_device> m_ppi8255;
	required_device<via6522_device> m_via6522;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<input_merger_device> m_irqs;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_VIB, acorn_vib_device)


#endif // MAME_BUS_ACORN_SYSTEM_VIB_H
