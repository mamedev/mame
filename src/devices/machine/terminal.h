// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_MACHINE_TERMINAL_H
#define MAME_MACHINE_TERMINAL_H

#pragma once

#include "machine/keyboard.h"
#include "sound/beep.h"


#define TERMINAL_SCREEN_TAG "terminal_screen"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

INPUT_PORTS_EXTERN( generic_terminal );

class generic_terminal_device : public device_t
{
public:
	generic_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T>
	void set_keyboard_callback(T &&... args)
	{
		m_keyboard_cb.set(std::forward<T>(args)...);
	}

	void write(u8 data) { term_write(data); }

	void kbd_put(u8 data);

protected:
	generic_terminal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, unsigned w, unsigned h);

	virtual void term_write(uint8_t data);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void send_key(uint8_t code) { m_keyboard_cb(code); }

	TIMER_CALLBACK_MEMBER(bell_off);

	required_ioport m_io_term_conf;

	static constexpr unsigned TERMINAL_WIDTH = 80;
	static constexpr unsigned TERMINAL_HEIGHT = 24;

	unsigned const m_width;
	unsigned const m_height;
	std::unique_ptr<uint8_t []> m_buffer;
	uint8_t m_x_pos;

private:
	void scroll_line();
	void write_char(uint8_t data);
	void clear();
	uint32_t update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t m_framecnt;
	uint8_t m_y_pos;

	emu_timer *m_bell_timer;
	required_device<beep_device> m_beeper;
	generic_keyboard_device::output_delegate m_keyboard_cb;
};

DECLARE_DEVICE_TYPE(GENERIC_TERMINAL, generic_terminal_device)

#endif // MAME_DEVICES_TERMINAL_H
