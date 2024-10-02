// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2corvus.h

    Implementation of the Corvus flat-cable hard disk interface
    for the Apple II.

*********************************************************************/

#ifndef MAME_A2BUS_A2CORVUS_H
#define MAME_A2BUS_A2CORVUS_H

#pragma once

#include "a2bus.h"
#include "machine/corvushd.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvus_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_corvus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_device<corvus_hdc_device> m_corvushd;

private:
	uint8_t *m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_CORVUS, a2bus_corvus_device)

#endif // MAME_A2BUS_A2CORVUS_H
