// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 80-Column Card emulation

**********************************************************************/

/*

(c) 1985 Comx World Operations

PCB Layout
----------

F-003-CLM-REV 1

    |---------------|
    |      CN1      |
|---|               |---------------------------|
|                                               |
|  MC14174  LS86    LS175   LS10    LS161       |
|                                   14.31818MHz |
|                           LS245   LS04        |
|    ROM1       6845                        CN2 |
|                           LS374   LS165       |
|LD2 LS138  LS157   LS157                       |
|LD1                        6116    ROM2    SW1 |
|    LS126  LS32    LS157                       |
|-----------------------------------------------|

Notes:
    All IC's shown.

    6845    - Motorola MC6845P CRT Controller
    6116    - Motorola MCM6116P15 2Kx8 Asynchronous CMOS Static RAM
    ROM1    - Mitsubishi 2Kx8 EPROM "C"
    ROM2    - Mitsubishi 2Kx8 EPROM "P"
    CN1     - COMX-35 bus PCB edge connector
    CN2     - RCA video output connector
    LD1     - LED
    LD2     - LED
    SW1     - switch

*/

#include "clm.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6845_TAG          "mc6845"
#define MC6845_SCREEN_TAG   "screen80"
#define VIDEORAM_SIZE       0x800



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_CLM = &device_creator<comx_clm_device>;


//-------------------------------------------------
//  ROM( comx_clm )
//-------------------------------------------------

ROM_START( comx_clm )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_DEFAULT_BIOS( "v11" )
	ROM_SYSTEM_BIOS( 0, "v10", "v1.0" )
	ROMX_LOAD( "p 1.0.cl1", 0x0000, 0x0800, CRC(b417d30a) SHA1(d428b0467945ecb9aec884211d0f4b1d8d56d738), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v11", "v1.1" )
	ROMX_LOAD( "p 1.1.cl1", 0x0000, 0x0800, CRC(0a2eaf19) SHA1(3f1f640caef964fb47aaa147cab6d215c2b30e9d), ROM_BIOS(2) )

	ROM_REGION( 0x800, MC6845_TAG, 0 )
	ROMX_LOAD( "c 1.0.cl4", 0x0000, 0x0800, CRC(69dd7b07) SHA1(71d368adbb299103d165eab8359a97769e463e26), ROM_BIOS(1) )
	ROMX_LOAD( "c 1.1.cl4", 0x0000, 0x0800, CRC(dc9b5046) SHA1(4e041cec03dda6dba5e2598d060c49908a4fab2a), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_clm_device::device_rom_region() const
{
	return ROM_NAME( comx_clm );
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( comx_clm_device::crtc_update_row )
{
	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_video_ram[((ma + column) & 0x7ff)];
		UINT16 addr = (code << 3) | (ra & 0x07);
		UINT8 data = m_char_rom->base()[addr & 0x7ff];

		if (BIT(ra, 3) && column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;

			bitmap.pix32(vbp + y, hbp + x) = m_palette->pen(BIT(data, 7) && de);

			data <<= 1;
		}
	}
}

//-------------------------------------------------
//  GFXDECODE( comx_clm )
//-------------------------------------------------

static GFXDECODE_START( comx_clm )
	GFXDECODE_ENTRY(MC6845_TAG, 0x0000, gfx_8x8x1, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( comx_clm )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( comx_clm )
	MCFG_SCREEN_ADD(MC6845_SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*8-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", comx_clm)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, MC6845_SCREEN_TAG, XTAL_14_31818MHz/7)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(comx_clm_device, crtc_update_row)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor comx_clm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( comx_clm );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_clm_device - constructor
//-------------------------------------------------

comx_clm_device::comx_clm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_CLM, "COMX 80 Column Card", tag, owner, clock, "comx_clm", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_palette(*this, "palette"),
	m_rom(*this, "c000"),
	m_char_rom(*this, MC6845_TAG),
	m_video_ram(*this, "video_ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_clm_device::device_start()
{
	// allocate memory
	m_video_ram.allocate(VIDEORAM_SIZE);

	// state saving
	save_item(NAME(m_ds));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_clm_device::device_reset()
{
}


//-------------------------------------------------
//  comx_ef4_r - external flag 4 read
//-------------------------------------------------

int comx_clm_device::comx_ef4_r()
{
	return m_ds ? m_crtc->hsync_r() : CLEAR_LINE;
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_clm_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0xff;

	if (offset >= 0xc000 && offset < 0xc800)
	{
		data = m_rom->base()[offset & 0x7ff];
	}
	else if (offset >= 0xd000 && offset < 0xd800)
	{
		data = m_video_ram[offset & 0x7ff];
	}
	else if (offset == 0xd801)
	{
		data = m_crtc->register_r(space, 0);
	}

	return data;
}


//-------------------------------------------------
//  comx_mwr_w - memory write
//-------------------------------------------------

void comx_clm_device::comx_mwr_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset >= 0xd000 && offset < 0xd800)
	{
		m_video_ram[offset & 0x7ff] = data;
	}
	else if (offset == 0xd800)
	{
		m_crtc->address_w(space, 0, data);
	}
	else if (offset == 0xd801)
	{
		m_crtc->register_w(space, 0, data);
	}
}
