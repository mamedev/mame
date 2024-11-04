// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Keypad Interface Board VP585 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP585_H
#define MAME_BUS_VIP_VP585_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp585_device

class vp585_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp585_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual void vip_io_w(offs_t offset, uint8_t data) override;
	virtual int vip_ef3_r() override;
	virtual int vip_ef4_r() override;

private:
	required_ioport m_j1;
	required_ioport m_j2;

	uint8_t m_keylatch;
};


// device type definition
DECLARE_DEVICE_TYPE(VP585, vp585_device)

#endif // MAME_BUS_VIP_VP585_H
