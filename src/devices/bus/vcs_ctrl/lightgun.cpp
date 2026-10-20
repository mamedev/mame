// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64/128 light gun emulation

    Magnum Light Phaser
    Stack Light Rifle
    Inkwell Systems 184-C Light Pen
    MHT Ingenieros Gun Stick

    Magnum Light Phaser / Stack Light Rifle / Inkwell 184-C: sensor on
    pin 6 (active low), feeds the video chip's light pen latch, timed
    from time_until_lightpen_pos(), independent of any button - the
    photodiode is powered continuously from +5V/GND rather than being
    switched by a mechanical tip switch (contrast vcs_lightpen_device in
    lightpen.cpp). Buttons: Magnum pin 5 (Pot AY) to +5V; Rifle pin 3
    (Left) to GND; Inkwell 184-C pin 3 (Left) to GND plus pin 5 (Pot AY)
    to +5V as a second button.

    Gun Stick: sensor drives pin 2 (Down) directly, only while trigger
    (pin 6/Fire) is held - pin 2 is shared with the keyboard matrix, so
    an always-on sensor would corrupt key reads. Modeled as a wide-angle
    sensor: samples the rendered picture near the crosshair on each
    trigger poll, same technique as the NES zapper
    (bus/nes_ctrl/zapper_sensor.h).

**********************************************************************/

#include "emu.h"
#include "lightgun.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MAGNUM_LIGHT_PHASER, magnum_light_phaser_device, "vcs_magnum", "Magnum Light Phaser")
DEFINE_DEVICE_TYPE(STACK_LIGHT_RIFLE, stack_light_rifle_device, "vcs_stacklr", "Stack Light Rifle")
DEFINE_DEVICE_TYPE(GUN_STICK, vcs_gunstick_device, "vcs_gunstick", "Gun Stick")
DEFINE_DEVICE_TYPE(INKWELL_184C, inkwell_184c_device, "vcs_inkwell184c", "Inkwell 184-C Light Pen")



//**************************************************************************
//  BASE DEVICE - shared photodiode sensor
//**************************************************************************

vcs_lightgun_device::vcs_lightgun_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_lightx(*this, "LIGHTX"),
	m_lighty(*this, "LIGHTY"),
	m_sensor_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_lightgun_device::device_start()
{
	m_sensor_timer = timer_alloc(FUNC(vcs_lightgun_device::sensor_tick), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vcs_lightgun_device::device_reset()
{
	// the photodiode sensor is powered directly from +5V/GND on real hardware, so it
	// runs continuously - pulsing whenever the beam is under the crosshair - regardless
	// of whether the (separate) trigger switch is held
	if (has_lightpen_timing())
		m_sensor_timer->adjust(time_until_lightpen_pos(m_lightx->read(), m_lighty->read()));
	else
		m_sensor_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  sensor_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER( vcs_lightgun_device::sensor_tick )
{
	trigger_w(1);
	trigger_w(0);

	m_sensor_timer->adjust(time_until_lightpen_pos(m_lightx->read(), m_lighty->read()));
}



//**************************************************************************
//  MAGNUM LIGHT PHASER
//**************************************************************************

static INPUT_PORTS_START( vcs_magnum )
	PORT_START("TRIGGER")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor magnum_light_phaser_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_magnum );
}

magnum_light_phaser_device::magnum_light_phaser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightgun_device(mconfig, MAGNUM_LIGHT_PHASER, tag, owner, clock),
	m_trigger(*this, "TRIGGER")
{
}

uint8_t magnum_light_phaser_device::vcs_pot_y_r()
{
	return BIT(m_trigger->read(), 0) ? 0x00 : 0xff;
}



//**************************************************************************
//  STACK LIGHT RIFLE
//**************************************************************************

static INPUT_PORTS_START( vcs_stacklr )
	PORT_START("JOY")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor stack_light_rifle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_stacklr );
}

stack_light_rifle_device::stack_light_rifle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightgun_device(mconfig, STACK_LIGHT_RIFLE, tag, owner, clock),
	m_joy(*this, "JOY")
{
}

uint8_t stack_light_rifle_device::vcs_joy_r()
{
	// the trigger switch pulls pin 3 (Left) to GND when held
	return m_joy->read();
}



//**************************************************************************
//  GUN STICK
//**************************************************************************

static INPUT_PORTS_START( vcs_gunstick )
	PORT_START("JOY")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                         // pin 2 - light sensor, driven by the device below
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Trigger")   // pin 6 - trigger
	PORT_BIT( 0xdd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor vcs_gunstick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_gunstick );
}

vcs_gunstick_device::vcs_gunstick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GUN_STICK, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_lightx(*this, "LIGHTX"),
	m_lighty(*this, "LIGHTY")
{
}

void vcs_gunstick_device::device_start()
{
}

uint8_t vcs_gunstick_device::vcs_joy_r()
{
	uint8_t data = m_joy->read();

	if (!BIT(data, 5) && light_detected(m_lightx->read(), m_lighty->read()))
		data &= ~0x02;

	return data;
}



//**************************************************************************
//  INKWELL SYSTEMS 184-C
//**************************************************************************

static INPUT_PORTS_START( vcs_inkwell184c )
	PORT_START("JOY")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Light Pen Button")
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Light Pen Button 2")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor inkwell_184c_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_inkwell184c );
}

inkwell_184c_device::inkwell_184c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vcs_lightgun_device(mconfig, INKWELL_184C, tag, owner, clock),
	m_joy(*this, "JOY"),
	m_button2(*this, "BUTTON2")
{
}

uint8_t inkwell_184c_device::vcs_joy_r()
{
	// pin 3 (Left) is a plain button, independent of the sensor on pin 6
	return m_joy->read();
}

uint8_t inkwell_184c_device::vcs_pot_y_r()
{
	// the second button pulls pin 5 (Pot AY) to +5V when held
	return BIT(m_button2->read(), 0) ? 0x00 : 0xff;
}
