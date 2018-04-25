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

DEFINE_DEVICE_TYPE(MULTIMAX, multimax_t, "multimax", "MultiMAX Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  multimax_t - constructor
//-------------------------------------------------

multimax_t::multimax_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MULTIMAX, tag, owner, clock), device_vic10_expansion_card_interface(mconfig, *this),
	m_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void multimax_t::device_start()
{
	// state saving
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void multimax_t::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  vic10_cd_r - cartridge data read
//-------------------------------------------------

uint8_t multimax_t::vic10_cd_r(address_space &space, offs_t offset, uint8_t data, int lorom, int uprom, int exram)
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

void multimax_t::vic10_cd_w(address_space &space, offs_t offset, uint8_t data, int lorom, int uprom, int exram)
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
