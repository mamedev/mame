/**********************************************************************

    Visual 1050 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __V1050_KEYBOARD__
#define __V1050_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define V1050_KEYBOARD_TAG	"v1050kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_V1050_KEYBOARD_ADD() \
    MCFG_DEVICE_ADD(V1050_KEYBOARD_TAG, V1050_KEYBOARD, 0)


#define V1050_KEYBOARD_INTERFACE(_name) \
	const V1050_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> V1050_keyboard_device

class v1050_keyboard_device :  public device_t
{
public:
    // construction/destruction
    v1050_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_READ_LINE_MEMBER( so_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "v1050kb"; }

private:
	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_discrete;

	UINT8 m_y;
	int m_so;
};


// device type definition
extern const device_type V1050_KEYBOARD;



#endif
