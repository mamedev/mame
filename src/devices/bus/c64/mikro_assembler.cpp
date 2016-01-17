// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mikro Assembler cartridge emulation

**********************************************************************/

#include "mikro_assembler.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MIKRO_ASSEMBLER = &device_creator<c64_mikro_assembler_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_mikro_assembler_cartridge_device - constructor
//-------------------------------------------------

c64_mikro_assembler_cartridge_device::c64_mikro_assembler_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MIKRO_ASSEMBLER, "C64 Mikro Assembler cartridge", tag, owner, clock, "c64_mikro_assembler", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_mikro_assembler_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_mikro_assembler_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh || !io1 || !io2)
	{
		data = m_roml[offset & 0x1fff];
	}

	return data;
}
