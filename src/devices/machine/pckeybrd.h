// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Kevin Thacker
/**********************************************************************

    pckeybrd.h

    PC-style keyboard emulation

    This emulation is decoupled from the AT 8042 emulation used in the
    IBM ATs and above

**********************************************************************/

#ifndef PCKEYBRD_H
#define PCKEYBRD_H

#include "emu.h"

class pc_keyboard_device : public device_t
{
public:
	pc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pc_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE_LINE_MEMBER(enable);

	template<class _Object> static devcb_base &static_set_keypress_callback(device_t &device, _Object object)
		{ return downcast<pc_keyboard_device &>(device).m_out_keypress_func.set_callback(object); }

	enum KEYBOARD_TYPE
	{
		KEYBOARD_TYPE_PC,
		KEYBOARD_TYPE_AT,
		KEYBOARD_TYPE_MF2
	};


protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void queue_insert(UINT8 data);
	void clear_buffer(void);

	int m_numlock;
	KEYBOARD_TYPE m_type;

private:
	void polling(void);
	UINT32 readport(int port);
	UINT8 unicode_char_to_at_keycode(unicode_char ch);

	virtual void standard_scancode_insert(int our_code, int pressed);
	virtual void extended_scancode_insert(int code, int pressed) { }
	int queue_size(void);
	int queue_chars(const unicode_char *text, size_t text_len);
	bool accept_char(unicode_char ch);
	bool charqueue_empty();

	bool m_on;
	UINT8 m_delay;   /* 240/60 -> 0,25s */
	UINT8 m_repeat;   /* 240/ 8 -> 30/s */

	UINT8 m_queue[256];
	UINT8 m_head;
	UINT8 m_tail;
	UINT8 m_make[128];

	optional_ioport m_ioport_0;
	optional_ioport m_ioport_1;
	optional_ioport m_ioport_2;
	optional_ioport m_ioport_3;
	optional_ioport m_ioport_4;
	optional_ioport m_ioport_5;
	optional_ioport m_ioport_6;
	optional_ioport m_ioport_7;

	devcb_write_line m_out_keypress_func;
	emu_timer *m_keyboard_timer;
};

class at_keyboard_device : public pc_keyboard_device
{
public:
	at_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( write );

	static void static_set_type(device_t &device, KEYBOARD_TYPE type, int default_set)
		{ downcast<at_keyboard_device &>(device).m_scan_code_set = default_set; downcast<at_keyboard_device &>(device).m_type = type; }

protected:
	virtual void device_reset() override;
	virtual void device_start() override;

private:
	virtual void standard_scancode_insert(int our_code, int pressed) override;
	virtual void extended_scancode_insert(int code, int pressed) override;
	void helper(const char *codes);
	void clear_buffer_and_acknowledge(void);

	struct extended_keyboard_code
	{
		const char *pressed;
		const char *released;
	};

	static const extended_keyboard_code m_extended_codes_set_2_3[];
	static const int m_scancode_set_2_3[];
	static const extended_keyboard_code m_mf2_code[0x10][2];

	int m_scan_code_set;
	int m_input_state;
};

INPUT_PORTS_EXTERN( pc_keyboard );
INPUT_PORTS_EXTERN( at_keyboard );

extern const device_type PC_KEYB;
extern const device_type AT_KEYB;

#define MCFG_PC_KEYB_ADD(_tag, _cb) \
	MCFG_DEVICE_ADD(_tag, PC_KEYB, 0) \
	devcb = &pc_keyboard_device::static_set_keypress_callback(*device, DEVCB_##_cb);

#define MCFG_AT_KEYB_ADD(_tag, _def_set, _cb) \
	MCFG_DEVICE_ADD(_tag, AT_KEYB, 0) \
	at_keyboard_device::static_set_type(*device, pc_keyboard_device::KEYBOARD_TYPE_AT, _def_set); \
	devcb = &pc_keyboard_device::static_set_keypress_callback(*device, DEVCB_##_cb);

#define MCFG_AT_MF2_KEYB_ADD(_tag, _def_set, _cb) \
	MCFG_DEVICE_ADD(_tag, AT_KEYB, 0) \
	at_keyboard_device::static_set_type(*device, pc_keyboard_device::KEYBOARD_TYPE_MF2, _def_set); \
	devcb = &pc_keyboard_device::static_set_keypress_callback(*device, DEVCB_##_cb);

#endif /* PCKEYBRD_H */
