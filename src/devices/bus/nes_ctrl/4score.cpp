// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System Four Score Adapter


 TODO: current implementation is a HACK, due to limitations of the
 slot system!
 A real Four Score would be connected to both the controller ports of
 the NES, but current core cannot emulate something like this until
 devices can live their own life and being connected to other devices
 at request.
 In current implementation the device has to be mounted separately in
 the two ports and each enables 2 inputs (this is more or less as hacky
 as the non-slot previous one, where the 4 ports were always available
 to the emulated system, but it's not a great consolation :( )
 Two subdevices are currently used so to warn the user that the first
 one gives P1+P3 inputs and the second one gives P2+P4 inputs.
 For the same reason, we don't currently emulate the 2P/4P switch,
 since we could at best have two switches to disable the second player
 inputs.

 Note: Two Pads are hardcoded in inputs below, instead of acting as
 passthrough for 2 standard joypad devices, in order to show in the
 internal UI that they belong to P1+P3 and P2+P4, otherwise they would
 be listed as P1+P2 and P3+P4 respectively. This *HAS* to be changed
 once the slot support in the code has improved (4 standard joypads
 shall be attached to the unit!)

**********************************************************************/

#include "4score.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_4SCORE_P1P3 = &device_creator<nes_4score_p1p3_device>;
const device_type NES_4SCORE_P2P4 = &device_creator<nes_4score_p2p4_device>;


static INPUT_PORTS_START( nes_4score_p1p3 )
	PORT_START("PAD1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("PAD3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A") PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B") PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
INPUT_PORTS_END


static INPUT_PORTS_START( nes_4score_p2p4 )
	PORT_START("PAD2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("PAD4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A") PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B") PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_4score_p1p3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_4score_p1p3 );
}

ioport_constructor nes_4score_p2p4_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_4score_p2p4 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_4score_device - constructor
//-------------------------------------------------

nes_4score_device::nes_4score_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_nes_control_port_interface(mconfig, *this), m_latch(0)
				{
}

nes_4score_p1p3_device::nes_4score_p1p3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					nes_4score_device(mconfig, NES_4SCORE_P1P3, "Nintendo Four Score Adapter P1/P3", tag, owner, clock, "nes_4score_p1p3", __FILE__),
					m_joypad1(*this, "PAD1"),
					m_joypad3(*this, "PAD3")
{
}

nes_4score_p2p4_device::nes_4score_p2p4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					nes_4score_device(mconfig, NES_4SCORE_P2P4, "Nintendo Four Score Adapter P2/P4", tag, owner, clock, "nes_4score_p2p4", __FILE__),
					m_joypad2(*this, "PAD2"),
					m_joypad4(*this, "PAD4")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_4score_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_4score_device::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_4score_device::read_bit0()
{
	UINT8 ret = m_latch & 1;
	m_latch >>= 1;
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_4score_p1p3_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	// P3 & P4 inputs in NES Four Score are read serially with P1 & P2
	m_latch  = m_joypad1->read();
	m_latch |= (m_joypad3->read() << 8); // pad 3
	m_latch |= (0x08 << 16); // signature
}

void nes_4score_p2p4_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	// P3 & P4 inputs in NES Four Score are read serially with P1 & P2
	m_latch = m_joypad2->read();
	m_latch |= (m_joypad4->read() << 8); // pad 4
	m_latch |= (0x04 << 16); // signature
}
