// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_IBM_IBM5100_KBD_H
#define MAME_IBM_IBM5100_KBD_H

#pragma once

#include "machine/keyboard.h"

class ibm5100_keyboard_device
	: public device_t
	, protected device_matrix_keyboard_interface<18U>
{
public:
	auto strobe() { return m_strobe.bind(); }

	ibm5100_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	u8 read() { return m_scan; }
	void typamatic_w(int state);
	void lock_w(int state);

protected:
	ibm5100_keyboard_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_matrix_keyboard_interface implementation
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;

	virtual u8 translate(u8 column, u8 row, u8 modifiers) const;

private:
	devcb_write_line m_strobe;

	required_ioport m_modifiers;

	u8 m_scan;
	bool m_typamatic;
	bool m_lock;
};

class ibm5110_keyboard_device
	: public ibm5100_keyboard_device
{
public:
	ibm5110_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

protected:
	virtual u8 translate(u8 column, u8 row, u8 modifiers) const override;
};

DECLARE_DEVICE_TYPE(IBM5100_KEYBOARD, ibm5100_keyboard_device)
DECLARE_DEVICE_TYPE(IBM5110_KEYBOARD, ibm5110_keyboard_device)

#endif // MAME_IBM_IBM5100_KBD_H
