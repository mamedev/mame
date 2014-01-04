// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    geoCable Centronics Cable emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_GEOCABLE__
#define __C64_GEOCABLE__


#include "emu.h"
#include "user.h"
#include "bus/centronics/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_geocable_device

class c64_geocable_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_geocable_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( busy_w );

protected:
	// device-level overrides
	virtual void device_start();

	// device_pet_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(input_8);
	virtual WRITE_LINE_MEMBER(input_c) { if (state) m_parallel_output |= 1; else m_parallel_output &= ~1; update_output(); }
	virtual WRITE_LINE_MEMBER(input_d) { if (state) m_parallel_output |= 2; else m_parallel_output &= ~2; update_output(); }
	virtual WRITE_LINE_MEMBER(input_e) { if (state) m_parallel_output |= 4; else m_parallel_output &= ~4; update_output(); }
	virtual WRITE_LINE_MEMBER(input_f) { if (state) m_parallel_output |= 8; else m_parallel_output &= ~8; update_output(); }
	virtual WRITE_LINE_MEMBER(input_h) { if (state) m_parallel_output |= 16; else m_parallel_output &= ~16; update_output(); }
	virtual WRITE_LINE_MEMBER(input_j) { if (state) m_parallel_output |= 32; else m_parallel_output &= ~32; update_output(); }
	virtual WRITE_LINE_MEMBER(input_k) { if (state) m_parallel_output |= 64; else m_parallel_output &= ~64; update_output(); }
	virtual WRITE_LINE_MEMBER(input_l) { if (state) m_parallel_output |= 128; else m_parallel_output &= ~128; update_output(); }

private:
	required_device<centronics_device> m_centronics;

	void update_output();

	UINT8 m_parallel_output;
};


// device type definition
extern const device_type C64_GEOCABLE;


#endif
