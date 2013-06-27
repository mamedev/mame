/**********************************************************************

    Luxor ABC-80 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ABC80_KEYBOARD__
#define __ABC80_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC80_KEYBOARD_TAG  "abc80kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC80_KEYBOARD_ADD(_keydown) \
	MCFG_DEVICE_ADD(ABC80_KEYBOARD_TAG, ABC80_KEYBOARD, 0) \
	downcast<abc80_keyboard_device *>(device)->set_callback(DEVCB2_##_keydown);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_keyboard_device

class abc80_keyboard_device :  public device_t
{
public:
	// construction/destruction
	abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _keydown> void set_callback(_keydown keydown) { m_write_keydown.set_callback(keydown); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	UINT8 data_r();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	devcb2_write_line m_write_keydown;

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ABC80_KEYBOARD;



#endif
