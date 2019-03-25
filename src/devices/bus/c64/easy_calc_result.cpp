// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Easy Calc Result cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|     ROM0        LS73    |
    |=|                         |
    |=|                         |
    |=|                         |
    |=|                         |
    |=|     ROM1        LS00    |
    |=|                         |
    |===========================|

    ROM0  - Hitachi HN61364P 8Kx8 EPROM "CR2001"
    ROM1  - Hitachi HN613128P 16Kx8 EPROM "CR3001"

*/

#include "emu.h"
#include "easy_calc_result.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_EASY_CALC_RESULT, c64_easy_calc_result_cartridge_device, "c64_easy_calc_result", "C64 Easy Calc Result cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_easy_calc_result_cartridge_device - constructor
//-------------------------------------------------

c64_easy_calc_result_cartridge_device::c64_easy_calc_result_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_EASY_CALC_RESULT, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this), m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_easy_calc_result_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_easy_calc_result_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_easy_calc_result_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		data = m_romh[(m_bank << 13) | (offset & 0x1fff)];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_easy_calc_result_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_bank = !BIT(offset, 0);
	}
}
