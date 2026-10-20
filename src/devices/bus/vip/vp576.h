// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VP551 Super Sound 4-channel Expander Package emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP576_H
#define MAME_BUS_VIP_VP576_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp576_device

class vp576_device : public device_t, public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp576_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) override;
	virtual void vip_sc_w(int n, int sc) override;
	virtual void vip_q_w(int state) override;
	virtual void vip_run_w(int state) override;

private:
	static constexpr unsigned MAX_SLOTS = 2;

	void exp1_int_w(int state) { m_int[0] = state; update_interrupts(); }
	void exp2_int_w(int state) { m_int[1] = state; update_interrupts(); }

	void update_interrupts();

	required_device_array<vip_expansion_slot_device, MAX_SLOTS> m_expansion_slot;

	int m_int[MAX_SLOTS];
};


// device type definition
DECLARE_DEVICE_TYPE(VP576, vp576_device)

#endif // MAME_BUS_VIP_VP576_H
