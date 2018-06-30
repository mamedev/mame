// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2dx1.h

    Implementation of the Decillionix DX-1 sampler card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2DX1_H
#define MAME_BUS_A2BUS_A2DX1_H

#pragma once

#include "a2bus.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_dx1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

	required_device<dac_byte_interface> m_dac;
	required_device<dac_byte_interface> m_dacvol;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_DX1, a2bus_dx1_device)

#endif // MAME_BUS_A2BUS_A2DX1_H
