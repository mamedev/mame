// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Timeworks PARTNER 128 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __C128_PARTNER__
#define __C128_PARTNER__

#include "emu.h"
#include "bus/c64/exp.h"
#include "bus/vcs_ctrl/ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> partner128_t

class partner128_t : public device_t,
						public device_c64_expansion_card_interface
						//public device_vcs_control_port_interface
{
public:
	// construction/destruction
	partner128_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( nmi_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

	// device_vcs_control_port_interface overrides
	virtual void vcs_joy_w(UINT8 data);

private:
	optional_shared_ptr<UINT8> m_ram;

	emu_timer *t_joyb2;
	int m_ram_a12_a7;
	int m_ls74_cd;
	int m_ls74_q1;
	int m_ls74_q2;
	int m_joyb2;
};


// device type definition
extern const device_type C128_PARTNER;


#endif
