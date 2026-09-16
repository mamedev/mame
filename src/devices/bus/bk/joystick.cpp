// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Dual joystick interface

***************************************************************************/

#include "emu.h"
#include "joystick.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_joystick_device

class bk_joystick_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD {}

	virtual uint16_t io_r() override;

private:
	required_ioport m_joy;
};

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( bk_joystick )
	PORT_START("JOY")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_SELECT) PORT_PLAYER(1)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(2)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_SELECT) PORT_PLAYER(2)
INPUT_PORTS_END

ioport_constructor bk_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bk_joystick);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_joystick_device - constructor
//-------------------------------------------------

bk_joystick_device::bk_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_JOYSTICK, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_joy(*this, "JOY")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint16_t bk_joystick_device::io_r()
{
	return m_joy->read();
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_JOYSTICK, device_bk_parallel_interface, bk_joystick_device, "bk_joystick", "BK Joystick Interface")
