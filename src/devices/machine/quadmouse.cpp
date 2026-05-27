// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/**********************************************************************

    Generic quadrature mouse axis emulation

    Note: Axis only, no buttons, do the buttons in the owning device

**********************************************************************/

#include "emu.h"
#include "quadmouse.h"

DEFINE_DEVICE_TYPE(QUADMOUSE, quadmouse_device, "quadmouse", "Generic quadrature mouse support")

static INPUT_PORTS_START(quadmouse)
	PORT_START("x")
	PORT_BIT(0xf000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0fff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(quadmouse_device::x_changed), 0)

	PORT_START("y")
	PORT_BIT(0xf000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0fff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(quadmouse_device::y_changed), 0)
INPUT_PORTS_END

quadmouse_device::quadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QUADMOUSE, tag, owner, clock),
	m_port_x(*this, "x"),
	m_port_y(*this, "y"),
	m_up_cb(*this),
	m_down_cb(*this),
	m_left_cb(*this),
	m_right_cb(*this)
{
}

void quadmouse_device::device_start()
{
	m_x_timer = timer_alloc(FUNC(quadmouse_device::x_tick), this);
	m_y_timer = timer_alloc(FUNC(quadmouse_device::y_tick), this);

	save_item(NAME(m_up));
	save_item(NAME(m_down));
	save_item(NAME(m_left));
	save_item(NAME(m_right));
	save_item(NAME(m_x_time));
	save_item(NAME(m_y_time));
	save_item(NAME(m_x_delta));
	save_item(NAME(m_y_delta));
}

void quadmouse_device::device_reset()
{
	m_x_time = m_y_time = machine().time();

	m_up = m_down = m_left = m_right = false;
	m_x_delta = m_y_delta = 0;
}

ioport_constructor quadmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(quadmouse);
}

void quadmouse_device::changed(s32 oldval, s32 newval, s32 &delta, attotime &time, emu_timer *timer)
{
	s32 ldelta = (newval - oldval) & 0xfff;
	if(ldelta & 0x800)
		ldelta -= 0x1000;
	attotime ctime = machine().time();
	attotime tdelta = ctime - time;
	delta += ldelta;
	time = ctime;

	if(delta) {
		int steps = delta > 0 ? delta : -delta;
		attotime step = tdelta / (steps+1);
		timer->adjust(step/2, 0, step);
	} else
		timer->adjust(attotime::never);
}

INPUT_CHANGED_MEMBER(quadmouse_device::x_changed)
{
	changed(oldval, newval, m_x_delta, m_x_time, m_x_timer);
}

INPUT_CHANGED_MEMBER(quadmouse_device::y_changed)
{
	changed(oldval, newval, m_y_delta, m_y_time, m_y_timer);
}

void quadmouse_device::step(s32 &delta, bool &mn, bool &pl, devcb_write_line &mn_cb, devcb_write_line &pl_cb)
{
	if(delta > 0) {
		delta --;

		if(mn == pl)
			mn_cb(mn = !mn);
		else
			pl_cb(pl = !pl);

	} else if(delta < 0) {
		delta ++;

		if(mn == pl)
			pl_cb(pl = !pl);
		else
			mn_cb(mn = !mn);
	}
}

TIMER_CALLBACK_MEMBER(quadmouse_device::x_tick)
{
	step(m_x_delta, m_left, m_right, m_left_cb, m_right_cb);

	if(!m_x_delta)
		m_x_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(quadmouse_device::y_tick)
{
	step(m_y_delta, m_up, m_down, m_up_cb, m_down_cb);

	if(!m_y_delta)
		m_y_timer->adjust(attotime::never);
}

