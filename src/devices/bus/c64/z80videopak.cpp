// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/*

Z80 Video Pak
(c) 1983 Data 20 Corporation

*/

#include "emu.h"
#include "z80videopak.h"
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

DEFINE_DEVICE_TYPE(C64_Z80VIDEOPAK, c64_z80videopak_device, "c64_z80videopak", "Data20 Z80 Video Pak")


//-------------------------------------------------
//  ROM( c64_z80videopak )
//-------------------------------------------------

ROM_START( c64_z80videopak )
	ROM_REGION( 0x800, HD46505SP_TAG, 0 )
	ROM_LOAD( "c68297 vid pak cg.u18", 0x000, 0x800, CRC(9edf5e58) SHA1(4b244e6d94a7653a2e52c351589f0b469119fb04) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_z80videopak_device::device_rom_region() const
{
	return ROM_NAME( c64_z80videopak );
}

//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( c64_z80videopak_device::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t const code = m_ram[((ma + column) & 0x7ff)];
		uint16_t const addr = (m_case << 10) | ((code & 0x7f) << 3) | (ra & 0x07);
		uint8_t data = m_char_rom->base()[addr & 0x7ff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int const x = (column * 8) + bit;
			int const color = BIT(data ^ code, 7) && de;

			bitmap.pix(vbp + y, hbp + x) = pen[color];

			data <<= 1;
		}
	}
}

//-------------------------------------------------
//  GFXDECODE( c64_videopak )
//-------------------------------------------------

static GFXDECODE_START( gfx_c64_z80videopak )
	GFXDECODE_ENTRY(HD46505SP_TAG, 0x0000, gfx_8x8x1, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_z80videopak_device::device_add_mconfig(machine_config &config)
{
	c64_cpm_cartridge_device::device_add_mconfig(config);

	screen_device &screen(SCREEN(config, MC6845_SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::white()));
	screen.set_screen_update(HD46505SP_TAG, FUNC(hd6845s_device::screen_update));
	screen.set_size(80*8, 24*8);
	screen.set_visarea(0, 80*8-1, 0, 24*8-1);
	screen.set_refresh_hz(50);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_c64_z80videopak);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, XTAL(14'318'181) / 8);
	m_crtc->set_screen(MC6845_SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(c64_z80videopak_device::crtc_update_row));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_z80videopak_device - constructor
//-------------------------------------------------

c64_z80videopak_device::c64_z80videopak_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	c64_cpm_cartridge_device(mconfig, C64_Z80VIDEOPAK, tag, owner, clock),
	m_crtc(*this, HD46505SP_TAG),
	m_palette(*this, "palette"),
	m_char_rom(*this, HD46505SP_TAG),
	m_ram(*this, "ram", RAM_SIZE, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_z80videopak_device::device_start()
{
	// state saving
	save_item(NAME(m_case));

	c64_cpm_cartridge_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_z80videopak_device::device_reset()
{
	m_case = 0;
	m_exrom = 0;

	c64_cpm_cartridge_device::device_reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_z80videopak_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		if (BIT(offset, 11))
			data = m_ram[offset & 0x7ff];
		else
			data = m_roml[offset & 0x7ff];
	}
	else if (!io2 && !BIT(offset, 1))
	{
		if (BIT(offset, 0))
			data = m_crtc->register_r();
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_z80videopak_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	c64_cpm_cartridge_device::c64_cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);

	if ((!m_exrom && offset >= 0x9800 && offset < 0xa000) ||
		(m_exrom && offset >= 0xf800 && offset < 0x10000))
	{
		m_ram[offset & 0x7ff] = data;
	}
	else if (!io2 && !BIT(offset, 1))
	{
		if (!BIT(offset, 0))
			m_crtc->address_w(data);
		else
			m_crtc->register_w(data);
	}
	else if (!io2)
	{
		m_case = BIT(data, 0);
		// BIT(data, 1); // unknown
		// BIT(data, 2); // unknown
		m_exrom = BIT(data, 4);
	}
}
