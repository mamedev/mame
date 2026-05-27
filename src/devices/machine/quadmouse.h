// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/**********************************************************************

    Generic quadrature mouse axis emulation

    Note: Axis only, no buttons, do the buttons in the owning device

**********************************************************************/

#ifndef MAME_MACHINE_QUADMOUSE_H
#define MAME_MACHINE_QUADMOUSE_H

#pragma once

class quadmouse_device : public device_t
{
public:
	// construction/destruction
	quadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto write_up()    { return m_up_cb.bind();    }
	auto write_down()  { return m_down_cb.bind();  }
	auto write_left()  { return m_left_cb.bind();  }
	auto write_right() { return m_right_cb.bind(); }

	bool up_r()     { return m_up; }
	bool down_r()   { return m_down; }
	bool left_r()   { return m_left; }
	bool right_r()  { return m_right; }

	DECLARE_INPUT_CHANGED_MEMBER(x_changed);
	DECLARE_INPUT_CHANGED_MEMBER(y_changed);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_port_x, m_port_y;

	devcb_write_line m_up_cb, m_down_cb, m_left_cb, m_right_cb;
	emu_timer *m_x_timer, *m_y_timer;
	attotime m_x_time, m_y_time;
	s32 m_x_delta, m_y_delta;
	bool m_down, m_up, m_left, m_right;

	TIMER_CALLBACK_MEMBER(x_tick);
	TIMER_CALLBACK_MEMBER(y_tick);

	void changed(s32 oldval, s32 newval, s32 &delta, attotime &time, emu_timer *timer);
	void step(s32 &delta, bool &mn, bool &pl, devcb_write_line &mn_cb, devcb_write_line &pl_cb);
};

DECLARE_DEVICE_TYPE(QUADMOUSE, quadmouse_device)

#endif // MAME_MACHINE_QUADMOUSE_H
