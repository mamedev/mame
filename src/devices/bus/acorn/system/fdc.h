// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Floppy Disc Controller Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_fdc.html

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_FDC_H
#define MAME_BUS_ACORN_SYSTEM_FDC_H

#pragma once

#include "bus/acorn/bus.h"
#include "imagedev/floppy.h"
#include "machine/i8271.h"
#include "formats/acorn_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_fdc_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER(bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_WRITE_LINE_MEMBER(side_w);

	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_FDC, acorn_fdc_device)


#endif // MAME_BUS_ACORN_SYSTEM_FDC_H
