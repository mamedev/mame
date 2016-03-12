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

#include "hsg.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define EF9365_TAG  "ef9365"
#define EF9366_TAG  EF9365_TAG
#define SCREEN_TAG  "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM8000_HSG_A = &device_creator<cbm8000_hsg_a_t>;
const device_type CBM8000_HSG_B = &device_creator<cbm8000_hsg_b_t>;


//-------------------------------------------------
//  ROM( cbm8000_hsg )
//-------------------------------------------------

ROM_START( cbm8000_hsg )
	ROM_REGION( 0x1000, "9000", 0 )
	ROM_LOAD( "pet_hsg-ud12 on 8032 9000 (2532).bin", 0x0000, 0x1000, CRC(d651bf72) SHA1(d3d68228a5a8ec73fb39be860c00edb0d21bd1a9) )

	ROM_REGION( 0x1000, "a000", 0 )
	ROM_LOAD( "324381-01 rev b sw graphi", 0x0000, 0x1000, CRC(c8e3bff9) SHA1(12ed3176ddd632f52e91082ab574adcba2149684) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cbm8000_hsg_t::device_rom_region() const
{
	return ROM_NAME( cbm8000_hsg );
}


//-------------------------------------------------
//  ADDRESS_MAP( hsg_a_map )
//-------------------------------------------------

static ADDRESS_MAP_START( hsg_a_map, AS_0, 8, cbm8000_hsg_a_t )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x7fff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( hsg_b_map )
//-------------------------------------------------

static ADDRESS_MAP_START( hsg_b_map, AS_0, 8, cbm8000_hsg_b_t )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x3fff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm8000_hsg_a )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm8000_hsg_a )
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DEVICE(EF9365_TAG, ef9365_device, screen_update)
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
	MCFG_SCREEN_REFRESH_RATE(25)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD(EF9365_TAG, EF9365, 1750000)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, hsg_a_map)
	MCFG_EF936X_PALETTE("palette")
	MCFG_EF936X_BITPLANES_CNT(1);
	MCFG_EF936X_DISPLAYMODE(EF936X_512x512_DISPLAY_MODE);
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm8000_hsg_b )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm8000_hsg_b )
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DEVICE(EF9366_TAG, ef9365_device, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD(EF9366_TAG, EF9365, 1750000)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, hsg_b_map)
	MCFG_EF936X_PALETTE("palette")
	MCFG_EF936X_BITPLANES_CNT(1);
	MCFG_EF936X_DISPLAYMODE(EF936X_512x256_DISPLAY_MODE);
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor cbm8000_hsg_a_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cbm8000_hsg_a );
}

machine_config_constructor cbm8000_hsg_b_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cbm8000_hsg_b );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm8000_hsg_t - constructor
//-------------------------------------------------

cbm8000_hsg_t::cbm8000_hsg_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_pet_expansion_card_interface(mconfig, *this),
	m_gdc(*this, EF9365_TAG),
	m_9000(*this, "9000"),
	m_a000(*this, "a000")
{
}

cbm8000_hsg_a_t::cbm8000_hsg_a_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cbm8000_hsg_t(mconfig, CBM8000_HSG_A, "CBM 8000 High Speed Graphics (A)", tag, owner, clock, "cbm8000_hsg_a", __FILE__)
{
}

cbm8000_hsg_b_t::cbm8000_hsg_b_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cbm8000_hsg_t(mconfig, CBM8000_HSG_B, "CBM 8000 High Speed Graphics (B)", tag, owner, clock, "cbm8000_hsg_b", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm8000_hsg_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm8000_hsg_t::device_reset()
{
	m_gdc->reset();
}


//-------------------------------------------------
//  pet_norom_r - NO ROM read
//-------------------------------------------------

int cbm8000_hsg_t::pet_norom_r(address_space &space, offs_t offset, int sel)
{
	return !(offset >= 0x9000 && offset < 0xaf00);
}


//-------------------------------------------------
//  pet_bd_r - buffered data read
//-------------------------------------------------

UINT8 cbm8000_hsg_t::pet_bd_r(address_space &space, offs_t offset, UINT8 data, int &sel)
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
			data = m_gdc->data_r(space, offset & 0x0f);
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  pet_bd_w - buffered data write
//-------------------------------------------------

void cbm8000_hsg_t::pet_bd_w(address_space &space, offs_t offset, UINT8 data, int &sel)
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
		m_gdc->data_w(space, offset & 0x0f, data);
	}
}
