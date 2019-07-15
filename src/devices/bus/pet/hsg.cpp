// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 8000 High Speed Graphics (324402-01) card emulation

**********************************************************************/

/*

    TODO:

    http://www.6502.org/users/sjgray/computer/hsg/index.html

    - version A (EF9365, 512x512 interlaced, 1 page)
    - version B (EF9366, 512x256 non-interlaced, 2 pages)

*/

#include "emu.h"
#include "hsg.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define EF9365_TAG  "ef9365"
#define EF9366_TAG  EF9365_TAG
#define SCREEN_TAG  "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM8000_HSG_A, cbm8000_hsg_a_device, "cbm8000_hsg_a", "CBM 8000 High Speed Graphics (A)")
DEFINE_DEVICE_TYPE(CBM8000_HSG_B, cbm8000_hsg_b_device, "cbm8000_hsg_b", "CBM 8000 High Speed Graphics (B)")


//-------------------------------------------------
//  ROM( cbm8000_hsg )
//-------------------------------------------------

ROM_START( cbm8000_hsg )
	ROM_REGION( 0x1000, "9000", 0 )
	ROM_LOAD( "pet_hsg-ud12 on 8032 9000,2532.bin", 0x0000, 0x1000, CRC(d651bf72) SHA1(d3d68228a5a8ec73fb39be860c00edb0d21bd1a9) )

	ROM_REGION( 0x1000, "a000", 0 )
	ROM_LOAD( "324381-01 rev b sw graphi", 0x0000, 0x1000, CRC(c8e3bff9) SHA1(12ed3176ddd632f52e91082ab574adcba2149684) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cbm8000_hsg_device::device_rom_region() const
{
	return ROM_NAME( cbm8000_hsg );
}


//-------------------------------------------------
//  ADDRESS_MAP( hsg_a_map )
//-------------------------------------------------

void cbm8000_hsg_a_device::hsg_a_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( hsg_b_map )
//-------------------------------------------------

void cbm8000_hsg_b_device::hsg_b_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).ram();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cbm8000_hsg_a_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(EF9365_TAG, FUNC(ef9365_device::screen_update));
	screen.set_size(512, 512);
	screen.set_visarea(0, 512-1, 0, 512-1);
	screen.set_refresh_hz(25);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	EF9365(config, m_gdc, 1750000);
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm8000_hsg_a_device::hsg_a_map);
	m_gdc->set_palette_tag("palette");
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x512);
}

void cbm8000_hsg_b_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(EF9366_TAG, FUNC(ef9365_device::screen_update));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_refresh_hz(50);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	EF9365(config, m_gdc, 1750000); //EF9366
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm8000_hsg_b_device::hsg_b_map);
	m_gdc->set_palette_tag("palette");
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm8000_hsg_device - constructor
//-------------------------------------------------

cbm8000_hsg_device::cbm8000_hsg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pet_expansion_card_interface(mconfig, *this),
	m_gdc(*this, EF9365_TAG),
	m_9000(*this, "9000"),
	m_a000(*this, "a000")
{
}

cbm8000_hsg_a_device::cbm8000_hsg_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm8000_hsg_device(mconfig, CBM8000_HSG_A, tag, owner, clock)
{
}

cbm8000_hsg_b_device::cbm8000_hsg_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm8000_hsg_device(mconfig, CBM8000_HSG_B, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm8000_hsg_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm8000_hsg_device::device_reset()
{
	m_gdc->reset();
}


//-------------------------------------------------
//  pet_norom_r - NO ROM read
//-------------------------------------------------

int cbm8000_hsg_device::pet_norom_r(offs_t offset, int sel)
{
	return !(offset >= 0x9000 && offset < 0xaf00);
}


//-------------------------------------------------
//  pet_bd_r - buffered data read
//-------------------------------------------------

uint8_t cbm8000_hsg_device::pet_bd_r(offs_t offset, uint8_t data, int &sel)
{
	switch (sel)
	{
	case pet_expansion_slot_device::SEL9:
		data = m_9000->base()[offset & 0xfff];
		break;

	case pet_expansion_slot_device::SELA:
		if (offset < 0xaf00)
		{
			data = m_a000->base()[offset & 0xfff];
		}
		else if (offset == 0xaf10)
		{
			/*

			    bit     description

			    0       light pen
			    1
			    2
			    3
			    4
			    5
			    6
			    7

			*/
		}
		else if (offset == 0xad30)
		{
			// hard copy
		}
		else if (offset >= 0xaf70 && offset < 0xaf80)
		{
			data = m_gdc->data_r(offset & 0x0f);
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  pet_bd_w - buffered data write
//-------------------------------------------------

void cbm8000_hsg_device::pet_bd_w(offs_t offset, uint8_t data, int &sel)
{
	if (offset == 0xaf00)
	{
		/*

		    bit     description

		    0       hard copy (0=active)
		    1       operating page select (version B)
		    2
		    3       read-modify-write (1=active)
		    4       display switch (1=graphic)
		    5       display page select (version B)
		    6
		    7

		*/
	}
	else if (offset >= 0xaf70 && offset < 0xaf80)
	{
		m_gdc->data_w(offset & 0x0f, data);
	}
}
