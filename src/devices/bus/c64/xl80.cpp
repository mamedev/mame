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

#include "emu.h"
#include "xl80.h"
#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define RAM_SIZE            0x800

#define HD46505SP_TAG       "mc6845"
#define MC6845_SCREEN_TAG   "screen80"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_XL80, c64_xl80_device, "c64_xl80", "C64 XL 80 cartridge")


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

const tiny_rom_entry *c64_xl80_device::device_rom_region() const
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
		uint8_t code = m_ram[((ma + column) & 0x7ff)];
		uint16_t addr = (code << 3) | (ra & 0x07);
		uint8_t data = m_char_rom->base()[addr & 0x7ff];

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

static GFXDECODE_START( gfx_c64_xl80 )
	GFXDECODE_ENTRY(HD46505SP_TAG, 0x0000, gfx_8x8x1, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_xl80_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, MC6845_SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::white()));
	screen.set_screen_update(HD46505SP_TAG, FUNC(hd6845s_device::screen_update));
	screen.set_size(80*8, 24*8);
	screen.set_visarea(0, 80*8-1, 0, 24*8-1);
	screen.set_refresh_hz(50);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_c64_xl80);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, XTAL(14'318'181) / 8);
	m_crtc->set_screen(MC6845_SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(c64_xl80_device::crtc_update_row));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_xl80_device - constructor
//-------------------------------------------------

c64_xl80_device::c64_xl80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_XL80, tag, owner, clock),
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

uint8_t c64_xl80_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && BIT(offset, 2))
	{
		if (offset & 0x01)
		{
			data = m_crtc->register_r();
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

void c64_xl80_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (offset >= 0x9800 && offset < 0xa000)
	{
		m_ram[offset & 0x7ff] = data;
	}
	else if (!io2 && BIT(offset, 2))
	{
		if (offset & 0x01)
		{
			m_crtc->register_w(data);
		}
		else
		{
			m_crtc->address_w(data);
		}
	}
}
