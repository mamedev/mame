// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AMS 3" Microdrive Disc System

**********************************************************************/


#ifndef MAME_BUS_BBC_FDC_AMS_H
#define MAME_BUS_BBC_FDC_AMS_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/i8271.h"
#include "formats/acorn_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_ams3_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	// construction/destruction
	bbc_ams3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_WRITE_LINE_MEMBER(side_w);

	required_device<i8271_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_AMS3, bbc_ams3_device)


#endif // MAME_BUS_BBC_FDC_AMS_H
