// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Data 20 Corporation Video Pak cartridge emulation
    aka Data 20 Display Manager aka Protecto 40/80

**********************************************************************/

#include "emu.h"
#include "videopak.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VIDEORAM_SIZE       0x800
#define RAM_SIZE            0x10000

#define MC6845_TAG          "mc6845"
#define MC6845_SCREEN_TAG   "screen80"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_VIDEO_PAK, vic20_video_pak_device, "vic20_videopak", "Data 20 Video Pak")


//-------------------------------------------------
//  ROM( videopak )
//-------------------------------------------------

ROM_START( videopak )
	ROM_REGION( 0x800, MC6845_TAG, 0 )
	// ROM has been borrowed from the C64 XL80 cartridge
	ROM_LOAD( "chargen", 0x000, 0x800, BAD_DUMP CRC(9edf5e58) SHA1(4b244e6d94a7653a2e52c351589f0b469119fb04) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic20_video_pak_device::device_rom_region() const
{
	return ROM_NAME( videopak );
}

//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( vic20_video_pak_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_videoram[((ma + column) & 0x7ff)];
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
//  GFXDECODE( vic20_video_pak )
//-------------------------------------------------

static GFXDECODE_START( gfx_vic20_video_pak )
	GFXDECODE_ENTRY(MC6845_TAG, 0x0000, gfx_8x8x1, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic20_video_pak_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, MC6845_SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::white()));
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_size(80*8, 24*8);
	screen.set_visarea(0, 80*8-1, 0, 24*8-1);
	screen.set_refresh_hz(50);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_vic20_video_pak);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, XTAL(14'318'181) / 8); // HD46505RP or similar
	m_crtc->set_screen(MC6845_SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(vic20_video_pak_device::crtc_update_row));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_video_pak_device - constructor
//-------------------------------------------------

vic20_video_pak_device::vic20_video_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC20_VIDEO_PAK, tag, owner, clock),
	device_vic20_expansion_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_palette(*this, "palette"),
	m_char_rom(*this, MC6845_TAG),
	m_videoram(*this, "videoram"),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_video_pak_device::device_start()
{
	// allocate memory
	m_videoram.allocate(VIDEORAM_SIZE);
	m_ram.allocate(RAM_SIZE);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_video_pak_device::device_reset()
{
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic20_video_pak_device::vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!m_ram_enable)
	{
		if (m_bank_size)
		{
			if (!blk1)
			{
				offs_t addr = m_bank_msb << 15 | m_bank_lsb << 14 | offset;
				data = m_ram[addr];
			}

			if (!blk2)
			{
				offs_t addr = m_bank_msb << 15 | m_bank_lsb << 14 | 0x2000 | offset;
				data = m_ram[addr];
			}
		}
		else
		{
			if (!blk1)
			{
				offs_t addr = m_bank_msb << 15 | offset;
				data = m_ram[addr];
			}

			if (!blk2)
			{
				offs_t addr = m_bank_msb << 15 | 0x2000 | offset;
				data = m_ram[addr];
			}

			if (!blk3)
			{
				offs_t addr = m_bank_msb << 15 | 0x4000 | offset;
				data = m_ram[addr];
			}
		}
	}

	if (!blk5)
	{
		switch ((offset >> 11) & 0x03)
		{
		case 0:
			if (m_blk5)
				data = m_blk5[offset & 0x7ff];
			break;

		case 3:
			data = m_videoram[offset & 0x7ff];
			break;
		}
	}

	if (!io2)
	{
		if (offset == 0x1bf9)
		{
			data = m_crtc->register_r();
		}
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic20_video_pak_device::vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!m_ram_enable)
	{
		if (m_bank_size)
		{
			if (!blk1)
			{
				offs_t addr = m_bank_msb << 15 | m_bank_lsb << 14 | offset;

				m_ram[addr] = data;
			}

			if (!blk2)
			{
				offs_t addr = m_bank_msb << 15 | m_bank_lsb << 14 | 0x2000 | offset;

				m_ram[addr] = data;
			}
		}
		else
		{
			if (!blk1)
			{
				offs_t addr = m_bank_msb << 15 | offset;

				m_ram[addr] = data;
			}

			if (!blk2)
			{
				offs_t addr = m_bank_msb << 15 | 0x2000 | offset;

				m_ram[addr] = data;
			}

			if (!blk3)
			{
				offs_t addr = m_bank_msb << 15 | 0x4000 | offset;

				m_ram[addr] = data;
			}
		}
	}

	if (!blk5)
	{
		switch ((offset >> 11) & 0x03)
		{
		case 3:
			m_videoram[offset & 0x7ff] = data;
			break;
		}
	}

	if (!io2)
	{
		switch (offset)
		{
		case 0x1bf8:
			m_crtc->address_w(data);
			break;

		case 0x1bf9:
			m_crtc->register_w(data);
			break;

		case 0x1bfc:
			/*

			    bit     description

			    0       0 = upper case, 1 = lower case
			    1       bank size: 0 = 2x24KB, 1 = 4x16KB
			    2       16KB mode address LSB
			    3       memory address MSB
			    4       0 = enable RAM, 1 = disable RAM
			    5       0 = 40 columns, 1 = 80 columns (Data 20 Video Manager)

			*/

			m_case = BIT(data, 0);
			m_bank_size = BIT(data, 1);
			m_bank_lsb = BIT(data, 2);
			m_bank_msb = BIT(data, 3);
			m_ram_enable = BIT(data, 4);
			m_columns = BIT(data, 5);
			break;
		}
	}
}
