// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/**********************************************************************

    Generic quadrature mouse axis emulation

    Note: Axis only, no buttons, do the buttons in the owning device

**********************************************************************/

#include "emu.h"
#include "quadmouse.h"

DEFINE_DEVICE_TYPE(QUADENCODER, quadencoder_device, "quadencoder", "Generic quadature encoder support")
DEFINE_DEVICE_TYPE(QUADMOUSE, quadmouse_device, "quadmouse", "Generic quadrature mouse support")

static INPUT_PORTS_START(quadmouse)
	PORT_START("x")
	PORT_BIT(0xf000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0fff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER("encoder_x", FUNC(quadencoder_device::changed), 0)

	PORT_START("y")
	PORT_BIT(0xf000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0fff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER("encoder_y", FUNC(quadencoder_device::changed), 0)
INPUT_PORTS_END


quadencoder_device::quadencoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QUADENCODER, tag, owner, clock),
	m_mn_cb(*this),
	m_pl_cb(*this)
{
}

void quadencoder_device::device_start()
{
	m_timer = timer_alloc(FUNC(quadencoder_device::tick), this);

	save_item(NAME(m_mn));
	save_item(NAME(m_pl));
	save_item(NAME(m_time));
	save_item(NAME(m_delta));
}

void quadencoder_device::device_reset()
{
	m_time = machine().time();

	m_mn = m_pl = false;
	m_delta = 0;
}

DECLARE_INPUT_CHANGED_MEMBER(quadencoder_device::changed)
{
	assert(field.minval() == 0);

	// Given the old and new values, compute the magnitude of change for both
	// scenarios: an increase and a decrease, taking into account wrapping.
	s32 delta_inc, delta_dec;  // Magnitudes. Always positive.
	if(newval > oldval) {
		delta_inc = newval - oldval;
		delta_dec = oldval + field.maxval() + 1 - newval;
	} else {
		delta_inc = field.maxval() + 1 - oldval + newval;
		delta_dec = oldval - newval;
	}

	// Pick the direction with the smallest magnitude as the correct one.
	const s32 ldelta = (delta_inc < delta_dec) ? delta_inc : -delta_dec;

	attotime ctime = machine().time();
	attotime tdelta = ctime - m_time;
	m_delta += ldelta;
	m_time = ctime;

	if(m_delta) {
		int steps = m_delta > 0 ? m_delta : -m_delta;
		attotime step = tdelta / (steps+1);
		m_timer->adjust(step/2, 0, step);
	} else
		m_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(quadencoder_device::tick)
{
	if(m_delta > 0) {
		m_delta --;

		if(m_mn == m_pl)
			m_mn_cb(m_mn = !m_mn);
		else
			m_pl_cb(m_pl = !m_pl);

	} else if(m_delta < 0) {
		m_delta ++;

		if(m_mn == m_pl)
			m_pl_cb(m_pl = !m_pl);
		else
			m_mn_cb(m_mn = !m_mn);
	}

	if(!m_delta)
		m_timer->adjust(attotime::never);
}

quadmouse_device::quadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QUADMOUSE, tag, owner, clock),
	m_enc_x(*this, "encoder_x"),
	m_enc_y(*this, "encoder_y"),
	m_port_x(*this, "x"),
	m_port_y(*this, "y"),
	m_up_cb(*this),
	m_down_cb(*this),
	m_left_cb(*this),
	m_right_cb(*this)
{
}

void quadmouse_device::device_add_mconfig(machine_config &config)
{
	QUADENCODER(config, m_enc_x);
	m_enc_x->write_mn().set([this] (int state) { m_left_cb(state); });
	m_enc_x->write_pl().set([this] (int state) { m_right_cb(state); });

	QUADENCODER(config, m_enc_y);
	m_enc_y->write_mn().set([this] (int state) { m_up_cb(state); });
	m_enc_y->write_pl().set([this] (int state) { m_down_cb(state); });
}

void quadmouse_device::device_start()
{
}

ioport_constructor quadmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(quadmouse);
}

