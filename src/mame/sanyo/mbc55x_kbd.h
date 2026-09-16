// license:BSD-3-Clause
// copyright-holders:AJR

/*******************************************************************************

    Serial keyboard emulation for Sanyo MBC-55x.

*******************************************************************************/

#ifndef MAME_SANYO_MBC55X_KBD_H
#define MAME_SANYO_MBC55X_KBD_H

#pragma once

#include "diserial.h"
#include "machine/keyboard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mbc55x_keyboard_device : public device_t, public device_matrix_keyboard_interface<12U>, public device_serial_interface
{
	static const u8 s_code_table[2][12][8];

public:
	// construction/destruction
	mbc55x_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto txd_callback() { return m_txd_callback.bind(); }

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_matrix_keyboard_interface implementation
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;
	virtual void scan_complete() override;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	void send_key(u8 code, bool ctrl_active);
	void send_translated(u8 row, u8 column);

	// input ports
	required_ioport m_modifiers;

	// output callback
	devcb_write_line m_txd_callback;
};

DECLARE_DEVICE_TYPE(MBC55X_KEYBOARD, mbc55x_keyboard_device)

#endif // MAME_SANYO_MBC55X_KBD_H
