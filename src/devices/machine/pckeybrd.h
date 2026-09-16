// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Kevin Thacker
/**********************************************************************

    pckeybrd.h

    PC-style keyboard emulation

    This emulation is decoupled from the AT 8042 emulation used in the
    IBM ATs and above

**********************************************************************/

#ifndef MAME_MACHINE_PCKEYBRD_H
#define MAME_MACHINE_PCKEYBRD_H

#pragma once


class pc_keyboard_device : public device_t
{
public:
	pc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read();
	void enable(int state);

	auto keypress() { return m_out_keypress_func.bind(); }

	enum class KEYBOARD_TYPE
	{
		PC,
		AT,
		MF2
	};

protected:
	pc_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(poll_keys);

	void queue_insert(uint8_t data);
	void clear_buffer();

	int m_numlock;
	KEYBOARD_TYPE m_type;

private:
	void polling();
	uint32_t readport(int port);
	uint8_t unicode_char_to_at_keycode(char32_t ch);

	virtual void standard_scancode_insert(int our_code, int pressed);
	virtual void extended_scancode_insert(int code, int pressed) { }
	int queue_size();
	int queue_chars(const char32_t *text, size_t text_len);
	bool accept_char(char32_t ch);
	bool charqueue_empty();

	bool m_on;
	uint8_t m_delay;   /* 240/60 -> 0,25s */
	uint8_t m_repeat;   /* 240/ 8 -> 30/s */

	uint8_t m_queue[256];
	uint8_t m_head;
	uint8_t m_tail;
	uint8_t m_make[128];

	optional_ioport_array<8> m_ioport;

	devcb_write_line m_out_keypress_func;
	emu_timer *m_keyboard_timer;
};

class at_keyboard_device : public pc_keyboard_device
{
public:
	at_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, KEYBOARD_TYPE type, int default_set)
		: at_keyboard_device(mconfig, tag, owner, 0)
	{
		set_type(type, default_set);
	}

	at_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write(uint8_t data);

	void set_type(KEYBOARD_TYPE type, int default_set) { m_scan_code_set = default_set; m_type = type; }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	virtual void standard_scancode_insert(int our_code, int pressed) override;
	virtual void extended_scancode_insert(int code, int pressed) override;
	void helper(const char *codes);
	void clear_buffer_and_acknowledge();

	struct extended_keyboard_code
	{
		const char *pressed;
		const char *released;
	};

	static const extended_keyboard_code m_extended_codes_set_2_3[];
	static const int m_scancode_set_2_3[];
	static const extended_keyboard_code m_mf2_code[0x10][2];

	output_finder<3> m_leds;
	int m_scan_code_set;
	int m_input_state;
};

INPUT_PORTS_EXTERN( pc_keyboard );

DECLARE_DEVICE_TYPE(PC_KEYB, pc_keyboard_device)
DECLARE_DEVICE_TYPE(AT_KEYB, at_keyboard_device)

#endif // MAME_MACHINE_PCKEYBRD_H
