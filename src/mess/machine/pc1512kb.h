/**********************************************************************

    Amstrad PC1512 Keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __PC1512_KEYBOARD__
#define __PC1512_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PC1512_KEYBOARD_TAG	"pc1512_keyboard"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PC1512_KEYBOARD_ADD(_config) \
    MCFG_DEVICE_ADD(PC1512_KEYBOARD_TAG, PC1512_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define PC1512_KEYBOARD_INTERFACE(_name) \
	const pc1512_keyboard_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc1512_keyboard_interface

struct pc1512_keyboard_interface
{
	devcb_write_line	m_out_data_cb;
	devcb_write_line	m_out_clock_cb;
};


// ======================> pc1512_keyboard_device

class pc1512_keyboard_device :  public device_t,
								public pc1512_keyboard_interface
{
public:
    // construction/destruction
    pc1512_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( data_w );
	DECLARE_WRITE_LINE_MEMBER( clock_w );
	DECLARE_WRITE_LINE_MEMBER( m1_w );
	DECLARE_WRITE_LINE_MEMBER( m2_w );

	DECLARE_READ8_MEMBER( kb_bus_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_READ8_MEMBER( kb_p2_r );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_READ8_MEMBER( kb_t0_r );
	DECLARE_READ8_MEMBER( kb_t1_r );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
    virtual void device_config_complete();

private:
	devcb_resolved_write_line	m_out_data_func;
	devcb_resolved_write_line	m_out_clock_func;

	required_device<cpu_device> m_maincpu;

	int m_data_in;
	int m_clock_in;
	int m_kb_y;
	int m_joy_com;
	int m_m1;
	int m_m2;

	emu_timer *m_reset_timer;
};


// device type definition
extern const device_type PC1512_KEYBOARD;



#endif
