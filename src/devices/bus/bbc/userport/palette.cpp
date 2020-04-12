// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Clwyd Technics Colour Palette

    Micro User Chameleon (DIY) - Micro User Jan/Feb 1990

    TODO:
    - the Chameleon should also implement a CB2 handler, as this is
      pulsed whenever PB is written to. pb_w should only latch the
      data, then a CB2 handler would update the palette.

**********************************************************************/

#include "emu.h"
#include "palette.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CHAMELEON, bbc_chameleon_device, "bbc_chameleon", "Micro User Chameleon (DIY)")
DEFINE_DEVICE_TYPE(BBC_CPALETTE, bbc_cpalette_device, "bbc_cpalette", "Clwyd Technics Colour Palette")


//-------------------------------------------------
//  ROM( chameleon )
//-------------------------------------------------

ROM_START(chameleon)
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("chameleon.rom", 0x0000, 0x2000, CRC(e0ea0252) SHA1(72cf334cdb866ba3dd2353fc7da0dfdb6abf63a7))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry* bbc_chameleon_device::device_rom_region() const
{
	return ROM_NAME(chameleon);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_palette_device - constructor
//-------------------------------------------------

bbc_palette_device::bbc_palette_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_palette(*this, ":palette")
	, m_colour(0)
{
}

bbc_chameleon_device::bbc_chameleon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_palette_device(mconfig, BBC_CHAMELEON, tag, owner, clock)
{
}

bbc_cpalette_device::bbc_cpalette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_palette_device(mconfig, BBC_CPALETTE, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_palette_device::device_start()
{
	memset(m_palette_ram, 0, sizeof(m_palette_ram));

	/* register for save states */
	save_item(NAME(m_colour));
	save_item(NAME(m_palette_ram));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_chameleon_device::pb_w(uint8_t data)
{
	data ^= 0x0f;

	switch (data >> 6)
	{
	case 0x00:
		m_palette_ram[m_colour].set_r((data << 4) | (data & 0x0f));
		break;
	case 0x01:
		m_palette_ram[m_colour].set_g((data << 4) | (data & 0x0f));
		break;
	case 0x02:
		m_palette_ram[m_colour].set_b((data << 4) | (data & 0x0f));
		break;
	case 0x03:
		m_colour = data & 0x0f;
		m_palette->set_pen_colors(0, &m_palette_ram[BIT(data, 5) << 3], 8);
		break;
	}
}

void bbc_cpalette_device::pb_w(uint8_t data)
{
	if (BIT(data, 3))
	{
		switch (data & 0x07)
		{
		case 0x00:
			m_colour = ~(data >> 4) & 0x07;
			break;
		case 0x01:
			m_palette_ram[m_colour].set_r((data & 0xf0) | (data >> 4));
			break;
		case 0x02:
			m_palette_ram[m_colour].set_g((data & 0xf0) | (data >> 4));
			break;
		case 0x03:
			m_palette_ram[m_colour].set_b((data & 0xf0) | (data >> 4));
			break;
		}
	}
	else
	{
		m_palette->set_pen_colors(0, &m_palette_ram[0], 8);
	}
}
