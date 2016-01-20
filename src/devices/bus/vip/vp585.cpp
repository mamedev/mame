// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Keypad Interface Board VP585 emulation

**********************************************************************/

#include "vp585.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VP585 = &device_creator<vp585_device>;


//-------------------------------------------------
//  INPUT_PORTS( vp585 )
//-------------------------------------------------

static INPUT_PORTS_START( vp585 )
	PORT_START("J1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 3")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 4")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 5")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 6")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 7")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad 9")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad A")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad B")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad C")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad D")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad E")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 1 Keypad F")

	PORT_START("J2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 3")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 4")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 5")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 6")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 7")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad 9")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad A")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad B")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad C")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad D")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad E")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Player 2 Keypad F")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vp585_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vp585 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp585_device - constructor
//-------------------------------------------------

vp585_device::vp585_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VP585, "VP585", tag, owner, clock, "vp585", __FILE__),
	device_vip_expansion_card_interface(mconfig, *this),
	m_j1(*this, "J1"),
	m_j2(*this, "J2"), m_keylatch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp585_device::device_start()
{
	// state saving
	save_item(NAME(m_keylatch));
}


//-------------------------------------------------
//  vip_io_w - I/O write
//-------------------------------------------------

void vp585_device::vip_io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset == 0x02)
	{
		m_keylatch = data & 0x0f;
	}
}


//-------------------------------------------------
//  vip_ef3_r - EF3 read
//-------------------------------------------------

int vp585_device::vip_ef3_r()
{
	return BIT(m_j1->read(), m_keylatch) ? CLEAR_LINE : ASSERT_LINE;
}


//-------------------------------------------------
//  vip_ef4_r - EF4 read
//-------------------------------------------------

int vp585_device::vip_ef4_r()
{
	return BIT(m_j2->read(), m_keylatch) ? CLEAR_LINE : ASSERT_LINE;
}
