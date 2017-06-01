// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    laser128.h

    Helper to implement the Laser 128's built-in slot peripherals

*********************************************************************/

#ifndef MAME_BUS_A2BUS_LASER128_H
#define MAME_BUS_A2BUS_LASER128_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_laser128_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_laser128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	a2bus_laser128_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;
	virtual uint8_t read_c800(address_space &space, uint16_t offset) override;
	virtual void write_c800(address_space &space, uint16_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	uint8_t *m_rom;
	uint8_t m_slot7_ram[0x800];
	int m_slot7_bank, m_slot7_ram_bank;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_LASER128, a2bus_laser128_device)

#endif // MAME_BUS_A2BUS_LASER128_H
