// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ssc.c

    4 player digital joystick card for the Apple II by Lukazi
    http://lukazi.blogspot.com/2016/05/apple-ii-4play-joystick-card-revb.html

*********************************************************************/

#include "emu.h"
#include "4play.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_4play_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_4play_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_4play_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;

	required_ioport m_p1, m_p2, m_p3, m_p4;
};

static INPUT_PORTS_START( a24play )
	PORT_START("p1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("p2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("p3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)

	PORT_START("p4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_4play_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a24play );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_4play_device::a2bus_4play_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_4play_device(mconfig, A2BUS_4PLAY, tag, owner, clock)
{
}

a2bus_4play_device::a2bus_4play_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_p1(*this, "p1"),
		m_p2(*this, "p2"),
		m_p3(*this, "p3"),
		m_p4(*this, "p4")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_4play_device::device_start()
{
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_4play_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_p1->read();
		case 1:
			return m_p2->read();
		case 2:
			return m_p3->read();
		case 3:
			return m_p4->read();

		default:
			return 0xff;
	}

	return 0;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_4PLAY, device_a2bus_card_interface, a2bus_4play_device, "a24play", "4play Joystick Card (rev. B)")
