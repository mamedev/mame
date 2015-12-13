// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-80 keyboard emulation

**********************************************************************/

#pragma once

#ifndef __ABC80_KEYBOARD__
#define __ABC80_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC80_KEYBOARD_TAG  "abc80kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC80_KEYBOARD_KEYDOWN_CALLBACK(_write) \
	devcb = &abc80_keyboard_device::set_keydown_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_keyboard_device

class abc80_keyboard_device :  public device_t
{
public:
	// construction/destruction
	abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_keydown_wr_callback(device_t &device, _Object object) { return downcast<abc80_keyboard_device &>(device).m_write_keydown.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	UINT8 data_r();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_write_keydown;

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ABC80_KEYBOARD;



#endif
