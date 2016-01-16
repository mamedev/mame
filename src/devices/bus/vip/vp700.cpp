// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Tiny BASIC VP-700 emulation

**********************************************************************/

#include "vp700.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VP700 = &device_creator<vp700_device>;


//-------------------------------------------------
//  ROM( vp700 )
//-------------------------------------------------

ROM_START( vp700 )
	ROM_REGION( 0x1000, "vp700", 0 )
	ROM_LOAD( "vp700.bin", 0x0000, 0x1000, CRC(3f2b8524) SHA1(8fa88740cae82d8d62ea34891a657d3ca1fb732a) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *vp700_device::device_rom_region() const
{
	return ROM_NAME( vp700 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp700_device - constructor
//-------------------------------------------------

vp700_device::vp700_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VP700, "VP700", tag, owner, clock, "vp700", __FILE__),
	device_vip_expansion_card_interface(mconfig, *this),
	m_rom(*this, "vp700")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp700_device::device_start()
{
}


//-------------------------------------------------
//  vip_program_r - program read
//-------------------------------------------------

UINT8 vp700_device::vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh)
{
	UINT8 data = 0xff;

	if (offset < 0x1000)
	{
		*minh = 1;

		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}
