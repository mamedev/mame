// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xerox 820/820-II ASCII keyboard emulation

*********************************************************************/

#pragma once

#ifndef __XEROX_820_KEYBOARD__
#define __XEROX_820_KEYBOARD__

#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_XEROX_820_KEYBOARD_KBSTB_CALLBACK(_devcb) \
	devcb = &xerox_820_keyboard_t::set_kbstb_wr_callback(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xerox_820_keyboard_t

class xerox_820_keyboard_t :  public device_t
{
public:
	// construction/destruction
	xerox_820_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_kbstb_wr_callback(device_t &device, _Object object) { return downcast<xerox_820_keyboard_t &>(device).m_kbstb_cb.set_callback(object); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t read() { return m_bus; }

	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_READ8_MEMBER( kb_p2_r );
	DECLARE_READ8_MEMBER( kb_t0_r );
	DECLARE_READ8_MEMBER( kb_t1_r );
	DECLARE_WRITE8_MEMBER( kb_bus_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<16> m_y;

	devcb_write_line   m_kbstb_cb;

	uint8_t m_p1;
	uint8_t m_bus;
};


// device type definition
extern const device_type XEROX_820_KEYBOARD;



#endif
