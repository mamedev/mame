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

#define VICTOR9K_KEYBOARD_TAG   "victor9kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VICTOR9K_KEYBOARD_ADD(_kbrdy) \
	MCFG_DEVICE_ADD(VICTOR9K_KEYBOARD_TAG, VICTOR9K_KEYBOARD, 0) \
	downcast<victor9k_keyboard_device *>(device)->set_kbrdy_callback(DEVCB2_##_kbrdy);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> victor9k_keyboard_device

class victor9k_keyboard_device :  public device_t
{
public:
	// construction/destruction
	victor9k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _kbrdy> void set_kbrdy_callback(_kbrdy kbrdy) { m_write_kbrdy.set_callback(kbrdy); }

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

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_ya;
	required_ioport m_yb;
	required_ioport m_yc;

	devcb2_write_line   m_write_kbrdy;

	UINT8 m_y;
	int m_kbrdy;
	int m_kbdata;
	int m_kback;
};


// device type definition
extern const device_type VICTOR9K_KEYBOARD;



#endif
