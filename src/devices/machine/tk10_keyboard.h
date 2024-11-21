// license:BSD-3-Clause
// copyright-holders: Golden Child

#ifndef MAME_TK10_KEYBOARD_H
#define MAME_TK10_KEYBOARD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tk10_keyboard_device

class tk10_keyboard_device : public device_t
{
public:
	// device type constructor
	tk10_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	INPUT_CHANGED_MEMBER( tk10_func );
	INPUT_CHANGED_MEMBER( tk10_reset );

	auto reset_write_cb() { return m_reset_write.bind(); }
	auto data_write_cb() { return m_data_write.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	devcb_write8 m_data_write;
	devcb_write_line m_reset_write;

private:
	void tk10_mem_map(address_map &map);

	required_device<i8039_device> m_tk10cpu;
	required_ioport_array<8> m_keyboard;
	required_ioport m_kbspecial;

};

// device type declaration
DECLARE_DEVICE_TYPE(TK10_KEYBOARD, tk10_keyboard_device)

#endif // MAME_TK10_KEYBOARD_H
