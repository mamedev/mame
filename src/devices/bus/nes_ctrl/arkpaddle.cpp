// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System -
    Arkanoid Paddle input device

 TODO: Investigate the differences between the 3 paddles released with
 Arkanoid (US), Arkanoid (JP), Arkanoid II (JP). At the moment we only
 emulate the US and daisy-chainable Famicom (Ark2) paddles.

 Known differences: NES and Ark2 paddles have screws to adjust range
 of returned values, Ark2 paddle has an extra expansion port.

 All paddles return the ones' complement of a 9-bit value from their
 potentiometers, MSB first. The three commercial releases, Arkanoids
 and Chase HQ, only read 8-bits, ignoring the LSB.

**********************************************************************/

#include "emu.h"
#include "arkpaddle.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ARKPADDLE,    nes_vaus_device,   "nes_vaus",   "NES Arkanoid Vaus Controller")
DEFINE_DEVICE_TYPE(NES_ARKPADDLE_FC, nes_vausfc_device, "nes_vausfc", "FC Arkanoid Vaus Controller")


static INPUT_PORTS_START( arkanoid_paddle )
	PORT_START("PADDLE")
	// MINMAX range large enough to encompass what Taito games expect, Ark1 will glitch if MIN is too low
	// This may be too wide compared with real paddles, even accounting for those with adjustment screws
	PORT_BIT( 0x1ff, 0x130, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(35) PORT_CENTERDELTA(0) PORT_MINMAX(0x78, 0x1f8)

	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Paddle button")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_vaus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( arkanoid_paddle );
}

static void vausfc_daisy(device_slot_interface &device)
{
	device.option_add("vaus", NES_ARKPADDLE_FC);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_vausfc_device::device_add_mconfig(machine_config &config)
{
	// expansion port to allow daisy chaining
	NES_CONTROL_PORT(config, m_daisychain, vausfc_daisy, nullptr);
	if (m_port != nullptr)
		m_daisychain->set_screen_tag(m_port->m_screen);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_vaus_device - constructor
//-------------------------------------------------

nes_vaus_device::nes_vaus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_paddle(*this, "PADDLE")
	, m_button(*this, "BUTTON")
	, m_latch(0)
{
}

nes_vaus_device::nes_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_vaus_device(mconfig, NES_ARKPADDLE, tag, owner, clock)
{
}

nes_vausfc_device::nes_vausfc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_vaus_device(mconfig, NES_ARKPADDLE_FC, tag, owner, clock)
	, m_daisychain(*this, "subexp")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_vaus_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_vaus_device::read_bit34()
{
	u8 ret = m_button->read() << 3;
	ret |= (m_latch & 0x100) >> 4;
	m_latch <<= 1;
	return ret;
}

u8 nes_vausfc_device::read_exp(offs_t offset)
{
	u8 ret;
	if (offset == 0)    //$4016
		ret = m_button->read() << 1;
	else    //$4017
	{
		ret = (m_latch & 0x100) >> 7;
		m_latch <<= 1;
		ret |= m_daisychain->read_exp(0) << 2;
		ret |= m_daisychain->read_exp(1) << 3;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_vaus_device::write(u8 data)
{
	if (write_strobe(data))
		m_latch = ~m_paddle->read();
}

void nes_vausfc_device::write(u8 data)
{
	m_daisychain->write(data);
	nes_vaus_device::write(data);
}
