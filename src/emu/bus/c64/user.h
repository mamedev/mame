// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       /FLAG2
                 /RESET       3      C       PB0
                   CNT1       4      D       PB1
                    SP1       5      E       PB2
                   CNT2       6      F       PB3
                    SP2       7      H       PB4
                   /PC2       8      J       PB5
                    ATN       9      K       PB6
                  +9VAC      10      L       PB7
                  +9VAC      11      M       PA2
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __C64_USER_PORT__
#define __C64_USER_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define C64_USER_PORT_TAG       "user"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_C64_USER_PORT_ADD(_tag, _slot_intf, _def_slot, _reset) \
	MCFG_DEVICE_ADD(_tag, C64_USER_PORT, 0) \
	downcast<c64_user_port_device *>(device)->set_reset_callback(DEVCB2_##_reset); \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_C64_USER_PORT_CIA1_CALLBACKS(_cnt, _sp) \
	downcast<c64_user_port_device *>(device)->set_cia1_callbacks(DEVCB2_##_cnt, DEVCB2_##_sp);

#define MCFG_C64_USER_PORT_CIA2_CALLBACKS(_cnt, _sp, _flag) \
	downcast<c64_user_port_device *>(device)->set_cia2_callbacks(DEVCB2_##_cnt, DEVCB2_##_sp, DEVCB2_##_flag);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_user_port_device

class device_c64_user_port_interface;

class c64_user_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	c64_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _reset> void set_reset_callback(_reset reset) { m_write_reset.set_callback(reset); }

	template<class _cnt, class _sp> void set_cia1_callbacks(_cnt cnt, _sp sp) {
		m_write_cnt1.set_callback(cnt);
		m_write_sp1.set_callback(sp);
	}

	template<class _cnt, class _sp, class _flag> void set_cia2_callbacks(_cnt cnt, _sp sp, _flag flag) {
		m_write_cnt2.set_callback(cnt);
		m_write_sp2.set_callback(sp);
		m_write_flag2.set_callback(flag);
	}

	// computer interface
	DECLARE_READ8_MEMBER( pb_r );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_READ_LINE_MEMBER( pa2_r );
	DECLARE_WRITE_LINE_MEMBER( pa2_w );
	DECLARE_WRITE_LINE_MEMBER( pc2_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( cnt1_w );
	DECLARE_WRITE_LINE_MEMBER( sp1_w );
	DECLARE_WRITE_LINE_MEMBER( cnt2_w );
	DECLARE_WRITE_LINE_MEMBER( sp2_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( cia_cnt1_w ) { m_write_cnt1(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_sp1_w ) { m_write_sp1(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_cnt2_w ) { m_write_cnt2(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_sp2_w ) { m_write_sp2(state); }
	DECLARE_WRITE_LINE_MEMBER( cia_flag2_w ) { m_write_flag2(state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { m_write_reset(state); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	devcb2_write_line   m_write_cnt1;
	devcb2_write_line   m_write_sp1;
	devcb2_write_line   m_write_cnt2;
	devcb2_write_line   m_write_sp2;
	devcb2_write_line   m_write_flag2;
	devcb2_write_line   m_write_reset;

	device_c64_user_port_interface *m_card;
};


// ======================> device_c64_user_port_interface

// class representing interface-specific live c64_expansion card
class device_c64_user_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_c64_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_c64_user_port_interface();

	virtual UINT8 c64_pb_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void c64_pb_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual int c64_pa2_r() { return 1; };
	virtual void c64_pa2_w(int state) { };
	virtual void c64_cnt1_w(int state) { };
	virtual void c64_sp1_w(int state) { };
	virtual void c64_pc2_w(int state) { };
	virtual void c64_cnt2_w(int state) { };
	virtual void c64_sp2_w(int state) { };
	virtual void c64_atn_w(int state) { };

protected:
	c64_user_port_device *m_slot;
};


// device type definition
extern const device_type C64_USER_PORT;


// slot devices
#include "4cga.h"
#include "4dxh.h"
#include "4ksa.h"
#include "4tba.h"
#include "bn1541.h"
#include "geocable.h"
#include "vic1011.h"

SLOT_INTERFACE_EXTERN( c64_user_port_cards );



#endif
