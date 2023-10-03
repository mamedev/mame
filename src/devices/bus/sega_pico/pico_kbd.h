// license:BSD-3-Clause
// copyright-holders:QUFB

#ifndef MAME_BUS_PICO_KBD_H
#define MAME_BUS_PICO_KBD_H

#pragma once

#include "pico_ps2.h"

class pico_kbd_device : public device_t, public device_pico_ps2_slot_interface
{
public:
	pico_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_pico_ps2_slot_interface overrides
	virtual uint8_t ps2_r(offs_t offset) override;
	virtual void ps2_w(offs_t offset, uint8_t data) override;

	static inline constexpr uint8_t PICO_KEYCODE_CAPSLOCK = 0x58;
	static inline constexpr uint8_t PICO_KEYCODE_LSHIFT = 0x12;

private:
	enum key_state : uint8_t
	{
		KEY_RELEASED = 0,
		KEY_DOWN,
		KEY_UP,
	};
	enum shift_state : uint8_t
	{
		SHIFT_RELEASED = 0,
		SHIFT_DOWN,
		SHIFT_UP_HELD_DOWN,
		SHIFT_RELEASED_HELD_DOWN,
		SHIFT_UP
	};

	required_ioport_array<4> m_io_keys;

	uint8_t m_caps_lock;
	bool m_has_caps_lock;
	bool m_has_read;
	uint8_t m_i;
	uint8_t m_key_state;
	bool m_is_negative;
	uint8_t m_shift_state;
	osd_ticks_t m_start_time_keydown;
	osd_ticks_t m_time_keydown;

	uint16_t parse_keycode();
};

DECLARE_DEVICE_TYPE(PICO_KBD, pico_kbd_device)

#endif // MAME_BUS_PICO_KBD_H;
