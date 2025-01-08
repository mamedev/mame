// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-801 Disk Controller

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV801_H
#define MAME_BUS_SVI3X8_SLOT_SV801_H

#pragma once

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
	sv801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void motor_w(uint8_t data);

	void intrq_w(int state);
	void drq_w(int state);

	static void floppy_formats(format_registration &fr);

	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	floppy_image_device *m_floppy;

	int m_irq;
	int m_drq;
};

// device type definition
DECLARE_DEVICE_TYPE(SV801, sv801_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV801_H
