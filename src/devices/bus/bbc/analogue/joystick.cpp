// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Joystick Controllers

    ANH01 - Acorn Analogue Joystick Controllers
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANH01_JoystickController.html

    Voltmace Delta 3b "Twin" Joystick Controllers (self centering)

**********************************************************************/

#include "emu.h"
#include "joystick.h"


namespace {

class bbc_joystick_device : public device_t, public device_bbc_analogue_interface
{
protected:
	bbc_joystick_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_bbc_analogue_interface(mconfig, *this),
		m_joy(*this, "JOY%u", 0),
		m_buttons(*this, "BUTTONS")
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual uint16_t ch_r(offs_t channel) override
	{
		return m_joy[channel]->read() << 4;
	}

	virtual uint8_t pb_r() override
	{
		return m_buttons->read() & 0x30;
	}

private:
	required_ioport_array<4> m_joy;
	required_ioport m_buttons;
};


class bbc_acornjoy_device : public bbc_joystick_device
{
public:
	bbc_acornjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		bbc_joystick_device(mconfig, BBC_ACORNJOY, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


class bbc_voltmace3b_device : public bbc_joystick_device
{
public:
	bbc_voltmace3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		bbc_joystick_device(mconfig, BBC_VOLTMACE3B, tag, owner, clock)
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( acornjoy )
	PORT_START("JOY0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( voltmace3b )
	PORT_START("JOY0")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(50) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(50) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(50) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT(0xfff, 0x800, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_CENTERDELTA(50) PORT_MINMAX(0x00, 0xfff) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
INPUT_PORTS_END


ioport_constructor bbc_acornjoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( acornjoy );
}

ioport_constructor bbc_voltmace3b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( voltmace3b );
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_ACORNJOY, device_bbc_analogue_interface, bbc_acornjoy_device, "bbc_acornjoy", "Acorn Analogue Joysticks")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_VOLTMACE3B, device_bbc_analogue_interface, bbc_voltmace3b_device, "bbc_voltmace3b", "Voltmace Delta 3b Twin Joysticks")
