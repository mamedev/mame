// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 FDC device (model B504)

**********************************************************************/

#ifndef MAME_BUS_ISA_P1_FDC_H
#define MAME_BUS_ISA_P1_FDC_H

#pragma once


#include "isa.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p1_fdc_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	p1_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(p1_fdc_r);
	DECLARE_WRITE8_MEMBER(p1_fdc_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(p1_fdc_irq_drq);

	required_device<fd1793_device> m_fdc;

public:
	void p1_wd17xx_aux_w(int data);
	uint8_t p1_wd17xx_aux_r();
	uint8_t p1_wd17xx_motor_r();
};


// device type definition
DECLARE_DEVICE_TYPE(P1_FDC, p1_fdc_device)


#endif // MAME_BUS_ISA_P1_FDC_H
