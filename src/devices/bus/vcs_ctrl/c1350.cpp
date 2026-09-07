// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1350/1351 mouse emulation

    Both mice contain a pair of quadrature encoders, one per axis, and
    a MOS 5717 mouse controller.

    The 1350 is a mouse in name only: the controller turns the encoder
    steps into joystick direction switch closures, so the mouse works
    with any joystick driven software.

    The 1351 is a proportional mouse. The controller keeps a counter per
    axis and copies it modulo 64 to the SID POTX/POTY inputs every 512
    microseconds. Holding the right button at power-on selects a
    1350 compatible joystick mode instead.

    Left button is wired to pin 6 (fire) and right button to pin 1 (up)
    in both modes.

**********************************************************************/

#include "emu.h"
#include "c1350.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// how long a single encoder step keeps its direction switch closed
static constexpr int PULSE_WIDTH_MS = 20;

// quadrature state (mn << 1) | pl, indexed [previous][current]
static const int8_t QUADRATURE_DELTA[4][4] =
{
	{  0, -1,  1,  0 },
	{  1,  0,  0, -1 },
	{ -1,  0,  0,  1 },
	{  0,  1, -1,  0 }
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1350, c1350_device, "c1350", "Commodore 1350 Mouse")
DEFINE_DEVICE_TYPE(C1351, c1351_device, "c1351", "Commodore 1351 Mouse")


static INPUT_PORTS_START( c1350 )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_WRITE_LINE_MEMBER(FUNC(c1350_device_base::trigger_w))
	PORT_BIT( 0xde, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER("encx", FUNC(quadencoder_device::changed), 0)

	PORT_START("Y")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_REVERSE PORT_CHANGED_MEMBER("ency", FUNC(quadencoder_device::changed), 0)
INPUT_PORTS_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1350_device_base - constructor
//-------------------------------------------------

c1350_device_base::c1350_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_count{ 0, 0 },
	m_encoder(*this, { "encx", "ency" }),
	m_pulse_timer{ nullptr, nullptr },
	m_phase{ 0, 0 },
	m_direction{ 0, 0 }
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration
//-------------------------------------------------

void c1350_device_base::device_add_mconfig(machine_config &config)
{
	QUADENCODER(config, m_encoder[0]);
	m_encoder[0]->write_mn().set([this] (int state) { encoder_w(0, 0, state); });
	m_encoder[0]->write_pl().set([this] (int state) { encoder_w(0, 1, state); });

	QUADENCODER(config, m_encoder[1]);
	m_encoder[1]->write_mn().set([this] (int state) { encoder_w(1, 0, state); });
	m_encoder[1]->write_pl().set([this] (int state) { encoder_w(1, 1, state); });
}


//-------------------------------------------------
//  device_input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1350_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1350 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1350_device_base::device_start()
{
	for (int axis = 0; axis < 2; axis++)
		m_pulse_timer[axis] = timer_alloc(FUNC(c1350_device_base::pulse_expired), this);

	save_item(NAME(m_count));
	save_item(NAME(m_phase));
	save_item(NAME(m_direction));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1350_device_base::device_reset()
{
	for (int axis = 0; axis < 2; axis++)
	{
		m_count[axis] = 0;
		m_phase[axis] = 0;
		m_direction[axis] = 0;
		m_pulse_timer[axis]->adjust(attotime::never);
	}
}


//-------------------------------------------------
//  encoder_w - quadrature encoder phase change
//-------------------------------------------------

void c1350_device_base::encoder_w(int axis, int phase, int state)
{
	const uint8_t previous = m_phase[axis];
	const uint8_t current = phase ?
			((previous & 0x02) | (state ? 0x01 : 0x00)) :
			((previous & 0x01) | (state ? 0x02 : 0x00));

	if (current == previous)
		return;

	m_phase[axis] = current;

	const int8_t delta = QUADRATURE_DELTA[previous][current];

	if (!delta)
		return;

	m_count[axis] += delta;

	m_direction[axis] = delta;
	m_pulse_timer[axis]->adjust(attotime::from_msec(PULSE_WIDTH_MS), axis);
}


//-------------------------------------------------
//  pulse_expired - release a direction switch
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( c1350_device_base::pulse_expired )
{
	m_direction[param] = 0;
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t c1350_device_base::vcs_joy_r()
{
	uint8_t data = m_joy->read();

	if (m_direction[1] > 0) data &= ~0x01; // up
	if (m_direction[1] < 0) data &= ~0x02; // down
	if (m_direction[0] < 0) data &= ~0x04; // left
	if (m_direction[0] > 0) data &= ~0x08; // right

	return data;
}


//-------------------------------------------------
//  c1350_device - constructor
//-------------------------------------------------

c1350_device::c1350_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1350_device_base(mconfig, C1350, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1351_device - constructor
//-------------------------------------------------

c1351_device::c1351_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1350_device_base(mconfig, C1351, tag, owner, clock),
	m_joystick_mode(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1351_device::device_start()
{
	c1350_device_base::device_start();

	save_item(NAME(m_joystick_mode));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1351_device::device_reset()
{
	c1350_device_base::device_reset();

	// holding the right button at power-on selects joystick mode
	m_joystick_mode = !BIT(m_joy->read(), 0);
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t c1351_device::vcs_joy_r()
{
	return m_joystick_mode ? c1350_device_base::vcs_joy_r() : m_joy->read();
}


//-------------------------------------------------
//  vcs_pot_x_r - potentiometer X read
//-------------------------------------------------

uint8_t c1351_device::vcs_pot_x_r()
{
	return m_joystick_mode ? 0xff : ((m_count[0] & 0x3f) << 1);
}


//-------------------------------------------------
//  vcs_pot_y_r - potentiometer Y read
//-------------------------------------------------

uint8_t c1351_device::vcs_pot_y_r()
{
	return m_joystick_mode ? 0xff : ((m_count[1] & 0x3f) << 1);
}
