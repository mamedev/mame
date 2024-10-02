// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Super Sound System VP550 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP550_H
#define MAME_BUS_VIP_VP550_H

#pragma once

#include "exp.h"
#include "sound/cdp1863.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp550_device

class vp550_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp550_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) override;
	virtual void vip_sc_w(int n, int sc) override;
	virtual void vip_q_w(int state) override;
	virtual void vip_run_w(int state) override;

	TIMER_CALLBACK_MEMBER(sync_tick);

private:
	void octave_w(uint8_t data);
	void vlmna_w(uint8_t data);
	void vlmnb_w(uint8_t data);
	void sync_w(uint8_t data);

	required_device_array<cdp1863_device, 2> m_pfg;

	// timers
	emu_timer *m_sync_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(VP550, vp550_device)

#endif // MAME_BUS_VIP_VP550_H
