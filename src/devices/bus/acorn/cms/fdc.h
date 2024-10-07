// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS Floppy Disc Controller Board

**********************************************************************/

#ifndef MAME_BUS_ACORN_CMS_FDC_H
#define MAME_BUS_ACORN_CMS_FDC_H

#pragma once

#include "bus/acorn/bus.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cms_fdc_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	cms_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t wd1770_state_r();
	void wd1770_control_w(uint8_t data);

	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
};


// device type definition
DECLARE_DEVICE_TYPE(CMS_FDC, cms_fdc_device)


#endif // MAME_BUS_ACORN_CMS_FDC_H
