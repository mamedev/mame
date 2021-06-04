// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    booti.h

    Implementation of the Booti card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_BOOTI_H
#define MAME_BUS_A2BUS_BOOTI_H

#pragma once

#include "a2bus.h"
#include "machine/at28c64b.h"
#include "machine/ch376.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_booti_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_booti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_booti_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

private:
	required_device<at28c64b_device> m_flash;
	required_device<ch376_device> m_ch376;

	int m_rombank;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_BOOTI, a2bus_booti_device)

#endif // MAME_BUS_A2BUS_BOOTI_H
