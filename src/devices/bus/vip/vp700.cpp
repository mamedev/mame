// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Tiny BASIC VP-700 emulation

**********************************************************************/

#include "emu.h"
#include "vp700.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP700, vp700_device, "vp700", "VP-700 Expanded Tiny BASIC")


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

const tiny_rom_entry *vp700_device::device_rom_region() const
{
	return ROM_NAME( vp700 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp700_device - constructor
//-------------------------------------------------

vp700_device::vp700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP700, tag, owner, clock),
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

uint8_t vp700_device::vip_program_r(offs_t offset, int cs, int cdef, int *minh)
{
	uint8_t data = 0xff;

	if (offset < 0x1000)
	{
		*minh = 1;

		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}
