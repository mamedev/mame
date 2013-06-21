/**********************************************************************

    IBM PC/XT 5150/5160 83-key keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __PCXT83_KEYBOARD__
#define __PCXT83_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/pc_kbdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ibm_pc_xt_83_keyboard_device

class ibm_pc_xt_83_keyboard_device :  public device_t,
									  public device_pc_kbd_interface
{
public:
	// construction/destruction
	ibm_pc_xt_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write );
	virtual DECLARE_WRITE_LINE_MEMBER( data_write );

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type PC_KBD_IBM_PC_XT_83;



#endif
