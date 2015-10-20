// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 F&M Joycard emulation

**********************************************************************/

#pragma once

#ifndef __COMX_JOY__
#define __COMX_JOY__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_joy_device

class comx_joy_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_io_r(address_space &space, offs_t offset);

private:
	required_ioport m_joy1;
	required_ioport m_joy2;
};


// device type definition
extern const device_type COMX_JOY;


#endif
