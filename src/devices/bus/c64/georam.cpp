// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Berkeley Softworks GeoRAM emulation

**********************************************************************/

#include "georam.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_GEORAM = &device_creator<c64_georam_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_georam_cartridge_device - constructor
//-------------------------------------------------

c64_georam_cartridge_device::c64_georam_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_GEORAM, "C64 GeoRAM cartridge", tag, owner, clock, "c64_georam", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram"), 
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_georam_cartridge_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x80000);

	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_georam_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_georam_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		offs_t addr = (m_bank << 8) | (offset & 0xff);
		data = m_ram[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_georam_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		offs_t addr = (m_bank << 8) | (offset & 0xff);
		m_ram[addr] = data;
	}
	else if (!io2)
	{
		if (BIT(offset, 0))
		{
			m_bank = ((data & 0x1f) << 6) | (m_bank & 0x3f);
		}
		else
		{
			m_bank = (m_bank & 0x7c0) | (data & 0x3f);
		}
	}
}
