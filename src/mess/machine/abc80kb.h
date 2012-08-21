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

#define ABC80_KEYBOARD_TAG	"abc80kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC80_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(ABC80_KEYBOARD_TAG, ABC80_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define ABC80_KEYBOARD_INTERFACE(_name) \
	const abc80_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_keyboard_interface

struct abc80_keyboard_interface
{
	devcb_write_line	m_out_keydown_cb;
};


// ======================> abc80_keyboard_device

class abc80_keyboard_device :  public device_t,
							   public abc80_keyboard_interface
{
public:
    // construction/destruction
    abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	UINT8 data_r();

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
	virtual void device_reset();

private:
	devcb_resolved_write_line	m_out_keydown_func;

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ABC80_KEYBOARD;



#endif
