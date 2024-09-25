// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Internal floppy drive for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_DRIVE_FLOPPY_H
#define MAME_BUS_SAMCOUPE_DRIVE_FLOPPY_H

#pragma once

#include "drive.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_floppy_device

class sam_floppy_device : public device_t, public device_samcoupe_drive_interface
{
public:
	// construction/destruction
	sam_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_drive;

	static void floppy_formats(format_registration &fr);
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_FLOPPY, sam_floppy_device)

#endif // MAME_BUS_SAMCOUPE_DRIVE_FLOPPY_H
