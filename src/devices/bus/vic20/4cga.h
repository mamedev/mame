// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Classical Games/Protovision 4 Player Interface emulation

**********************************************************************/

#pragma once

#ifndef __C64_4CGA__
#define __C64_4CGA__


#include "emu.h"
#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4cga_device

class c64_4cga_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_4cga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER(write_joy3_0) { if (state) m_joy3 |= 1; else m_joy3 &= ~1; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy3_1) { if (state) m_joy3 |= 2; else m_joy3 &= ~2; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy3_2) { if (state) m_joy3 |= 4; else m_joy3 &= ~4; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy3_3) { if (state) m_joy3 |= 8; else m_joy3 &= ~8; update_output(); }

	DECLARE_WRITE_LINE_MEMBER(write_joy4_0) { if (state) m_joy4 |= 1; else m_joy4 &= ~1; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy4_1) { if (state) m_joy4 |= 2; else m_joy4 &= ~2; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy4_2) { if (state) m_joy4 |= 4; else m_joy4 &= ~4; update_output(); }
	DECLARE_WRITE_LINE_MEMBER(write_joy4_3) { if (state) m_joy4 |= 8; else m_joy4 &= ~8; update_output(); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_pet_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( input_l ) override;

private:
	void update_output();

	int m_port;
	UINT8 m_joy3;
	UINT8 m_joy4;
};


// device type definition
extern const device_type C64_4CGA;


#endif
