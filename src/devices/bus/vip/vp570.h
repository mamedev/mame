// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Memory Expansion Board VP-570 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP570_H
#define MAME_BUS_VIP_VP570_H

#pragma once

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
	vp570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual uint8_t vip_program_r(offs_t offset, int cs, int cdef, int *minh) override;
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) override;

private:
	memory_share_creator<uint8_t> m_ram;
	required_ioport m_base;
	required_ioport m_sw1;
};


// device type definition
DECLARE_DEVICE_TYPE(VP570, vp570_device)


#endif // MAME_BUS_VIP_VP570_H
