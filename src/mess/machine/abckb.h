/**********************************************************************

    Luxor ABC 800/802/806/1600 keyboard port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ABC_KEYBOARD_PORT__
#define __ABC_KEYBOARD_PORT__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC_KEYBOARD_PORT_TAG	"kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC_KEYBOARD_PORT_ADD(_def_slot, _trxc, _keydown) \
	MCFG_DEVICE_ADD(ABC_KEYBOARD_PORT_TAG, ABC_KEYBOARD_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(abc_keyboard_devices, _def_slot, false) \
	downcast<abc_keyboard_port_device *>(device)->set_callbacks(DEVCB2_##_trxc, DEVCB2_##_keydown);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class abc_keyboard_interface;

class abc_keyboard_port_device : public device_t,
									public device_slot_interface
{
public:
	// construction/destruction
	abc_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _trxc, class _keydown> void set_callbacks(_trxc trxc, _keydown keydown) {
		m_write_trxc.set_callback(trxc);
		m_write_keydown.set_callback(keydown);
	}

	// computer interface
	DECLARE_READ_LINE_MEMBER( rxd_r );
	DECLARE_WRITE_LINE_MEMBER( txd_w );

	// peripheral interface
	DECLARE_WRITE_LINE_MEMBER( trxc_w );
	DECLARE_WRITE_LINE_MEMBER( keydown_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	devcb2_write_line m_write_trxc;
	devcb2_write_line m_write_keydown;

	abc_keyboard_interface *m_card;
};


class abc_keyboard_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	abc_keyboard_interface(const machine_config &mconfig, device_t &device);

	virtual int rxd_r() { return 1; };
	virtual void txd_w(int state) { };

protected:
	abc_keyboard_port_device *m_slot;
};


// device type definition
extern const device_type ABC_KEYBOARD_PORT;


// supported devices
SLOT_INTERFACE_EXTERN( abc_keyboard_devices );



#endif
