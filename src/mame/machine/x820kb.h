// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xerox 820/820-II ASCII keyboard emulation

*********************************************************************/

#ifndef MAME_MACHINE_X820KB_H
#define MAME_MACHINE_X820KB_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> xerox_820_keyboard_device

class xerox_820_keyboard_device :  public device_t
{
public:
	// construction/destruction
	xerox_820_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto kbstb_wr_callback() { return m_kbstb_cb.bind(); }

	uint8_t read() { return m_bus; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<i8048_device> m_maincpu;
	required_ioport_array<16> m_y;

	devcb_write_line   m_kbstb_cb;

	uint8_t m_p1;
	uint8_t m_bus;

	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_READ8_MEMBER( kb_p2_r );
	DECLARE_READ_LINE_MEMBER( kb_t0_r );
	DECLARE_READ_LINE_MEMBER( kb_t1_r );
	DECLARE_WRITE8_MEMBER( kb_bus_w );
};


// device type definition
DECLARE_DEVICE_TYPE(XEROX_820_KEYBOARD, xerox_820_keyboard_device)


#endif // MAME_MACHINE_X820KB_H
