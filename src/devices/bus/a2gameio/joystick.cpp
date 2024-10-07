// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Apple II analog joysticks

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/joystick.h"

namespace {

// ======================> apple2_joystick_device

class apple2_joystick_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_a2gameio_interface overrides
	virtual u8 pdl0_r() override;
	virtual u8 pdl1_r() override;
	virtual u8 pdl2_r() override;
	virtual u8 pdl3_r() override;
	virtual int sw0_r() override;
	virtual int sw1_r() override;
	virtual int sw2_r() override;
	virtual int sw3_r() override;

private:
	// input ports
	required_ioport_array<2> m_joy_x;
	required_ioport_array<2> m_joy_y;
	required_ioport m_buttons;
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

static INPUT_PORTS_START( apple2_joystick )
	PORT_START("joystick_1_x")      /* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P1 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)

	PORT_START("joystick_1_y")      /* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P1 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)

	PORT_START("joystick_2_x")      /* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P2 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)

	PORT_START("joystick_2_y")      /* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P2 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)

	PORT_START("joystick_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(1)            PORT_CODE(KEYCODE_0_PAD)    PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)   PORT_PLAYER(1)            PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(2)            PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2)   PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON2)
INPUT_PORTS_END

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple2_joystick_device::apple2_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_JOYSTICK, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_joy_x(*this, "joystick_%u_x", 1U)
	, m_joy_y(*this, "joystick_%u_y", 1U)
	, m_buttons(*this, "joystick_buttons")
{
}

ioport_constructor apple2_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(apple2_joystick);
}

void apple2_joystick_device::device_start()
{
}

u8 apple2_joystick_device::pdl0_r()
{
	return m_joy_x[0]->read();
}

u8 apple2_joystick_device::pdl1_r()
{
	return m_joy_y[0]->read();
}

u8 apple2_joystick_device::pdl2_r()
{
	return m_joy_x[1]->read();
}

u8 apple2_joystick_device::pdl3_r()
{
	return m_joy_y[1]->read();
}

int apple2_joystick_device::sw0_r()
{
	return BIT(m_buttons->read(), 4);
}

int apple2_joystick_device::sw1_r()
{
	return BIT(m_buttons->read(), 5);
}

int apple2_joystick_device::sw2_r()
{
	return BIT(m_buttons->read(), 6);
}

int apple2_joystick_device::sw3_r()
{
	return BIT(m_buttons->read(), 7);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_JOYSTICK, device_a2gameio_interface, apple2_joystick_device, "a2joy", "Apple II analog joysticks")
