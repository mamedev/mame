// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

**********************************************************************/

/*

    TODO:

    http://www.wfking.de/hires.htm

    - version A (EF9365, 512x512 interlaced, 1 page)
    - version B (EF9366, 512x256 non-interlaced, 2 pages)
    - 256KB version ROM

*/

#include "emu.h"
#include "hrg.h"
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

DEFINE_DEVICE_TYPE(CBM2_HRG_A, cbm2_hrg_a_device, "cbm2_hrga", "CBM 500/600/700 High Resolution Graphics (A)")
DEFINE_DEVICE_TYPE(CBM2_HRG_B, cbm2_hrg_b_device, "cbm2_hrgb", "CBM 500/600/700 High Resolution Graphics (B)")


//-------------------------------------------------
//  ROM( cbm2_hrg )
//-------------------------------------------------

ROM_START( cbm2_hrg )
	ROM_REGION( 0x2000, "bank3", 0 )
	ROM_LOAD( "324688-01 sw gr 600.bin", 0x0000, 0x2000, CRC(863e9ef8) SHA1(d75ffa97b2dd4e1baefe4acaa130daae866ab0e8) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cbm2_hrg_device::device_rom_region() const
{
	return ROM_NAME( cbm2_hrg );
}


//-------------------------------------------------
//  ADDRESS_MAP( hrg_a_map )
//-------------------------------------------------

void cbm2_hrg_a_device::hrg_a_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( hrg_b_map )
//-------------------------------------------------

void cbm2_hrg_b_device::hrg_b_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).ram();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cbm2_hrg_a_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(EF9365_TAG, FUNC(ef9365_device::screen_update));
	screen.set_size(512, 512);
	screen.set_visarea(0, 512-1, 0, 512-1);
	screen.set_refresh_hz(25);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	EF9365(config, m_gdc, 1750000);
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm2_hrg_a_device::hrg_a_map);
	m_gdc->set_palette_tag("palette");
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x512);
}

void cbm2_hrg_b_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(EF9366_TAG, FUNC(ef9365_device::screen_update));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_refresh_hz(50);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	EF9365(config, m_gdc, 1750000); //EF9366
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm2_hrg_b_device::hrg_b_map);
	m_gdc->set_palette_tag("palette");
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_hrg_device - constructor
//-------------------------------------------------

cbm2_hrg_device::cbm2_hrg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this),
	m_gdc(*this, EF9366_TAG),
	m_bank3(*this, "bank3")
{
}

cbm2_hrg_a_device::cbm2_hrg_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm2_hrg_device(mconfig, CBM2_HRG_A, tag, owner, clock)
{
}

cbm2_hrg_b_device::cbm2_hrg_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm2_hrg_device(mconfig, CBM2_HRG_B, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_hrg_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_hrg_device::device_reset()
{
	m_gdc->reset();
}


//-------------------------------------------------
//  cbm2_bd_r - cartridge data read
//-------------------------------------------------

uint8_t cbm2_hrg_device::cbm2_bd_r(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank3)
	{
		if (offset < 0x7f80)
		{
			data = m_bank3->base()[offset & 0x1fff];
		}
		else if (offset == 0x7f90)
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
		else if (offset == 0x7fb0)
		{
			// hard copy
		}
		else if (offset >= 0x7ff0)
		{
			data = m_gdc->data_r(machine().dummy_space(), offset & 0x0f);
		}
	}

	return data;
}


//-------------------------------------------------
//  cbm2_bd_w - cartridge data write
//-------------------------------------------------

void cbm2_hrg_device::cbm2_bd_w(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank3)
	{
		if (offset == 0x7f80)
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
		else if (offset >= 0x7ff0)
		{
			m_gdc->data_w(machine().dummy_space(), offset & 0x0f, data);
		}
	}
}
