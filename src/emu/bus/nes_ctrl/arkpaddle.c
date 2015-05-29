// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System -
    Arkanoid Paddle input device

**********************************************************************/

#include "arkpaddle.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_ARKPADDLE = &device_creator<nes_vaus_device>;
const device_type NES_ARKPADDLE_FC = &device_creator<nes_vausfc_device>;


static INPUT_PORTS_START( arkanoid_paddle )
	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_CENTERDELTA(0) PORT_MINMAX(0x62,0xf2)
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Paddle button")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_vaus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( arkanoid_paddle );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_vaus_device - constructor
//-------------------------------------------------

nes_vaus_device::nes_vaus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_nes_control_port_interface(mconfig, *this),
						m_paddle(*this, "PADDLE"),
						m_button(*this, "BUTTON")
{
}

nes_vaus_device::nes_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_ARKPADDLE, "NES Arkanoid Vaus Controller", tag, owner, clock, "nes_vaus", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_paddle(*this, "PADDLE"),
					m_button(*this, "BUTTON")
{
}

nes_vausfc_device::nes_vausfc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					nes_vaus_device(mconfig, NES_ARKPADDLE_FC, "FC Arkanoid Vaus Controller", tag, owner, clock, "nes_vausfc", __FILE__)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_vaus_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_start_conv));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_vaus_device::device_reset()
{
	m_latch = 0;
	m_start_conv = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_vaus_device::read_bit34()
{
	UINT8 ret = (m_button->read() << 3);
	ret |= (m_latch & 0x80) >> 3;
	m_latch <<= 1;
	m_latch &= 0xff;
	return ret;
}

UINT8 nes_vausfc_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 0)    //$4016
		ret = m_button->read() << 1;
	else    //$4017
	{
		ret = (m_latch & 0x80) >> 6;
		m_latch <<= 1;
		m_latch &= 0xff;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_vaus_device::write(UINT8 data)
{
	int old = m_start_conv;

	if (data == 0 && old == 1)
		m_latch = (UINT8) (m_paddle->read() ^ 0xff);

	m_start_conv = data;
}
