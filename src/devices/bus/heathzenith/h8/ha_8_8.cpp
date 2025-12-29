// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit HA-8-8 Extended Configuration Option board

****************************************************************************/

#include "emu.h"

#include "ha_8_8.h"

#define LOG_REG_R (1U << 1)
#define LOG_REG_W (1U << 2)
#define LOG_ORG0 (1U << 3)    // Shows register setup

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREGR(...)        LOGMASKED(LOG_REG_R, __VA_ARGS__)
#define LOGREGW(...)        LOGMASKED(LOG_REG_W, __VA_ARGS__)
#define LOGORG0(...)        LOGMASKED(LOG_ORG0, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

class ha_8_8_device : public device_t
					, public device_h8bus_card_interface
{
public:

	ha_8_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void map_io(address_space_installer & space) override ATTR_COLD;

	u8 portf2_r();
	void portf2_w(u8 data);

	u8 m_gpp;
	required_ioport m_sw1;

private:

	void update_gpp(u8 data);
};


ha_8_8_device::ha_8_8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_HA_8_8, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_sw1(*this, "SW1")
{
}

u8 ha_8_8_device::portf2_r()
{
	u8 sw1 = m_sw1->read();

	LOGREGR("%s: value: 0x%02x\n", FUNCNAME, sw1);

	return sw1;
}

void ha_8_8_device::portf2_w(u8 data)
{
	LOGREGW("%s: value: 0x%02x\n", FUNCNAME, data);

	if (data != m_gpp)
	{
		update_gpp(data);
	}
}

void ha_8_8_device::device_start()
{
	save_item(NAME(m_gpp));
}

void ha_8_8_device::device_reset()
{
	m_gpp = 0;

	set_slot_rom_disable(BIT(m_gpp, 5));
}

void ha_8_8_device::update_gpp(u8 data)
{
	u8 changed_gpp = data ^ m_gpp;

	m_gpp = data;

	if (BIT(changed_gpp, 5))
	{
		int state = BIT(m_gpp, 5);

		LOGORG0("%s: updating gpp: %d\n", FUNCNAME, state);

		set_slot_rom_disable(state);
	}
}

void ha_8_8_device::map_io(address_space_installer & space)
{
	space.install_readwrite_handler(0xf2, 0xf2,
		read8smo_delegate(*this, FUNC(ha_8_8_device::portf2_r)),
		write8smo_delegate(*this, FUNC(ha_8_8_device::portf2_w))
	);
}

static INPUT_PORTS_START( sw1 )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Port 174 device" )                 PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "H17" )
	PORT_DIPSETTING(    0x01, "H47" )
	PORT_DIPSETTING(    0x02, "Undefined" )
	PORT_DIPSETTING(    0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Port 170 device" )                 PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "Not in use" )
	PORT_DIPSETTING(    0x04, "H47" )
	PORT_DIPSETTING(    0x08, "Undefined" )
	PORT_DIPSETTING(    0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x00, "Primary Boot from" )               PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "Boots from device at port 174" )
	PORT_DIPSETTING(    0x10, "Boots from device at port 170" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x40, "19200" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

INPUT_PORTS_END

ioport_constructor ha_8_8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sw1);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_HA_8_8, device_h8bus_card_interface, ha_8_8_device, "h8_ha_8_8", "Heath HA-8-8 Extended Configuration Option Board");
