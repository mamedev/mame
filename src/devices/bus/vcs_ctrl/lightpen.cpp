// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64/128 lightpen emulation

	Generic light pen (up)
	Koala Soft Koala Light Pen (up)
	Anirog Microscribe Lightpen (right)
	Turbo Computer Light Pen (left)
	Inkwell Systems 170-C (left)

	Not emulated:

	Datel Pen
	Tech Sketch LP-10S
	Tech Sketch LP-15
	Trojan CAD-Master
	Trojan Penmaster
	Trojan Light Pen
	Cardco Light Pen
	Pixstik Lightpen
	McPen
	Futurehouse Edumate Light Pen
	Rex Datentechnik 9518
	Rex Datentechnik 9631
	Lindy Lightpen
	Stonechip Electronics Light Pen

    Tip switch on pin 6 (active low), also wired to a joystick direction
    (Up/Left/Right per variant) as a second button. Tip switch arms the
    sensor and idles pin 6 high; on the beam's next pass under the
    crosshair, pin 6 is pulsed low, latching the video chip's light pen
    coordinates and the trigger at the same instant.

**********************************************************************/

#include "emu.h"
#include "lightpen.h"

// the pulse-stretch stage on real sensor circuits turns a sub-microsecond
// photodiode blip into a signal wide enough for software to poll for it;
// we don't model that circuit, just its effect
static constexpr attotime SENSOR_PULSE_WIDTH = attotime::from_usec(50);



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_LIGHTPEN_UP, vcs_light_pen_up_device, "vcs_lightpen_up", "Light Pen (trigger on up)")
DEFINE_DEVICE_TYPE(VCS_LIGHTPEN_LEFT, vcs_light_pen_left_device, "vcs_lightpen_left", "Light Pen (trigger on left)")
DEFINE_DEVICE_TYPE(VCS_LIGHTPEN_RIGHT, vcs_light_pen_right_device, "vcs_lightpen_right", "Light Pen (trigger on right)")



//**************************************************************************
//  BASE DEVICE - shared tip-switch/strobe sensor
//**************************************************************************

INPUT_CHANGED_MEMBER( vcs_lightpen_device::trigger )
{
	// the tip switch is active low, so newval is 0 while it is held down
	m_armed = !newval;

	if (!has_lightpen_timing())
	{
		// no video chip to time against - fall back to strobing on the switch itself
		trigger_w(newval);
		return;
	}

	if (m_armed)
	{
		// tip switch closed: idle pin 6 high and wait for the beam
		trigger_w(1);
		m_sensor_timer->adjust(time_until_lightpen_pos(m_lightx->read(), m_lighty->read()));
	}
	else
	{
		m_sensor_timer->adjust(attotime::never);
		m_strobed = false;
		trigger_w(1);
	}
}


TIMER_CALLBACK_MEMBER( vcs_lightpen_device::sensor_tick )
{
	if (m_strobed)
	{
		// pulse done: release pin 6 and re-arm for the beam's next pass. Re-reading
		// the crosshair position here (rather than caching it from when the tip
		// switch closed) is what lets the pen retrigger at a new spot if it's been
		// moved while still held down - otherwise painting programs couldn't track
		// the pen being dragged across the screen.
		m_strobed = false;
		trigger_w(1);
		m_sensor_timer->adjust(time_until_lightpen_pos(m_lightx->read(), m_lighty->read()));
	}
	else
	{
		// The beam is under the crosshair: pull pin 6 low. This is a single wire, so
		// the video chip latches its coordinates and the system sees the trigger at
		// the same instant - software that polls for the trigger and then reads the
		// latched position gets this strobe's coordinates, not the previous click's.
		// Hold it low briefly so slow (BASIC) pollers can catch it too.
		m_strobed = true;
		trigger_w(0);
		m_sensor_timer->adjust(SENSOR_PULSE_WIDTH);
	}
}


vcs_lightpen_device::vcs_lightpen_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_lightx(*this, "LIGHTX"),
	m_lighty(*this, "LIGHTY"),
	m_sensor_timer(nullptr),
	m_armed(false),
	m_strobed(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_lightpen_device::device_start()
{
	m_sensor_timer = timer_alloc(FUNC(vcs_lightpen_device::sensor_tick), this);

	save_item(NAME(m_armed));
	save_item(NAME(m_strobed));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vcs_lightpen_device::device_reset()
{
	m_armed = false;
	m_strobed = false;
	m_sensor_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  vcs_joy_r - lightpen read
//-------------------------------------------------

uint8_t vcs_lightpen_device::vcs_joy_r()
{
	if (!has_lightpen_timing())
		return m_joy->read();

	// pin 6 carries the sensor strobe; the tip switch only arms it
	uint8_t data = m_joy->read() | 0x20;
	if (m_strobed)
		data &= ~0x20;

	return data;
}



//**************************************************************************
//  LIGHT PEN (UP) - button wired to joystick Up (pin 1)
//**************************************************************************

static INPUT_PORTS_START( vcs_lpu )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Light Pen Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vcs_lightpen_device::trigger), 0)
	PORT_BIT( 0xde, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor vcs_light_pen_up_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_lpu );
}

vcs_light_pen_up_device::vcs_light_pen_up_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightpen_device(mconfig, VCS_LIGHTPEN_UP, tag, owner, clock)
{
}



//**************************************************************************
//  LIGHT PEN (LEFT) - button wired to joystick Left (pin 3)
//**************************************************************************

static INPUT_PORTS_START( vcs_lpl )
	PORT_START("JOY")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Light Pen Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vcs_lightpen_device::trigger), 0)
	PORT_BIT( 0xdb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor vcs_light_pen_left_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_lpl );
}

vcs_light_pen_left_device::vcs_light_pen_left_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightpen_device(mconfig, VCS_LIGHTPEN_LEFT, tag, owner, clock)
{
}



//**************************************************************************
//  LIGHT PEN (RIGHT) - button wired to joystick Right (pin 4)
//**************************************************************************

static INPUT_PORTS_START( vcs_lpr )
	PORT_START("JOY")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Light Pen Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vcs_lightpen_device::trigger), 0)
	PORT_BIT( 0xd7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor vcs_light_pen_right_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_lpr );
}

vcs_light_pen_right_device::vcs_light_pen_right_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightpen_device(mconfig, VCS_LIGHTPEN_RIGHT, tag, owner, clock)
{
}
