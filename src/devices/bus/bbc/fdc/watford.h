// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics DDB FDC

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_WATFORD_H
#define MAME_BUS_BBC_FDC_WATFORD_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_watfordfdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	static void floppy_formats(format_registration &fr);

protected:
	// construction/destruction
	bbc_watfordfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class bbc_weddb2_device : public bbc_watfordfdc_device
{
public:
	bbc_weddb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	int m_drive_control;
};

class bbc_weddb3_device : public bbc_watfordfdc_device
{
public:
	bbc_weddb3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	int m_drive_control;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_WEDDB2, bbc_weddb2_device)
DECLARE_DEVICE_TYPE(BBC_WEDDB3, bbc_weddb3_device)


#endif // MAME_BUS_BBC_FDC_WATFORD_H
