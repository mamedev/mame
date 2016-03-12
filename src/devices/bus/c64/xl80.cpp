// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

XL 80 cartridge
(c) 1984 Data 20 Corporation

PCB Layout
----------

        |==================================|
        |   LS175    LS20    LS139         |
        |   LS157                          |
|=======|                                  |
|=|                          RAM           |
|=|    LS157      LS157             LS165  |
|=|                                        |
|=|                                     CN1|
|=|          CRTC            ROM1          |
|=|                                        |
|=|    ROM0          LS245   LS151         |
|=|                             14.31818MHz|
|=======|            LS174   LS00          |
        |                           HCU04  |
        |    LS74    LS161   LS74          |
        |==================================|

Notes:
    All IC's shown.

    CRTC    - Hitachi HD46505SP
    RAM     - Toshiba TMM2016AP-12 2Kx8 Static RAM
    ROM0    - GI 9433CS-0090 8Kx8 ROM
    ROM1    - GI 9316CS-F67 2Kx8 ROM "DTC"
    CN1     - RCA video output

*/

#include "xl80.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define RAM_SIZE            0x800

#define HD46505SP_TAG       "mc6845"
#define MC6845_SCREEN_TAG   "screen80"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_XL80 = &device_creator<c64_xl80_device>;


//-------------------------------------------------
//  ROM( c64_xl80 )
//-------------------------------------------------

ROM_START( c64_xl80 )
	ROM_REGION( 0x800, HD46505SP_TAG, 0 )
	ROM_LOAD( "dtc.u14", 0x000, 0x800, CRC(9edf5e58) SHA1(4b244e6d94a7653a2e52c351589f0b469119fb04) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_xl80_device::device_rom_region() const
{
	return ROM_NAME( c64_xl80 );
}

//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( c64_xl80_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_ram[((ma + column) & 0x7ff)];
		UINT16 addr = (code << 3) | (ra & 0x07);
		UINT8 data = m_char_rom->base()[addr & 0x7ff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(data, 7) && de;

			bitmap.pix32(vbp + y, hbp + x) = pen[color];

			data <<= 1;
		}
	}
}

//-------------------------------------------------
//  GFXDECODE( c64_xl80 )
//-------------------------------------------------

static GFXDECODE_START( c64_xl80 )
	GFXDECODE_ENTRY(HD46505SP_TAG, 0x0000, gfx_8x8x1, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_xl80 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_xl80 )
	MCFG_SCREEN_ADD_MONOCHROME(MC6845_SCREEN_TAG, RASTER, rgb_t::white)
	MCFG_SCREEN_UPDATE_DEVICE(HD46505SP_TAG, h46505_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*8-1)
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", c64_xl80)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_MC6845_ADD(HD46505SP_TAG, H46505, MC6845_SCREEN_TAG, XTAL_14_31818MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(c64_xl80_device, crtc_update_row)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_xl80_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_xl80 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_xl80_device - constructor
//-------------------------------------------------

c64_xl80_device::c64_xl80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_XL80, "XL 80", tag, owner, clock, "c64_xl80", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_crtc(*this, HD46505SP_TAG),
	m_palette(*this, "palette"),
	m_char_rom(*this, HD46505SP_TAG),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_xl80_device::device_start()
{
	// allocate memory
	m_ram.allocate(RAM_SIZE);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_xl80_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_xl80_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && BIT(offset, 2))
	{
		if (offset & 0x01)
		{
			data = m_crtc->register_r(space, 0);
		}
	}
	else if (offset >= 0x8000 && offset < 0x9000)
	{
		data = m_roml[offset & 0xfff];
	}
	else if (offset >= 0x9800 && offset < 0xa000)
	{
		data = m_ram[offset & 0x7ff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_xl80_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (offset >= 0x9800 && offset < 0xa000)
	{
		m_ram[offset & 0x7ff] = data;
	}
	else if (!io2 && BIT(offset, 2))
	{
		if (offset & 0x01)
		{
			m_crtc->register_w(space, 0, data);
		}
		else
		{
			m_crtc->address_w(space, 0, data);
		}
	}
}
