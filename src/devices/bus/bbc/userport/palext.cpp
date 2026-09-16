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
#include "palext.h"

#include "emupal.h"

#include <algorithm>


namespace {

class bbc_palext_device : public device_t, public device_bbc_userport_interface
{
protected:
	bbc_palext_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_bbc_userport_interface(mconfig, *this)
		, m_palette(*this, ":palette")
		, m_colour(0)
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	required_device<palette_device> m_palette;

	uint8_t m_colour;
	rgb_t m_palette_ram[16];
};


// ======================> bbc_chameleon_device

class bbc_chameleon_device : public bbc_palext_device
{
public:
	bbc_chameleon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_palext_device(mconfig, BBC_CHAMELEON, tag, owner, clock)
	{
	}

protected:
	// optional information overrides
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual void pb_w(uint8_t data) override;
};


// ======================> bbc_cpalette_device

class bbc_cpalette_device : public bbc_palext_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::PALETTE; }

	bbc_cpalette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: bbc_palext_device(mconfig, BBC_CPALETTE, tag, owner, clock)
	{
	}

protected:
	virtual void pb_w(uint8_t data) override;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(chameleon)
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("chameleon.rom", 0x0000, 0x2000, CRC(e0ea0252) SHA1(72cf334cdb866ba3dd2353fc7da0dfdb6abf63a7))
ROM_END

const tiny_rom_entry* bbc_chameleon_device::device_rom_region() const
{
	return ROM_NAME(chameleon);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_palext_device::device_start()
{
	std::fill(std::begin(m_palette_ram), std::end(m_palette_ram), rgb_t(0));

	// register for save states
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
		m_palette_ram[m_colour].set_r(pal4bit(data));
		break;
	case 0x01:
		m_palette_ram[m_colour].set_g(pal4bit(data));
		break;
	case 0x02:
		m_palette_ram[m_colour].set_b(pal4bit(data));
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
			m_palette_ram[m_colour].set_r(pal4bit(data));
			break;
		case 0x02:
			m_palette_ram[m_colour].set_g(pal4bit(data));
			break;
		case 0x03:
			m_palette_ram[m_colour].set_b(pal4bit(data));
			break;
		}
	}
	else
	{
		m_palette->set_pen_colors(0, &m_palette_ram[0], 8);
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_CHAMELEON, device_bbc_userport_interface, bbc_chameleon_device, "bbc_chameleon", "Micro User Chameleon (DIY)")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_CPALETTE, device_bbc_userport_interface, bbc_cpalette_device, "bbc_cpalette", "Clwyd Technics Colour Palette")
