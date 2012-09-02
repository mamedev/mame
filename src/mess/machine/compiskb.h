/**********************************************************************

    Telenova Compis keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __COMPIS_KEYBOARD__
#define __COMPIS_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define COMPIS_KEYBOARD_TAG	"compiskb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COMPIS_KEYBOARD_ADD() \
    MCFG_DEVICE_ADD(COMPIS_KEYBOARD_TAG, COMPIS_KEYBOARD, 0)


#define COMPIS_KEYBOARD_INTERFACE(_name) \
	const COMPIS_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_keyboard_interface

struct compis_keyboard_interface
{
	devcb_write_line	m_out_int_cb;
};


// ======================> compis_keyboard_device

class compis_keyboard_device :  public device_t,
								public compis_keyboard_interface
{
public:
    // construction/destruction
    compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( dtr_w );
	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_READ_LINE_MEMBER( so_r );

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
	virtual void device_reset();

private:
	devcb_resolved_write_line	m_out_int_func;

	required_device<cpu_device> m_maincpu;

	int m_so;
};


// device type definition
extern const device_type COMPIS_KEYBOARD;



#endif
