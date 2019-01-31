// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES Multitap Adapter

**********************************************************************/

#include "emu.h"
#include "multitap.h"
#include "joypad.h"
#include "twintap.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SNES_MULTITAP, snes_multitap_device, "snes_multitap", "Nintendo SNES / SFC Multitap Adapter")


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


static void snes_multitap(device_slot_interface &device)
{
	device.option_add("joypad", SNES_JOYPAD);
	device.option_add("twintap", SNES_TWINTAP);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void snes_multitap_device::device_add_mconfig(machine_config &config)
{
	SNES_CONTROL_PORT(config, m_port1, snes_multitap, "joypad");
	SNES_CONTROL_PORT(config, m_port2, snes_multitap, "joypad");
	SNES_CONTROL_PORT(config, m_port3, snes_multitap, "joypad");
	SNES_CONTROL_PORT(config, m_port4, snes_multitap, "joypad");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_multitap_device - constructor
//-------------------------------------------------

snes_multitap_device::snes_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SNES_MULTITAP, tag, owner, clock),
	device_snes_control_port_interface(mconfig, *this),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_cfg(*this, "CONFIG"), m_select(0)
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

uint8_t snes_multitap_device::read_pin4()
{
	uint8_t ret = 0;

	if (m_cfg->read() == 0) // 4P
		ret |= m_select ? m_port1->read_pin4() : m_port3->read_pin4();
	else    // 1P
		ret |= m_select ? m_port1->read_pin4() : 0;

	return ret;
}

uint8_t snes_multitap_device::read_pin5()
{
	uint8_t ret = 0;

	if (m_cfg->read() == 0) // 4P
		ret |= m_select ? m_port2->read_pin4() : m_port4->read_pin4();
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_multitap_device::write_strobe(uint8_t data)
{
	m_port1->write_strobe(data);
	if (m_cfg->read() == 0) // 4P
	{
		m_port2->write_strobe(data);
		m_port3->write_strobe(data);
		m_port4->write_strobe(data);
	}
}

void snes_multitap_device::write_pin6(uint8_t data)
{
	m_select = data & 1;
}
