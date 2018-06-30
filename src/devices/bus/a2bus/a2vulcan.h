// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2vulcan.h

    Applied Engineering Vulcan and Vulcan Gold IDE controllers

*********************************************************************/

#ifndef MAME_BUS_A2BUS_VULCAN_H
#define MAME_BUS_A2BUS_VULCAN_H

#pragma once

#include "a2bus.h"
#include "machine/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_vulcanbase_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_vulcanbase_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<ata_interface_device> m_ata;

	uint8_t *m_rom;
	uint8_t m_ram[8*1024];

private:
	uint16_t m_lastdata;
	int m_rombank, m_rambank;
	bool m_last_read_was_0;
};

class a2bus_vulcan_device : public a2bus_vulcanbase_device
{
public:
	a2bus_vulcan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	virtual void device_start() override;

protected:
};

class a2bus_vulcangold_device : public a2bus_vulcanbase_device
{
public:
	a2bus_vulcangold_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_VULCAN,     a2bus_vulcan_device)
DECLARE_DEVICE_TYPE(A2BUS_VULCANGOLD, a2bus_vulcangold_device)

#endif // MAME_BUS_A2BUS_VULCAN_H
