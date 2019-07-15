// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana QFS 8877A FDC

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_CUMANA_H
#define MAME_BUS_BBC_FDC_CUMANA_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cumanafdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);

protected:
	// construction/destruction
	bbc_cumanafdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	bool m_invert;

private:
	int m_drive_control;
	int m_fdc_ie;
};

class bbc_cumana1_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_cumana2_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CUMANA1, bbc_cumana1_device)
DECLARE_DEVICE_TYPE(BBC_CUMANA2, bbc_cumana2_device)


#endif // MAME_BUS_BBC_FDC_CUMANA_H
