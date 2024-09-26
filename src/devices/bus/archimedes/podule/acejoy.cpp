// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Computer Enterprises Joy Connect

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/ACE_JoyConnect.html

**********************************************************************/

#include "emu.h"
#include "acejoy.h"


namespace {

class arc_acejoy_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_acejoy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override { }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override
	{
		map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
		map(0x2008, 0x200b).portr("JOY0");
		map(0x200c, 0x200f).portr("JOY1");
	}

private:
	required_memory_region m_podule_rom;
};


//-------------------------------------------------
//  ROM( acejoy )
//-------------------------------------------------

ROM_START( acejoy )
	ROM_REGION(0x1000, "podule_rom", 0)
	ROM_LOAD("acejoy.bin", 0x0000, 0x1000, CRC(300c4aa8) SHA1(1addf507844c6590d111765051a43fe571491224))
ROM_END

const tiny_rom_entry *arc_acejoy_device::device_rom_region() const
{
	return ROM_NAME( acejoy );
}


//-------------------------------------------------
//  INPUT_PORTS( acejoy )
//-------------------------------------------------

static INPUT_PORTS_START( acejoy )
	PORT_START("JOY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor arc_acejoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( acejoy );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_acejoy_device - constructor
//-------------------------------------------------

arc_acejoy_device::arc_acejoy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ACEJOY, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ACEJOY, device_archimedes_podule_interface, arc_acejoy_device, "arc_acejoy", "ACE Joy Connect")
