// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson TO8 built-in keyboard & TO9 detached keyboard

**********************************************************************/

#ifndef MAME_THOMSON_TO_KBD_H
#define MAME_THOMSON_TO_KBD_H

#pragma once

class to8_keyboard_device : public device_t
{
public:
	to8_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	auto data_cb() { return m_data_cb.bind(); }

	int ktest_r();
	int caps_r() { return m_kbd_caps; }
	void set_ack( int data );

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_data_cb;

	required_ioport_array<10> m_io_keyboard;
	output_finder<> m_caps_led;

	uint8_t  m_kbd_ack = 0;       /* 1 = cpu inits / accepts transfers */
	uint16_t m_kbd_data = 0;      /* data to transmit */
	uint16_t m_kbd_step = 0;      /* transmission automaton state */
	uint8_t  m_kbd_last_key = 0;  /* last key (for repetition) */
	uint32_t m_kbd_key_count = 0; /* keypress time (for repetition)  */
	uint8_t  m_kbd_caps = 0;      /* caps lock */
	emu_timer* m_kbd_timer = nullptr;   /* bit-send */
	emu_timer* m_kbd_signal = nullptr;  /* signal from CPU */

	TIMER_CALLBACK_MEMBER( timer_cb );
	int get_key();
	void timer_func();
};

class to9_keyboard_device : public device_t
{
public:
	to9_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	auto irq_cb() { return m_irq_cb.bind(); }

	uint8_t kbd_acia_r(offs_t offset);
	void kbd_acia_w(offs_t offset, uint8_t data);

	int ktest_r();

protected:
	to9_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0U);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_irq_cb;

	required_ioport_array<10> m_io_keyboard;
	optional_ioport m_io_mouse_x;
	optional_ioport m_io_mouse_y;
	optional_ioport m_io_mouse_button;
	output_finder<> m_caps_led;

	uint8_t  m_kbd_parity = 0;  /* 0=even, 1=odd, 2=no parity */
	uint8_t  m_kbd_intr = 0;    /* interrupt mode */
	uint8_t  m_kbd_in = 0;      /* data from keyboard */
	uint8_t  m_kbd_status = 0;  /* status */
	uint8_t  m_kbd_overrun = 0; /* character lost */
	uint8_t  m_kbd_periph = 0;     /* peripheral mode */
	uint8_t  m_kbd_byte_count = 0; /* byte-count in peripheral mode */
	uint16_t m_mouse_x = 0;
	uint16_t m_mouse_y = 0;
	uint8_t  m_kbd_last_key = 0;  /* for key repetition */
	uint16_t m_kbd_key_count = 0;
	uint8_t  m_kbd_caps = 0;  /* caps-lock */
	uint8_t  m_kbd_pad = 0;   /* keypad outputs special codes */
	emu_timer* m_kbd_timer = nullptr;

	TIMER_CALLBACK_MEMBER( timer_cb );
	void update_irq();
	void send( uint8_t data, int parity );
	int get_key();
};

class to9p_keyboard_device : public to9_keyboard_device
{
public:
	to9p_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

// device type declarations
DECLARE_DEVICE_TYPE(TO8_KEYBOARD, to8_keyboard_device)
DECLARE_DEVICE_TYPE(TO9_KEYBOARD, to9_keyboard_device)
DECLARE_DEVICE_TYPE(TO9P_KEYBOARD, to9p_keyboard_device)

#endif // MAME_THOMSON_TO_KBD_H
