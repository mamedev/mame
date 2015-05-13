// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Board VP-570 emulation

**********************************************************************/

#include "vp570.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VP570 = &device_creator<vp570_device>;


//-------------------------------------------------
//  INPUT_PORTS( vp570 )
//-------------------------------------------------

static INPUT_PORTS_START( vp570 )
	PORT_START("BASE")
	PORT_DIPNAME( 0x07, 0x01, "Address Range" )
	PORT_DIPSETTING(    0x00, "0000 thru 0FFF" )
	PORT_DIPSETTING(    0x01, "1000 thru 1FFF" )
	PORT_DIPSETTING(    0x02, "2000 thru 2FFF" )
	PORT_DIPSETTING(    0x03, "3000 thru 3FFF" )
	PORT_DIPSETTING(    0x04, "4000 thru 4FFF" )
	PORT_DIPSETTING(    0x05, "5000 thru 50FFF" )
	PORT_DIPSETTING(    0x06, "6000 thru 6FFF" )
	PORT_DIPSETTING(    0x07, "7000 thru 7FFF" )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Write Protect" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vp570_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vp570 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp570_device - constructor
//-------------------------------------------------

vp570_device::vp570_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VP570, "VP570", tag, owner, clock, "vp570", __FILE__),
	device_vip_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram"),
	m_base(*this, "BASE"),
	m_sw1(*this, "SW1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp570_device::device_start()
{
	m_ram.allocate(0x1000);
}


//-------------------------------------------------
//  vip_program_r - program read
//-------------------------------------------------

UINT8 vp570_device::vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh)
{
	UINT8 data = 0xff;

	offs_t base = m_base->read() << 12;

	if (offset >= base && offset < base + 0x1000)
	{
		*minh = 1;

		data = m_ram[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp570_device::vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh)
{
	offs_t base = m_base->read() << 12;

	if (offset >= base && offset < base + 0x1000)
	{
		*minh = 1;

		if (m_sw1->read())
		{
			m_ram[offset & 0xfff] = data;
		}
	}
}
