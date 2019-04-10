// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Software EXDOS Disk Controller Module emulation

**********************************************************************/

#ifndef MAME_BUS_EP64_EXDOS_H
#define MAME_BUS_EP64_EXDOS_H

#pragma once

#include "exp.h"
#include "formats/ep64_dsk.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ep64_exdos_device

class ep64_exdos_device : public device_t,
							public device_ep64_expansion_bus_card_interface
{
public:
	// construction/destruction
	ep64_exdos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	required_memory_region m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(EP64_EXDOS, ep64_exdos_device)


#endif // MAME_BUS_EP64_EXDOS_H
