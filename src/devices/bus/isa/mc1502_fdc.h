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
	mc1502_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void motor_callback(void *ptr, int32_t param);

	uint8_t mc1502_fdc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mc1502_fdcv2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mc1502_fdc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mc1502_fdc_irq_drq(int state);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<fd1793_t> m_fdc;
	int motor_on;
	emu_timer *motor_timer;

public:
	void mc1502_wd17xx_aux_w(uint8_t data);
	uint8_t mc1502_wd17xx_aux_r();
	uint8_t mc1502_wd17xx_drq_r();
	uint8_t mc1502_wd17xx_motor_r();

};


// device type definition
extern const device_type MC1502_FDC;


#endif
