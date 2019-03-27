// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MultiMAX 1MB ROM / 2KB RAM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "multimax.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_MULTIMAX, vic10_multimax_device, "vic10_multimax", "VIC-10 MultiMAX Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_multimax_device - constructor
//-------------------------------------------------

vic10_multimax_device::vic10_multimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC10_MULTIMAX, tag, owner, clock), device_vic10_expansion_card_interface(mconfig, *this),
	m_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_multimax_device::device_start()
{
	// state saving
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_multimax_device::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  vic10_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic10_multimax_device::vic10_cd_r(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (!lorom)
	{
		data = m_lorom[((m_latch & 0x3f) << 14) | (offset & 0x1fff)];
	}
	else if (!uprom)
	{
		data = m_lorom[((m_latch & 0x3f) << 14) | 0x2000 | (offset & 0x1fff)];
	}
	else if (!exram)
	{
		if (m_latch)
		{
			data = m_exram[offset & 0x7ff];
		}
	}

	return data;
}


//-------------------------------------------------
//  vic10_cd_w - cartridge data write
//-------------------------------------------------

void vic10_multimax_device::vic10_cd_w(offs_t offset, uint8_t data, int lorom, int uprom, int exram)
{
	if (!exram)
	{
		if (m_latch)
		{
			m_exram[offset & 0x7ff] = data;
		}
		else
		{
			m_latch = data;
		}
	}
}
