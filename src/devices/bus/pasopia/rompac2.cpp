// license:BSD-3-Clause
// copyright-holders:AJR, Angelo Salese
/****************************************************************************

    Toshiba Pasopia Kanji ROM PAC2 emulation

    TODO: find out the difference between PA7246 and PA7247 (which one
    is actually dumped here?)

****************************************************************************/

#include "emu.h"
#include "rompac2.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PASOPIA_PA7246, pasopia_pa7246_device, "pa7246", "PA7246 Pasopia Kanji ROM PAC2")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  pasopia_pa7246_device - construction
//-------------------------------------------------

pasopia_pa7246_device::pasopia_pa7246_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PASOPIA_PA7246, tag, owner, clock)
	, pac2_card_interface(mconfig, *this)
	, m_kanji_rom(*this, "kanji")
	, m_kanji_index(0)
{
}


ROM_START(pa7246)
	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kanji.rom", 0x00000, 0x20000, CRC(6109e308) SHA1(5c21cf1f241ef1fa0b41009ea41e81771729785f))
ROM_END

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  rom region description for this device
//-------------------------------------------------

const tiny_rom_entry *pasopia_pa7246_device::device_rom_region() const
{
	return ROM_NAME(pa7246);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pasopia_pa7246_device::device_start()
{
	save_item(NAME(m_kanji_index));
}

//**************************************************************************
//  PAC2 INTERFACE
//**************************************************************************

//-------------------------------------------------
//  pac2_read - I/O read access
//-------------------------------------------------

u8 pasopia_pa7246_device::pac2_read(offs_t offset)
{
	return m_kanji_rom[m_kanji_index];
}


//-------------------------------------------------
//  pac2_write - I/O write access
//-------------------------------------------------

void pasopia_pa7246_device::pac2_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_kanji_index = (m_kanji_index & 0x1ff00) | ((data & 0xff) << 0);
		break;

	case 1:
		m_kanji_index = (m_kanji_index & 0x100ff) | ((data & 0xff) << 8);
		break;

	case 2:
		m_kanji_index = (m_kanji_index & 0x0ffff) | ((data & 0x01) << 16);
		break;
	}
}
