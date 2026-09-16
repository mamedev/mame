// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes RTFM Joystick Interface

**********************************************************************/

#include "emu.h"
#include "rtfmjoy.h"


namespace {

class arc_rtfm_joystick_device : public device_t, public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_rtfm_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, ARC_RTFM_JOY, tag, owner, clock)
		, device_archimedes_econet_interface(mconfig, *this)
		, m_joy(*this, "JOY%u", 0U)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 read(offs_t offset) override
	{
		u8 data = 0xff;

		switch (offset & 0x03)
		{
		case 0x01:
			data = ~m_joy[0]->read() & 0x1f;
			break;
		case 0x02:
			data = ~m_joy[1]->read() & 0x1f;
			break;
		}

		return data;
	}

private:
	required_ioport_array<2> m_joy;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( rtfm_joystick )
	PORT_START("JOY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire") PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire") PORT_PLAYER(2)
INPUT_PORTS_END

ioport_constructor arc_rtfm_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rtfm_joystick );
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ARC_RTFM_JOY, device_archimedes_econet_interface, arc_rtfm_joystick_device, "arc_rtfmjoy", "Acorn Archimedes RTFM Joystick Interface");
