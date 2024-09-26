// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_WYSE_WY50KB_H
#define MAME_WYSE_WY50KB_H

#pragma once

class wyse_parallel_keyboard_device : public device_t
{
public:
	void scan_w(u8 address);
	int sense_r();

protected:
	wyse_parallel_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner);

	virtual void device_start() override ATTR_COLD;

private:
	optional_ioport_array<16> m_key_matrix;

	u8 m_address;
};

class wy50_keyboard_device : public wyse_parallel_keyboard_device
{
public:
	wy50_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class wy100_keyboard_device : public wyse_parallel_keyboard_device
{
public:
	wy100_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(WY50_KEYBOARD, wy50_keyboard_device)
DECLARE_DEVICE_TYPE(WY100_KEYBOARD, wy100_keyboard_device)

#endif // MAME_WYSE_WY50KB_H
