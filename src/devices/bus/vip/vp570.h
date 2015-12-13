// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Memory Expansion Board VP-570 emulation

**********************************************************************/

#pragma once

#ifndef __VP570__
#define __VP570__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp570_device

class vp570_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp570_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vip_expansion_card_interface overrides
	virtual UINT8 vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh) override;
	virtual void vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh) override;

private:
	optional_shared_ptr<UINT8> m_ram;
	required_ioport m_base;
	required_ioport m_sw1;
};


// device type definition
extern const device_type VP570;


#endif
