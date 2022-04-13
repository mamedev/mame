// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Nintendo HVC-012 Family Computer Robot / Nintendo NES-012 R.O.B.

    TODO:
    - does nes_rob have motor sensors? (eg. limit switches, or optical
      sensor to determine position)
    - can't really play anything with nes_rob, because of interaction
      with physical objects (gyromite especially, since it has a gadget
      to make the robot press joypad buttons)

**********************************************************************/

#include "emu.h"
#include "rob.h"

#include "nes_rob.lh"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ROB, nes_rob_device, "nes_rob", "Nintendo R.O.B. / Family Computer Robot")


static INPUT_PORTS_START( nes_rob )
	PORT_START("EYE_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0, 255)
	PORT_START("EYE_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0, 239)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_rob_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_rob );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

nes_rob_device::nes_rob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_ROB, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_sensor(*this, "sensor")
	, m_eye_x(*this, "EYE_X")
	, m_eye_y(*this, "EYE_Y")
	, m_motor_out(*this, "rob_motor.%u", 0U)
	, m_led_out(*this, "rob_led")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_rob_device::device_start()
{
	// resolve handlers
	m_motor_out.resolve();
	m_led_out.resolve();
}


//-------------------------------------------------
//  R.O.B. specific handlers
//-------------------------------------------------

u8 nes_rob_device::input_r()
{
	// R00: lightsensor
	return !m_sensor->detect_light(m_eye_x->read(), m_eye_y->read());
}

void nes_rob_device::output_w(offs_t offset, u8 data)
{
	switch (offset & 3)
	{
		case 0:
			// R03: led
			m_led_out = BIT(data, 3);
			break;

		case 1:
			// R10-R13: motors: down, up, close, open
			for (int i = 0; i < 4; i++)
				m_motor_out[i] = BIT(data, i);
			break;

		case 2:
			// R20,R21: motors: right, left
			for (int i = 0; i < 2; i++)
				m_motor_out[i + 4] = BIT(data, i);
			break;

		default:
			break;
	}
}

void nes_rob_device::device_add_mconfig(machine_config &config)
{
	SM590(config, m_maincpu, 455_kHz_XTAL);
	m_maincpu->read_r<0>().set(FUNC(nes_rob_device::input_r));
	m_maincpu->write_r<0>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<1>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<2>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<3>().set(FUNC(nes_rob_device::output_w));

	NES_ZAPPER_SENSOR(config, m_sensor, 0);
	if (m_port != nullptr)
		m_sensor->set_screen_tag(m_port->m_screen);

	// must use -numscreens 2 to see the output status
	config.set_default_layout(layout_nes_rob);
}

ROM_START( nes_rob )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "rfc-cpu10.ic1", 0x000, 0x200, CRC(f9c96b9c) SHA1(a87e2f0f5e454c093d1352ac368aa9e82e9f6790) )
ROM_END

const tiny_rom_entry *nes_rob_device::device_rom_region() const
{
	return ROM_NAME(nes_rob);
}
