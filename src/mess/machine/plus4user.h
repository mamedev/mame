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

#define MCFG_PLUS4_USER_PORT_ADD(_tag, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, PLUS4_USER_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



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
	virtual ~plus4_user_port_device();

	// computer interface
	DECLARE_READ8_MEMBER( p_r );
	DECLARE_WRITE8_MEMBER( p_w );
	DECLARE_READ_LINE_MEMBER( rxd_r );
	DECLARE_READ_LINE_MEMBER( dcd_r );
	DECLARE_READ_LINE_MEMBER( dsr_r );
	DECLARE_WRITE_LINE_MEMBER( txd_w );
	DECLARE_WRITE_LINE_MEMBER( dtr_w );
	DECLARE_WRITE_LINE_MEMBER( rts_w );
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( breset_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { };
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

	virtual UINT8 plus4_p_r() { return 0xff; };
	virtual void plus4_p_w(UINT8 data) { };

	virtual int plus4_rxd_r() { return 1; };
	virtual int plus4_dcd_r() { return 0; };
	virtual int plus4_dsr_r() { return 0; };
	virtual void plus4_txd_w(int state) { };
	virtual void plus4_dtr_w(int state) { };
	virtual void plus4_rts_w(int state) { };
	virtual void plus4_rxc_w(int state) { };
	virtual void plus4_atn_w(int state) { };

	// reset
	virtual void plus4_breset_w(int state) { };

protected:
	plus4_user_port_device *m_slot;
};


// device type definition
extern const device_type PLUS4_USER_PORT;



#endif
