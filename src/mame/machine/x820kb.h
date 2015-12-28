// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xerox 820/820-II ASCII keyboard emulation

*********************************************************************/

#pragma once

#ifndef __XEROX_820_KEYBOARD__
#define __XEROX_820_KEYBOARD__

#include "emu.h"
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
	xerox_820_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_kbstb_wr_callback(device_t &device, _Object object) { return downcast<xerox_820_keyboard_t &>(device).m_kbstb_cb.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	UINT8 read() { return m_bus; }

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
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_yf;

	devcb_write_line   m_kbstb_cb;

	UINT8 m_p1;
	UINT8 m_bus;
};


// device type definition
extern const device_type XEROX_820_KEYBOARD;



#endif
