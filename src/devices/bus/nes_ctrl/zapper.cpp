// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun

**********************************************************************/

#include "emu.h"
#include "zapper.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device, "nes_zapper", "Nintendo Zapper Lightgun")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( nes_zapper )
	PORT_START("GUN_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0, 255)
	PORT_START("GUN_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0, 239)
	PORT_START("GUN_T")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Lightgun Trigger") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nes_zapper_device::trigger), 0)
INPUT_PORTS_END

ioport_constructor nes_zapper_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_zapper );
}

INPUT_CHANGED_MEMBER(nes_zapper_device::trigger)
{
	// a trigger pull is active for around 3-4 frames
	if (newval)
		m_trigger->adjust(attotime::from_msec(50));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_zapper_device::device_add_mconfig(machine_config &config)
{
	NES_ZAPPER_SENSOR(config, m_sensor, 0);
	if (m_port != nullptr)
		m_sensor->set_screen_tag(m_port->m_screen);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

nes_zapper_device::nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_ZAPPER, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_sensor(*this, "sensor")
	, m_lightx(*this, "GUN_X")
	, m_lighty(*this, "GUN_Y")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_zapper_device::device_start()
{
	m_trigger = timer_alloc(timer_expired_delegate());
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_zapper_device::read_bit34()
{
	u8 ret = m_trigger->enabled() ? 0x10 : 0;

	if (!m_sensor->detect_light(m_lightx->read(), m_lighty->read()))
		ret |= 0x08;

	return ret;
}

u8 nes_zapper_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	if (offset == 1)    // $4017
		ret = nes_zapper_device::read_bit34();
	return ret;
}
