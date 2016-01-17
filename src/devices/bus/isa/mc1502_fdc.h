// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Electronika MC 1502 FDC device

**********************************************************************/

#pragma once

#ifndef __MC1502_FDC__
#define __MC1502_FDC__

#include "emu.h"

#include "imagedev/flopdrv.h"
#include "isa.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mc1502_fdc_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	mc1502_fdc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	TIMER_CALLBACK_MEMBER( motor_callback );

	DECLARE_READ8_MEMBER(mc1502_fdc_r);
	DECLARE_READ8_MEMBER(mc1502_fdcv2_r);
	DECLARE_WRITE8_MEMBER(mc1502_fdc_w);
	DECLARE_WRITE_LINE_MEMBER( mc1502_fdc_irq_drq );

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<fd1793_t> m_fdc;
	int motor_on;
	emu_timer *motor_timer;

public:
	void mc1502_wd17xx_aux_w(UINT8 data);
	UINT8 mc1502_wd17xx_aux_r();
	UINT8 mc1502_wd17xx_drq_r();
	UINT8 mc1502_wd17xx_motor_r();

};


// device type definition
extern const device_type MC1502_FDC;


#endif
