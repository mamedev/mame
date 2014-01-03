// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __DIAG264_USER_PORT_LOOPBACK__
#define __DIAG264_USER_PORT_LOOPBACK__

#include "emu.h"
#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_user_port_loopback_device

class diag264_user_port_loopback_device :  public device_t,
	public device_plus4_user_port_interface
{
public:
	// construction/destruction
	diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_plus4_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(write_b) { m_slot->m_6_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_k) { m_slot->m_7_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_4) { m_slot->m_j_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_5) { m_slot->m_f_handler(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(write_6) { m_slot->m_b_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_7) { m_slot->m_k_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_j) { m_slot->m_4_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_f) { m_slot->m_5_handler(state); }

	//virtual DECLARE_WRITE_LINE_MEMBER(write_c) { m_slot->m_m_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_d) { m_slot->m_l_handler(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(write_e) { m_slot->m_h_handler(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(write_m) { m_slot->m_c_handler(state); }
	//virtual DECLARE_WRITE_LINE_MEMBER(write_l) { m_slot->m_d_handler(state); }
	//virtual DECLARE_WRITE_LINE_MEMBER(write_h) { m_slot->m_e_handler(state); }
};

// device type definition
extern const device_type DIAG264_USER_PORT_LOOPBACK;

#endif
