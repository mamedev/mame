// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom NASBUS Floppy Disc Controller

***************************************************************************/

#ifndef MAME_BUS_NASBUS_FLOPPY_H
#define MAME_BUS_NASBUS_FLOPPY_H

#pragma once

#include "nasbus.h"
#include "imagedev/floppy.h"
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

	DECLARE_READ8_MEMBER(select_r);
	DECLARE_WRITE8_MEMBER(select_w);
	DECLARE_READ8_MEMBER(status_r);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

private:
	TIMER_CALLBACK_MEMBER(motor_off);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	floppy_image_device *m_floppy;
	emu_timer *m_motor;

	uint8_t m_select;
};

// device type definition
DECLARE_DEVICE_TYPE(NASCOM_FDC, nascom_fdc_device)

#endif // MAME_BUS_NASBUS_FLOPPY_H
