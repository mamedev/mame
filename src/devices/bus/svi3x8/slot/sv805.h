// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-805 RS-232 Interface for SVI 318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV805_H
#define MAME_BUS_SVI3X8_SLOT_SV805_H

#pragma once

#include "slot.h"
#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv805_device

class sv805_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv805_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER( iorq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER( uart_intr_w );

	required_device<ins8250_device> m_uart;
	required_device<rs232_port_device> m_rs232;
};

// device type definition
DECLARE_DEVICE_TYPE(SV805, sv805_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV805_H
