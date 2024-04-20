// license:BSD-3-Clause
// copyright-holders:Mark/Space Inc.
/**********************************************************************

    ColecoVision MegaCart emulation

**********************************************************************/

#include "emu.h"
#include "megacart.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECOVISION_MEGACART, colecovision_megacart_cartridge_device, "colecovision_megacart", "ColecoVision MegaCart")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  colecovision_megacart_cartridge_device - constructor
//-------------------------------------------------

colecovision_megacart_cartridge_device::colecovision_megacart_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECOVISION_MEGACART, tag, owner, clock),
	device_colecovision_cartridge_interface(mconfig, *this),
	m_bankcount(0),
	m_activebank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void colecovision_megacart_cartridge_device::device_start()
{
	save_item(NAME(m_bankcount));
	save_item(NAME(m_activebank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void colecovision_megacart_cartridge_device::device_reset()
{
	m_bankcount = m_rom_size >> 14;
	m_activebank = 0;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t colecovision_megacart_cartridge_device::read(offs_t offset, int _8000, int _a000, int _c000, int _e000)
{
	uint8_t data = 0xff;

	if (!_8000 || !_a000 || !_c000 || !_e000)
	{
		if (m_bankcount > 2)
		{
			// offset as passed to us is a delta from address 0x8000.

			if (offset >= 0x7fc0)
			{
				// Reads within the final 64 bytes select which megacart bank is active.
				m_activebank = offset & (m_bankcount - 1);
			}

			if (offset >= 0x4000)
			{
				// The offset is within the active megacart bank.
				offset = (m_activebank << 14) + (offset - 0x4000);
			}
			else
			{
				// The offset is within the last bank.
				offset = (m_bankcount << 14) + (offset - 0x4000);
			}
		}

		data = m_rom[offset];
	}

	return data;
}
