// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES Multitap Adapter

**********************************************************************/

#include "multitap.h"
#include "joypad.h"
#include "twintap.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_MULTITAP = &device_creator<snes_multitap_device>;


static INPUT_PORTS_START( snes_multitap )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Number of players")
	PORT_CONFSETTING(  0x00, "3-5P" )
	PORT_CONFSETTING(  0x01, "2P" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor snes_multitap_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( snes_multitap );
}


static SLOT_INTERFACE_START( snes_multitap )
	SLOT_INTERFACE("joypad", SNES_JOYPAD)
	SLOT_INTERFACE("twintap", SNES_TWINTAP)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( multi5p )
	MCFG_SNES_CONTROL_PORT_ADD("port1", snes_multitap, "joypad")
	MCFG_SNES_CONTROL_PORT_ADD("port2", snes_multitap, "joypad")
	MCFG_SNES_CONTROL_PORT_ADD("port3", snes_multitap, "joypad")
	MCFG_SNES_CONTROL_PORT_ADD("port4", snes_multitap, "joypad")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor snes_multitap_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( multi5p );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_multitap_device - constructor
//-------------------------------------------------

snes_multitap_device::snes_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SNES_MULTITAP, "Nintendo SNES / SFC Multitap Adapter", tag, owner, clock, "snes_multitap", __FILE__),
	device_snes_control_port_interface(mconfig, *this),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_cfg(*this, "CONFIG")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_multitap_device::device_start()
{
	save_item(NAME(m_select));
}

void snes_multitap_device::device_reset()
{
	m_select = 1;
}

//-------------------------------------------------
//  poll
//-------------------------------------------------

void snes_multitap_device::port_poll()
{
	m_port1->port_poll();
	if (m_cfg->read() == 0) // 4P
	{
		m_port2->port_poll();
		m_port3->port_poll();
		m_port4->port_poll();
	}
}

//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 snes_multitap_device::read_pin4()
{
	UINT8 ret = 0;

	if (m_cfg->read() == 0) // 4P
		ret |= m_select ? m_port1->read_pin4() : m_port3->read_pin4();
	else    // 1P
		ret |= m_select ? m_port1->read_pin4() : 0;

	return ret;
}

UINT8 snes_multitap_device::read_pin5()
{
	UINT8 ret = 0;

	if (m_cfg->read() == 0) // 4P
		ret |= m_select ? m_port2->read_pin4() : m_port4->read_pin4();
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_multitap_device::write_strobe(UINT8 data)
{
	m_port1->write_strobe(data);
	if (m_cfg->read() == 0) // 4P
	{
		m_port2->write_strobe(data);
		m_port3->write_strobe(data);
		m_port4->write_strobe(data);
	}
}

void snes_multitap_device::write_pin6(UINT8 data)
{
	m_select = data & 1;
}
