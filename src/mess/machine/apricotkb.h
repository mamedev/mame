/**********************************************************************

    Apricot keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __APRICOT_KEYBOARD__
#define __APRICOT_KEYBOARD__


#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define APRICOT_KEYBOARD_TAG	"aprikb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_APRICOT_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(APRICOT_KEYBOARD_TAG, APRICOT_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define APRICOT_KEYBOARD_INTERFACE(_name) \
	const apricot_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_interface

struct apricot_keyboard_interface
{
	devcb_write_line	m_out_txd_cb;
};


// ======================> apricot_keyboard_device

class apricot_keyboard_device :  public device_t,
								 public apricot_keyboard_interface
{
public:
    // construction/destruction
    apricot_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	UINT8 read_keyboard();

	DECLARE_READ8_MEMBER( kb_lo_r );
	DECLARE_READ8_MEMBER( kb_hi_r );
	DECLARE_READ8_MEMBER( kb_p6_r );
	DECLARE_WRITE8_MEMBER( kb_p3_w );
	DECLARE_WRITE8_MEMBER( kb_y0_w );
	DECLARE_WRITE8_MEMBER( kb_y4_w );
	DECLARE_WRITE8_MEMBER( kb_y8_w );
	DECLARE_WRITE8_MEMBER( kb_yc_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	devcb_resolved_write_line	m_out_txd_func;

	UINT16 m_kb_y;
};


// device type definition
extern const device_type APRICOT_KEYBOARD;



#endif
