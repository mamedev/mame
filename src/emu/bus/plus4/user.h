// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       P0
                _BRESET       3      C       RxD
                     P2       4      D       RTS
                     P3       5      E       DTR
                     P4       6      F       P7
                     P5       7      H       DCD
                     P0       8      J       P6
                    ATN       9      K       P1
                  +9VAC      10      L       DSR
                  +9VAC      11      M       TxD
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __PLUS4_USER_PORT__
#define __PLUS4_USER_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PLUS4_USER_PORT_TAG     "user"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PLUS4_USER_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PLUS4_USER_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_PLUS4_USER_PORT_4_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_4_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_5_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_5_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_6_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_6_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_7_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_7_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_8_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_8_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_B_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_b_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_C_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_c_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_F_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_f_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_H_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_h_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_J_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_j_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_K_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_k_handler(*device, DEVCB2_##_devcb);

#define MCFG_PLUS4_USER_PORT_L_HANDLER(_devcb) \
	devcb = &plus4_user_port_device::set_l_handler(*device, DEVCB2_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_user_port_device

class device_plus4_user_port_interface;

class plus4_user_port_device : public device_t,
	public device_slot_interface
{
public:
	// construction/destruction
	plus4_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_4_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_4_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_5_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_5_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_6_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_6_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_7_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_7_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_8_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_8_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_b_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_b_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_c_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_c_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_f_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_f_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_h_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_h_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_j_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_j_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_k_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_k_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_l_handler(device_t &device, _Object object) { return downcast<plus4_user_port_device &>(device).m_l_handler.set_callback(object); }

	// computer interface
	DECLARE_WRITE_LINE_MEMBER( write_4 );
	DECLARE_WRITE_LINE_MEMBER( write_5 );
	DECLARE_WRITE_LINE_MEMBER( write_6 );
	DECLARE_WRITE_LINE_MEMBER( write_7 );
	DECLARE_WRITE_LINE_MEMBER( write_8 );
	DECLARE_WRITE_LINE_MEMBER( write_9 );
	DECLARE_WRITE_LINE_MEMBER( write_b );
	DECLARE_WRITE_LINE_MEMBER( write_d );
	DECLARE_WRITE_LINE_MEMBER( write_e );
	DECLARE_WRITE_LINE_MEMBER( write_f );
	DECLARE_WRITE_LINE_MEMBER( write_j );
	DECLARE_WRITE_LINE_MEMBER( write_k );
	DECLARE_WRITE_LINE_MEMBER( write_m );

	// device interface
	devcb2_write_line m_4_handler;
	devcb2_write_line m_5_handler;
	devcb2_write_line m_6_handler;
	devcb2_write_line m_7_handler;
	devcb2_write_line m_8_handler;
	devcb2_write_line m_b_handler;
	devcb2_write_line m_c_handler;
	devcb2_write_line m_f_handler;
	devcb2_write_line m_h_handler;
	devcb2_write_line m_j_handler;
	devcb2_write_line m_k_handler;
	devcb2_write_line m_l_handler;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	device_plus4_user_port_interface *m_cart;
};


// ======================> device_plus4_user_port_interface

// class representing interface-specific live plus4_expansion card
class device_plus4_user_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_plus4_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_plus4_user_port_interface();

	DECLARE_WRITE_LINE_MEMBER( write_4 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_5 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_6 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_7 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_8 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_9 ) {}
	DECLARE_WRITE_LINE_MEMBER( write_b ) {}
	DECLARE_WRITE_LINE_MEMBER( write_d ) {}
	DECLARE_WRITE_LINE_MEMBER( write_e ) {}
	DECLARE_WRITE_LINE_MEMBER( write_f ) {}
	DECLARE_WRITE_LINE_MEMBER( write_j ) {}
	DECLARE_WRITE_LINE_MEMBER( write_k ) {}
	DECLARE_WRITE_LINE_MEMBER( write_m ) {}

protected:
	plus4_user_port_device *m_slot;
};


// device type definition
extern const device_type PLUS4_USER_PORT;


SLOT_INTERFACE_EXTERN( plus4_user_port_cards );

#endif
