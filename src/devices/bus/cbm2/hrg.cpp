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

*/

#include "hrg.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define EF9365_TAG  "ef9365"
#define EF9366_TAG  EF9365_TAG
#define SCREEN_TAG  "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_HRG_A = &device_creator<cbm2_graphic_cartridge_a_device>;
const device_type CBM2_HRG_B = &device_creator<cbm2_graphic_cartridge_b_device>;


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

const rom_entry *cbm2_graphic_cartridge_device::device_rom_region() const
{
	return ROM_NAME( cbm2_hrg );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm2_hrg_a )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm2_hrg_a )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(EF9365_TAG, ef9365_device, screen_update)
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
	MCFG_SCREEN_REFRESH_RATE(25)

	MCFG_DEVICE_ADD(EF9365_TAG, EF9365, 1750000)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_EF936X_PALETTE("palette")
	MCFG_EF936X_BITPLANES_CNT(1);
	MCFG_EF936X_DISPLAYMODE(EF936X_512x512_DISPLAY_MODE);
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm2_hrg_b )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm2_hrg_b )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(EF9366_TAG, ef9365_device, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_DEVICE_ADD(EF9366_TAG, EF9365, 1750000)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_EF936X_PALETTE("palette")
	MCFG_EF936X_BITPLANES_CNT(1);
	MCFG_EF936X_DISPLAYMODE(EF936X_512x256_DISPLAY_MODE);
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor cbm2_graphic_cartridge_a_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cbm2_hrg_a );
}

machine_config_constructor cbm2_graphic_cartridge_b_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cbm2_hrg_b );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_graphic_cartridge_device - constructor
//-------------------------------------------------

cbm2_graphic_cartridge_device::cbm2_graphic_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_cbm2_expansion_card_interface(mconfig, *this),
	m_gdc(*this, EF9366_TAG),
	m_bank3(*this, "bank3")
{
}

cbm2_graphic_cartridge_a_device::cbm2_graphic_cartridge_a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cbm2_graphic_cartridge_device(mconfig, CBM2_HRG_A, "CBM 500/600/700 High Resolution Graphics (A)", tag, owner, clock, "cbm2_hrga", __FILE__)
{
}

cbm2_graphic_cartridge_b_device::cbm2_graphic_cartridge_b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cbm2_graphic_cartridge_device(mconfig, CBM2_HRG_B, "CBM 500/600/700 High Resolution Graphics (B)", tag, owner, clock, "cbm2_hrgb", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_graphic_cartridge_device::device_start()
{
	m_gdc->set_color_entry(0, 0, 0, 0);
	m_gdc->set_color_entry(1, 0, 0xff, 00);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_graphic_cartridge_device::device_reset()
{
	m_gdc->reset();
}


//-------------------------------------------------
//  cbm2_bd_r - cartridge data read
//-------------------------------------------------

UINT8 cbm2_graphic_cartridge_device::cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
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
			data = m_gdc->data_r(space, offset & 0x07);
		}
	}

	return data;
}


//-------------------------------------------------
//  cbm2_bd_w - cartridge data write
//-------------------------------------------------

void cbm2_graphic_cartridge_device::cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
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
			m_gdc->data_w(space, offset & 0x07, data);
		}
	}
}
