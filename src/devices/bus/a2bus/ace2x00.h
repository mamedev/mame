// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ace2x00.h

    Helpers for the Franklin Ace 2x00's slot 1 and 6 internal peripherals

*********************************************************************/

#ifndef MAME_BUS_A2BUS_ACE2X00_H
#define MAME_BUS_A2BUS_ACE2X00_H

#pragma once

#include "a2bus.h"
#include "imagedev/floppy.h"
#include "machine/iwm.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ace2x00_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ace2x00_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;
	virtual bool take_c800() override;

	uint8_t *m_rom;
};

class a2bus_ace2x00_slot1_device : public a2bus_ace2x00_device
{
public:
	a2bus_ace2x00_slot1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
};

class a2bus_ace2x00_slot6_device : public a2bus_ace2x00_device
{
public:
	a2bus_ace2x00_slot6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

	required_device<iwm_device> m_iwm;
	required_device_array<floppy_connector, 2> m_floppy;

private:
	void devsel_w(u8 data);
	void phases_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_ACE2X00_SLOT1, a2bus_ace2x00_slot1_device)
DECLARE_DEVICE_TYPE(A2BUS_ACE2X00_SLOT6, a2bus_ace2x00_slot6_device)

#endif // MAME_BUS_A2BUS_ACE2X00_H
