// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_WY50KB_H
#define MAME_MACHINE_WY50KB_H

#pragma once

class wy50_keyboard_device : public device_t
{
public:
	wy50_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void scan_w(u8 address);
	DECLARE_READ_LINE_MEMBER(sense_r);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	optional_ioport_array<16> m_key_matrix;

	u8 m_address;
};

DECLARE_DEVICE_TYPE(WY50_KEYBOARD, wy50_keyboard_device)

#endif // MAME_MACHINE_WY50KB_H
