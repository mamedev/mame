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

#define C64_USER_PORT_INTERFACE(_name) \
	const c64_user_port_interface (_name) =


#define MCFG_C64_USER_PORT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, C64_USER_PORT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_user_port_interface

struct c64_user_port_interface
{
	devcb_write_line    m_out_cnt1_cb;
	devcb_write_line    m_out_sp1_cb;
	devcb_write_line    m_out_cnt2_cb;
	devcb_write_line    m_out_sp2_cb;
	devcb_write_line    m_out_flag2_cb;
	devcb_write_line    m_out_reset_cb;
};


// ======================> c64_user_port_device

class device_c64_user_port_interface;

class c64_user_port_device : public device_t,
								public c64_user_port_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	c64_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~c64_user_port_device();

	// computer interface
	DECLARE_READ8_MEMBER( pb_r );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_READ_LINE_MEMBER( pa2_r );
	DECLARE_WRITE_LINE_MEMBER( pa2_w );
	DECLARE_WRITE_LINE_MEMBER( pc2_w );
	DECLARE_WRITE_LINE_MEMBER( port_reset_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( sp1_w );
	DECLARE_WRITE_LINE_MEMBER( cnt1_w );
	DECLARE_WRITE_LINE_MEMBER( sp2_w );
	DECLARE_WRITE_LINE_MEMBER( cnt2_w );
	DECLARE_WRITE_LINE_MEMBER( flag2_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	devcb_resolved_write_line   m_out_cnt1_func;
	devcb_resolved_write_line   m_out_sp1_func;
	devcb_resolved_write_line   m_out_cnt2_func;
	devcb_resolved_write_line   m_out_sp2_func;
	devcb_resolved_write_line   m_out_flag2_func;
	devcb_resolved_write_line   m_out_reset_func;

	device_c64_user_port_interface *m_cart;
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

	// reset
	virtual void c64_reset_w(int state) { };

protected:
	c64_user_port_device *m_slot;
};


// device type definition
extern const device_type C64_USER_PORT;



#endif
