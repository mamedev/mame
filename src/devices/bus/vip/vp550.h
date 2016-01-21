// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Super Sound System VP550 emulation

**********************************************************************/

#pragma once

#ifndef __VP550__
#define __VP550__

#include "emu.h"
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
	vp550_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	DECLARE_WRITE8_MEMBER( octave_w );
	DECLARE_WRITE8_MEMBER( vlmna_w );
	DECLARE_WRITE8_MEMBER( vlmnb_w );
	DECLARE_WRITE8_MEMBER( sync_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_vip_expansion_card_interface overrides
	virtual void vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh) override;
	virtual void vip_sc_w(int data) override;
	virtual void vip_q_w(int state) override;
	virtual void vip_run_w(int state) override;

private:
	required_device<cdp1863_device> m_pfg_a;
	required_device<cdp1863_device> m_pfg_b;

	// timers
	emu_timer *m_sync_timer;
};


// device type definition
extern const device_type VP550;


#endif
