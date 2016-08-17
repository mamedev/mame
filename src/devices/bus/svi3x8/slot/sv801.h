// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-801 Disk Controller

***************************************************************************/

#pragma once

#ifndef __SVI3X8_SLOT_SV801_H__
#define __SVI3X8_SLOT_SV801_H__

#include "emu.h"
#include "slot.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv801_device

class sv801_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	virtual DECLARE_READ8_MEMBER( iorq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) override;

	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_WRITE8_MEMBER( motor_w );

	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	floppy_image_device *m_floppy;

	int m_irq;
	int m_drq;
};

// device type definition
extern const device_type SV801;

#endif // __SVI3X8_SLOT_SV801_H__
