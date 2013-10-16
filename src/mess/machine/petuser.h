// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET User Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                  VIDEO       2      B       CA1
                _SRQ IN       3      C       PA0
                    EOI       4      D       PA1
                   DIAG       5      E       PA2
           #2 CASS READ       6      F       PA3
             CASS WRITE       7      H       PA4
           #1 CASS READ       8      J       PA5
             VERT DRIVE       9      K       PA6
             HORZ DRIVE      10      L       PA7
                    GND      11      M       CB2
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __PET_USER_PORT__
#define __PET_USER_PORT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PET_USER_PORT_TAG       "user"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define PET_USER_PORT_INTERFACE(_name) \
	const pet_user_port_interface (_name) =


#define MCFG_PET_USER_PORT_ADD(_tag, _config, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PET_USER_PORT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_user_port_interface

struct pet_user_port_interface
{
	devcb_write_line    m_out_ca1_cb;
	devcb_write_line    m_out_cb2_cb;
};


// ======================> pet_user_port_device

class device_pet_user_port_interface;

class pet_user_port_device : public device_t,
								public pet_user_port_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// computer interface
	DECLARE_READ8_MEMBER( pa_r );
	DECLARE_WRITE8_MEMBER( pa_w );
	DECLARE_READ_LINE_MEMBER( ca1_r );
	DECLARE_WRITE_LINE_MEMBER( ca1_w );
	DECLARE_READ_LINE_MEMBER( cb2_r );
	DECLARE_WRITE_LINE_MEMBER( cb2_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( via_ca1_w ) { m_out_ca1_func(state); }
	DECLARE_WRITE_LINE_MEMBER( via_cb2_w ) { m_out_cb2_func(state); }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	devcb_resolved_write_line   m_out_ca1_func;
	devcb_resolved_write_line   m_out_cb2_func;

	device_pet_user_port_interface *m_card;
};


// ======================> device_pet_user_port_interface

// class representing interface-specific live pet_expansion card
class device_pet_user_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_pet_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pet_user_port_interface();

	virtual UINT8 pet_pa_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void pet_pa_w(address_space &space, offs_t offset, UINT8 data) { };

	virtual int pet_ca1_r() { return 1; };
	virtual void pet_ca1_w(int state) { };
	virtual int pet_cb2_r() { return 1; };
	virtual void pet_cb2_w(int state) { };

protected:
	pet_user_port_device *m_slot;
};


// device type definition
extern const device_type PET_USER_PORT;


// slot devices
SLOT_INTERFACE_EXTERN( pet_user_port_cards );



#endif
