// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Electronika MC 1502 FDC device

**********************************************************************/

#ifndef MAME_BUS_ISA_MC1502_FDC_H
#define MAME_BUS_ISA_MC1502_FDC_H

#pragma once


#include "isa.h"
#include "imagedev/floppy.h"
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

	template <typename T>
	void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

	TIMER_CALLBACK_MEMBER(motor_callback);

	uint8_t mc1502_fdc_r(offs_t offset);
	uint8_t mc1502_fdcv2_r(offs_t offset);
	void mc1502_fdc_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mc1502_fdc_irq_drq(int state);

	required_device<fd1793_device> m_fdc;
	int motor_on;
	u8 m_control;
	emu_timer *motor_timer;
	required_device<cpu_device> m_cpu;
	void motors_onoff();
public:
	void mc1502_wd17xx_aux_w(uint8_t data);
	uint8_t mc1502_wd17xx_aux_r();
	uint8_t mc1502_wd17xx_drq_r();
	uint8_t mc1502_wd17xx_motor_r();
};


// device type definition
DECLARE_DEVICE_TYPE(MC1502_FDC, mc1502_fdc_device)

#endif // MAME_BUS_ISA_MC1502_FDC_H
