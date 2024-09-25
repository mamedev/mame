// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Apple II Wico Command Control Joystick Adapter for Atari-style
    digital joysticks

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/wico_joystick.h"

namespace {

// ======================> apple2_wico_joystick_device

class apple2_wico_joystick_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_wico_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_a2gameio_interface implementation
	virtual u8 pdl0_r() override;
	virtual u8 pdl1_r() override;
	virtual u8 pdl2_r() override;
	virtual u8 pdl3_r() override;
	virtual int sw0_r() override;
	virtual int sw1_r() override;

private:
	// input ports
	required_ioport m_player1, m_player2;
};

//**************************************************************************
//  PARAMETERS
//**************************************************************************

#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( apple2_wico_joystick )
	PORT_START("joystick_p1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("joystick_p2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple2_wico_joystick_device::apple2_wico_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_WICO_JOYSTICK, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_player1(*this, "joystick_p1")
	, m_player2(*this, "joystick_p2")
{
}

ioport_constructor apple2_wico_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(apple2_wico_joystick);
}

void apple2_wico_joystick_device::device_start()
{
}

u8 apple2_wico_joystick_device::pdl0_r()
{
	return !BIT(m_player1->read(), 3) ? 0x00 : !BIT(m_player1->read(), 1) ? 0xff : 0x80;
}

u8 apple2_wico_joystick_device::pdl1_r()
{
	return !BIT(m_player1->read(), 0) ? 0x00 : !BIT(m_player1->read(), 2) ? 0xff : 0x80;
}

u8 apple2_wico_joystick_device::pdl2_r()
{
	return !BIT(m_player2->read(), 3) ? 0x00 : !BIT(m_player2->read(), 1) ? 0xff : 0x80;
}

u8 apple2_wico_joystick_device::pdl3_r()
{
	return !BIT(m_player2->read(), 0) ? 0x00 : !BIT(m_player2->read(), 2) ? 0xff : 0x80;
}

int apple2_wico_joystick_device::sw0_r()
{
	return !BIT(m_player1->read(), 4);
}

int apple2_wico_joystick_device::sw1_r()
{
	return !BIT(m_player2->read(), 4);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_WICO_JOYSTICK, device_a2gameio_interface, apple2_wico_joystick_device, "a2wicojoy", "Wico Command Control Joystick Adapter")
