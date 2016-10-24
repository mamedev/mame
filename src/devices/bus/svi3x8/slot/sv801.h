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
	sv801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void intrq_w(int state);
	void drq_w(int state);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void motor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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
