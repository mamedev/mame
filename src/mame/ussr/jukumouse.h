// license: BSD-3-Clause
// copyright-holders:Märt Põder

#ifndef MAME_USSR_JUKUMOUSE_H
#define MAME_USSR_JUKUMOUSE_H

#pragma once

class juku_mouse_device : public device_t
{
public:
	juku_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto int_handler() { return m_int_handler.bind(); }

	uint8_t mouse_port_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER( poll_delta );

private:
	required_ioport m_mouse_x, m_mouse_y, m_mouse_b;
	uint8_t m_prev_mouse_x, m_prev_mouse_y;
	uint8_t m_prev_byte;
	devcb_write_line m_int_handler;
	emu_timer *m_mouse_timer;
};

DECLARE_DEVICE_TYPE(JUKU_MOUSE, juku_mouse_device)

#endif // MAME_USSR_JUKUMOUSE_H
