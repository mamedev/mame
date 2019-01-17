// license:BSD-3-Clause
// copyright-holders:Fredrik Öhrström
/**********************************************************************

    myb3k_kbd.h

    Matsushita My Brain 3000 -- Panasonic JB-3000 -- Ericsson Step/One
    keyboard emulation.

    The keyboard 8048 ROM is not currently available, therefore it
    does not emulate the serial communication to the host computer.

    Instead the full byte is sent to the callback. The callback
    is responsible for storing the byte into the serial/parallell converter
    (that can be read through IN from port 0x04) and then trigger an interrupt.

**********************************************************************/

#ifndef MAME_MACHINE_MYB3K_KBD_H
#define MAME_MACHINE_MYB3K_KBD_H

#pragma once

DECLARE_DEVICE_TYPE(MYB3K_KEYBOARD, myb3k_keyboard_device)
DECLARE_DEVICE_TYPE(JB3000_KEYBOARD, jb3000_keyboard_device)
DECLARE_DEVICE_TYPE(STEPONE_KEYBOARD, stepone_keyboard_device)

class myb3k_keyboard_device : public device_t
{
public:
	typedef device_delegate<void (u8)> output_delegate;

	myb3k_keyboard_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock);

	enum
	{
		TIMER_ID_SCAN_KEYS,
		TIMER_ID_FIRST_BYTE,
		TIMER_ID_SECOND_BYTE
	};

	template <class FunctionClass>
	void set_keyboard_callback(void (FunctionClass::*callback)(u8 character), const char *name)
	{
		set_keyboard_callback(output_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}
	// FIXME: this should be aware of current device for resolving the tag
	template <class FunctionClass>
	void set_keyboard_callback(const char *devname, void (FunctionClass::*callback)(u8 character), const char *name)
	{
		set_keyboard_callback(output_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	void set_keyboard_callback(output_delegate callback) { m_keyboard_cb = callback; }

protected:
	myb3k_keyboard_device(
			const machine_config &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void send_byte(u8 code);
	void key_changed(int x, int y, bool down);
	void scan_keys();
	void update_modifiers(int y, bool down);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	output_delegate             m_keyboard_cb;
	required_ioport_array<12>   m_io_kbd_t;
	u8                          m_io_kbd_state[12][8];

	int m_x, m_y;
	u8 m_first_byte;
	u8 m_second_byte;
	u8 m_modifier_keys;
};

class jb3000_keyboard_device : public myb3k_keyboard_device
{
public:
	 jb3000_keyboard_device(
		 const machine_config &mconfig,
		 char const *tag,
		 device_t *owner,
		 u32 clock);
private:
	virtual ioport_constructor device_input_ports() const override;
};

class stepone_keyboard_device : public myb3k_keyboard_device
{
public:
	 stepone_keyboard_device(
		 const machine_config &mconfig,
		 char const *tag,
		 device_t *owner,
		 u32 clock);
private:
	virtual ioport_constructor device_input_ports() const override;
};

#endif // MAME_MACHINE_MYB3K_KBD_H
