// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskiing.h

    Apple II Disk II Controller Card, new floppy

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2DISKIING_H
#define MAME_BUS_A2BUS_A2DISKIING_H

#pragma once

#include "a2bus.h"

#include "machine/wozfdc.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class diskiing_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	diskiing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	required_device<diskii_fdc_device> m_wozfdc;
	required_device_array<floppy_connector, 2> m_floppy;

	const uint8_t *m_rom;

private:
	static void floppy_formats(format_registration &fr);
};

class a2bus_diskiing_device: public diskiing_device
{
public:
	a2bus_diskiing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_diskiing13_device: public diskiing_device
{
public:
	a2bus_diskiing13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
};

class a2bus_applesurance_device: public diskiing_device
{
public:
	a2bus_applesurance_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_reset() override ATTR_COLD;

	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	virtual void write_c800(uint16_t offset, uint8_t data) override
	{
		if (offset == 0x7ff)
		{
			m_c800_bank = data & 1;
		}
	}

	virtual bool take_c800() override { return true; }

private:
	int m_c800_bank;
};

class a2bus_agat7flop_device : public diskiing_device
{
public:
	a2bus_agat7flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class a2bus_agat9flop_device : public diskiing_device
{
public:
	a2bus_agat9flop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_DISKIING, a2bus_diskiing_device)
DECLARE_DEVICE_TYPE(A2BUS_DISKIING13, a2bus_diskiing13_device)
DECLARE_DEVICE_TYPE(A2BUS_APPLESURANCE, a2bus_applesurance_device)
DECLARE_DEVICE_TYPE(A2BUS_AGAT7_FDC, a2bus_agat7flop_device)
DECLARE_DEVICE_TYPE(A2BUS_AGAT9_FDC, a2bus_agat9flop_device)

#endif  // MAME_BUS_A2BUS_A2DISKIING_H
