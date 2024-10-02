// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Sirius JoyPort with 4 Apple II paddles connected

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/joyport_paddles.h"


namespace {

// ======================> apple2_joyport_paddles_device

class apple2_joyport_paddles_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_joyport_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	virtual void an0_w(int state) override;

private:
	// input ports
	required_ioport_array<4> m_pdl;
	required_ioport m_buttons;
	int m_an0;
};

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( apple2_joyport_paddles )
	PORT_START("paddle_1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(1) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)

	PORT_START("paddle_2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(2) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)

	PORT_START("paddle_3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(3) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)

	PORT_START("paddle_4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(4) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255)

	PORT_START("paddle_buttons")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1)   PORT_PLAYER(4)
INPUT_PORTS_END

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple2_joyport_paddles_device::apple2_joyport_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_JOYPORT_PADDLES, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_pdl(*this, "paddle_%u", 1U)
	, m_buttons(*this, "paddle_buttons")
{
}

ioport_constructor apple2_joyport_paddles_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(apple2_joyport_paddles);
}

void apple2_joyport_paddles_device::device_start()
{
	save_item(NAME(m_an0));
}

u8 apple2_joyport_paddles_device::pdl0_r()
{
	return m_pdl[0]->read();
}

u8 apple2_joyport_paddles_device::pdl1_r()
{
	return m_pdl[1]->read();
}

u8 apple2_joyport_paddles_device::pdl2_r()
{
	return m_pdl[2]->read();
}

u8 apple2_joyport_paddles_device::pdl3_r()
{
	return m_pdl[3]->read();
}

int apple2_joyport_paddles_device::sw0_r()
{
	return m_an0 ? BIT(m_buttons->read(), 6) : BIT(m_buttons->read(), 4);
}

int apple2_joyport_paddles_device::sw1_r()
{
	return m_an0 ? BIT(m_buttons->read(), 7) : BIT(m_buttons->read(), 5);
}

void apple2_joyport_paddles_device::an0_w(int state)
{
	m_an0 = state;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_JOYPORT_PADDLES, device_a2gameio_interface, apple2_joyport_paddles_device, "a2joyprtpdls", "Sirius JoyPort with Apple II paddles")
