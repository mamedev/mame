// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS Floppy Disc Controller

***************************************************************************/

#pragma once

#ifndef __NASBUS_FLOPPY_H__
#define __NASBUS_FLOPPY_H__

#include "emu.h"
#include "nasbus.h"
#include "machine/wd_fdc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nascom_fdc_device

class nascom_fdc_device : public device_t, public device_nasbus_card_interface
{
public:
	// construction/destruction
	nascom_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

private:
	void motor_off(void *ptr, int32_t param);

	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	floppy_image_device *m_floppy;
	emu_timer *m_motor;

	uint8_t m_select;
};

// device type definition
extern const device_type NASCOM_FDC;

#endif // __NASBUS_FLOPPY_H__
