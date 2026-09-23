// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Citizen 120D 9-pin dot matrix print mechanism (skeleton)

    Used by the Commodore MPS-1200/MPS-1250 (see bus/cbmiec/mps1200.cpp).

**********************************************************************/

#ifndef MAME_MACHINE_CITIZEN120D_H
#define MAME_MACHINE_CITIZEN120D_H

#pragma once

#include "machine/bitmap_printer.h"
#include "machine/steppers.h"


class citizen_120d_device : public device_t
{
public:
	citizen_120d_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void phase_w(u8 data);
	void solenoid_w(u16 data);
	void head_en_w(int state);
	void fault_led_w(int state);
	void ready_led_w(int state);
	bool at_home() const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_cr_stepper;
	required_device<stepper_device> m_pf_stepper;

	u8 m_phase_data;
	u16 m_solenoid_data;
	s16 m_cr_delta;
	u8 m_fire_slot;
	bool m_head_en;
};

DECLARE_DEVICE_TYPE(CITIZEN_120D, citizen_120d_device)

#endif // MAME_MACHINE_CITIZEN120D_H
