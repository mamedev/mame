/**********************************************************************

    Tandy 2000 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __TANDY2K_KEYBOARD__
#define __TANDY2K_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TANDY2K_KEYBOARD_TAG	"tandy2kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TANDY2K_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(TANDY2K_KEYBOARD_TAG, TANDY2K_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define TANDY2K_KEYBOARD_INTERFACE(_name) \
	const tandy2k_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tandy2k_keyboard_interface

struct tandy2k_keyboard_interface
{
	devcb_write_line	m_out_clock_cb;
	devcb_write_line	m_out_data_cb;
};


// ======================> tandy2k_keyboard_device

class tandy2k_keyboard_device :  public device_t,
								 public tandy2k_keyboard_interface
{
public:
    // construction/destruction
    tandy2k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( power_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "tandy2kb"; }

private:
	devcb_resolved_write_line	m_out_clock_func;
	devcb_resolved_write_line	m_out_data_func;

	required_device<cpu_device> m_maincpu;

	UINT16 m_keylatch;

	int m_clock;
	int m_data;
};


// device type definition
extern const device_type TANDY2K_KEYBOARD;



#endif
