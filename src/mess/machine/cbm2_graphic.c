/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - version A (EF9365, 512x512 interlaced, 1 page)
    - version B (EF9366, 512x256 non-interlaced, 2 pages)

*/

#include "cbm2_graphic.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define EF9365_TAG	"ef9365"
#define EF9366_TAG	"ef9366"
#define SCREEN_TAG	"screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_GRAPHIC = &device_creator<cbm2_graphic_cartridge_device>;


//-------------------------------------------------
//  ef9365_interface gdp_intf
//-------------------------------------------------
/*
static const ef9365_interface gdp_intf =
{
    SCREEN_TAG
};
*/

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm2_graphic_a )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm2_graphic_a )
/*  MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_UPDATE_DEVICE(EF9365_TAG, ef9365_device, screen_update)
    MCFG_SCREEN_SIZE(512, 512)
    MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
    MCFG_SCREEN_REFRESH_RATE(50)

    MCFG_EF9365_ADD(EF9365_TAG, gdp_intf)*/
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( cbm2_graphic_b )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cbm2_graphic_b )
/*  MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_UPDATE_DEVICE(EF9366_TAG, ef9366_device, screen_update)
    MCFG_SCREEN_SIZE(512, 256)
    MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
    MCFG_SCREEN_REFRESH_RATE(50)

    MCFG_EF9366_ADD(EF9366_TAG, gdp_intf)*/
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor cbm2_graphic_cartridge_device::device_mconfig_additions() const
{
	switch (m_variant)
	{
	default: return MACHINE_CONFIG_NAME( cbm2_graphic_a );
	case TYPE_B: return MACHINE_CONFIG_NAME( cbm2_graphic_b );
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_graphic_cartridge_device - constructor
//-------------------------------------------------

cbm2_graphic_cartridge_device::cbm2_graphic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CBM2_GRAPHIC, "CBM 500/600/700 High Resolution Graphics", tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this),
	//m_gdc(*this, EF9365_TAG),
	m_variant(TYPE_A)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_graphic_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_graphic_cartridge_device::device_reset()
{
	//m_gdc->reset();
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
			data = m_bank3[offset];
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
			//data = m_gdc->data_r(space, offset & 0x07);
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
			//m_gdc->data_w(space, offset & 0x07, data);
		}
	}
}
