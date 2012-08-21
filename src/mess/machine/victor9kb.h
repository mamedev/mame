/**********************************************************************

    Victor 9000 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __VICTOR9K_KEYBOARD__
#define __VICTOR9K_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VICTOR9K_KEYBOARD_TAG	"victor9kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR9K_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(VICTOR9K_KEYBOARD_TAG, VICTOR9K_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define VICTOR9K_KEYBOARD_INTERFACE(_name) \
	const victor9k_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor9k_keyboard_interface

struct victor9k_keyboard_interface
{
	devcb_write_line	m_out_kbrdy_cb;
};


// ======================> victor9k_keyboard_device

class victor9k_keyboard_device :  public device_t,
								  public victor9k_keyboard_interface
{
public:
    // construction/destruction
    victor9k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ_LINE_MEMBER( kbrdy_r );
	DECLARE_WRITE_LINE_MEMBER( kback_w );
	DECLARE_READ_LINE_MEMBER( kbdata_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_READ8_MEMBER( kb_t1_r );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "victor9kb"; }

private:
	devcb_resolved_write_line	m_out_kbrdy_func;

	required_device<cpu_device> m_maincpu;

	UINT8 m_y;
	int m_kbrdy;
	int m_kbdata;
	int m_kback;
};


// device type definition
extern const device_type VICTOR9K_KEYBOARD;



#endif
