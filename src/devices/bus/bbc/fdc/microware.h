// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    United Disk Memories DDFS FDC

    Microware DDFS FDC

**********************************************************************/


#ifndef MAME_BUS_BBC_FDC_MICROWARE_H
#define MAME_BUS_BBC_FDC_MICROWARE_H

#pragma once

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_microware_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	bbc_microware_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(motor_w);

	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MICROWARE, bbc_microware_device);


#endif /* MAME_BUS_BBC_FDC_MICROWARE_H */
