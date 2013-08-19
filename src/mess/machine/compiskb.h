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

#define COMPIS_KEYBOARD_TAG "compiskb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COMPIS_KEYBOARD_ADD(_irq) \
	MCFG_DEVICE_ADD(COMPIS_KEYBOARD_TAG, COMPIS_KEYBOARD, 0) \
	downcast<compis_keyboard_device *>(device)->set_irq_callback(DEVCB2_##_irq);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_keyboard_device

class compis_keyboard_device :  public device_t
{
public:
	// construction/destruction
	compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _irq> void set_irq_callback(_irq irq) { m_write_irq.set_callback(irq); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER( dtr_w );
	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_READ_LINE_MEMBER( so_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	required_device<cpu_device> m_maincpu;

	devcb2_write_line   m_write_irq;

	int m_so;
};


// device type definition
extern const device_type COMPIS_KEYBOARD;



#endif
