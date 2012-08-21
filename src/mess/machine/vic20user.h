/**********************************************************************

    Commodore VIC-20 User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       CB1
                 /RESET       3      C       PB0
                   JOY0       4      D       PB1
                   JOY1       5      E       PB2
                   JOY2       6      F       PB3
              LIGHT PEN       7      H       PB4
        CASSETTE SWITCH       8      J       PB5
                    ATN       9      K       PB6
                  +9VAC      10      L       PB7
                  +9VAC      11      M       CB2
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __VIC20_USER_PORT__
#define __VIC20_USER_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIC20_USER_PORT_TAG		"user"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define VIC20_USER_PORT_INTERFACE(_name) \
	const vic20_user_port_interface (_name) =


#define MCFG_VIC20_USER_PORT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, VIC20_USER_PORT, 0) \
    MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_user_port_interface

struct vic20_user_port_interface
{
	devcb_write_line	m_out_cb1_cb;
	devcb_write_line	m_out_cb2_cb;
    devcb_write_line	m_out_reset_cb;
};


// ======================> vic20_user_port_device

class device_vic20_user_port_interface;

class vic20_user_port_device : public device_t,
						       public vic20_user_port_interface,
						       public device_slot_interface
{
public:
	// construction/destruction
	vic20_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~vic20_user_port_device();

	// computer interface
	DECLARE_READ8_MEMBER( pb_r );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_READ_LINE_MEMBER( joy0_r );
	DECLARE_READ_LINE_MEMBER( joy1_r );
	DECLARE_READ_LINE_MEMBER( joy2_r );
	DECLARE_READ_LINE_MEMBER( light_pen_r );
	DECLARE_READ_LINE_MEMBER( cassette_switch_r );
	DECLARE_WRITE_LINE_MEMBER( cb1_w );
	DECLARE_WRITE_LINE_MEMBER( cb2_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( port_reset_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( via_cb1_w );
	DECLARE_WRITE_LINE_MEMBER( via_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	devcb_resolved_write_line	m_out_cb1_func;
	devcb_resolved_write_line	m_out_cb2_func;
    devcb_resolved_write_line	m_out_reset_func;

	device_vic20_user_port_interface *m_cart;
};


// ======================> device_vic20_user_port_interface

// class representing interface-specific live vic20_expansion card
class device_vic20_user_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vic20_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vic20_user_port_interface();

	virtual UINT8 vic20_pb_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void vic20_pb_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual int vic20_joy0_r() { return 1; };
	virtual int vic20_joy1_r() { return 1; };
	virtual int vic20_joy2_r() { return 1; };
	virtual int vic20_light_pen_r() { return 1; };
	virtual int vic20_cassette_switch_r() { return 1; };
	virtual void vic20_cb1_w(int state) { };
	virtual void vic20_cb2_w(int state) { };
	virtual void vic20_atn_w(int state) { };

	// reset
	virtual void vic20_reset_w(int state) { };

protected:
	vic20_user_port_device *m_slot;
};


// device type definition
extern const device_type VIC20_USER_PORT;



#endif
