// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Keypad Interface Board VP585 emulation

**********************************************************************/

#pragma once

#ifndef __VP585__
#define __VP585__

#include "emu.h"
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
	vp585_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_vip_expansion_card_interface overrides
	virtual void vip_io_w(address_space &space, offs_t offset, UINT8 data);
	virtual int vip_ef3_r();
	virtual int vip_ef4_r();

private:
	required_ioport m_j1;
	required_ioport m_j2;

	UINT8 m_keylatch;
};


// device type definition
extern const device_type VP585;


#endif
