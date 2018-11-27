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
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_watfordfdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

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
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

private:
	required_device<wd_fdc_device_base> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
};

class bbc_weddb3_device : public bbc_watfordfdc_device
{
public:
	bbc_weddb3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

private:
	required_device<wd_fdc_device_base> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_WEDDB2, bbc_weddb2_device)
DECLARE_DEVICE_TYPE(BBC_WEDDB3, bbc_weddb3_device)


#endif // MAME_BUS_BBC_FDC_WATFORD_H
