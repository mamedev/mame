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
	nascom_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(select_r);
	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_READ8_MEMBER(status_r);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_reset_after_children();

private:
	TIMER_CALLBACK_MEMBER(motor_off);

	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	floppy_image_device *m_floppy;
	emu_timer *m_motor;

	UINT8 m_select;
};

// device type definition
extern const device_type NASCOM_FDC;

#endif // __NASBUS_FLOPPY_H__
