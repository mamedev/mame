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
#include "imagedev/floppy.h"
#include "formats/flopimg.h"
#include "machine/wozfdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class diskiing_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	diskiing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	required_device<diskii_fdc_device> m_wozfdc;
	required_device_array<floppy_connector, 2> m_floppy;

private:
	const uint8_t *m_rom;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
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
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_DISKIING, a2bus_diskiing_device)
DECLARE_DEVICE_TYPE(A2BUS_DISKIING13, a2bus_diskiing13_device)

#endif  // MAME_BUS_A2BUS_A2DISKIING_H
