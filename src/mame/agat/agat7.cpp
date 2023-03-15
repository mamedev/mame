// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7video.cpp

    Implementation of Agat-7 onboard video.

    5 video modes:
    - 32x32 color text
    - 64x32 mono text with reverse video
    - 64x64 color graphics
    - 128x128 color graphics
    - 256x256 mono graphics

    Character generator ROM could have 128 or 256 chars.

    C7xx: video mode select

*********************************************************************/

#include "emu.h"

#include "agat7.h"

#include "screen.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(AGAT7VIDEO, agat7video_device, "agat7video", "Agat-7 Video")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void agat7video_device::device_add_mconfig(machine_config &config)
{
	screen_device &a7screen(SCREEN(config, "a7screen", SCREEN_TYPE_RASTER));
	a7screen.set_raw(XTAL(10'500'000), 672, 0, 512, 312, 0, 256);
	a7screen.set_screen_update(FUNC(agat7video_device::screen_update));
	a7screen.set_palette(DEVICE_SELF);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

agat7video_device::agat7video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AGAT7VIDEO, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_ram_dev(*this, finder_base::DUMMY_TAG)
	, m_char_region(*this, finder_base::DUMMY_TAG)
	, m_char_ptr(nullptr)
	, m_char_size(0)
	, m_start_address(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void agat7video_device::device_start()
{
	m_char_ptr = m_char_region->base();
	m_char_size = m_char_region->bytes();

	// per http://agatcomp.ru/Reading/IiO/87-2-077.djvu
	for (int i = 0; 8 > i; ++i)
	{
		set_pen_color(i + 0, rgb_t(BIT(i, 0) ? 0xff : 0, BIT(i, 1) ? 0xff : 0, BIT(i, 2) ? 0xff : 0));
		set_pen_color(i + 8, rgb_t(BIT(i, 0) ? 0x7f : 0, BIT(i, 1) ? 0x7f : 0, BIT(i, 2) ? 0x7f : 0));
	}

//  save_item(NAME(m_video_mode));
	save_item(NAME(m_start_address));
}

void agat7video_device::device_reset()
{
	// TODO to be confirmed
	m_video_mode = TEXT_LORES;
	m_start_address = 0x7800;
}


uint8_t agat7video_device::read(offs_t offset)
{
	if(!machine().side_effects_disabled())
		do_io(offset);

	return 0;
}

void agat7video_device::write(offs_t offset, uint8_t data)
{
	do_io(offset);
}


void agat7video_device::do_io(int offset)
{
	switch (offset & 3)
	{
	case 0:
		m_video_mode = GRAPHICS_LORES;
		m_start_address = (offset & 0x3c) << 9;
		logerror("offset %02X -> %04X, video mode 0 (GRAPHICS_LORES)\n", offset, m_start_address);
		break;

	case 1:
		m_video_mode = GRAPHICS_HIRES;
		m_start_address = ((offset & 0x31) - 0x01) << 9;
		logerror("offset %02X -> %04X, video mode 1 (GRAPHICS_HIRES)\n", offset, m_start_address);
		break;

	case 2:
		if (offset > 0x80)
		{
			m_video_mode = TEXT_HIRES;
			m_start_address = (offset - 0x82) << 9;
			logerror("offset %02X -> %04X, video mode 2 (TEXT_HIRES)\n", offset, m_start_address);
		}
		else
		{
			m_video_mode = TEXT_LORES;
			m_start_address = (offset - 0x02) << 9;
			logerror("offset %02X -> %04X, video mode 2 (TEXT_LORES)\n", offset, m_start_address);
		}
		break;

	case 3:
		m_video_mode = GRAPHICS_MONO;
		m_start_address = ((offset - 0x03) & 0x30) << 9;
		logerror("offset %02X -> %04X, video mode 3 (GRAPHICS_MONO)\n", offset, m_start_address);
		break;
	}
}


void agat7video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	/* look up the character data */
	uint8_t const *const chardata = &textgfx_data[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			uint16_t const color = (chardata[y] & (1 << (7 - x))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void agat7video_device::text_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch, attr;
	int fg = 0;
	int bg = 0;

	beginrow = std::max(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = std::min(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 32; col++)
		{
			/* calculate address */
			address = m_start_address + (col * 2) + (row * 8);
			ch = m_ram_dev->read(address);
			attr = m_ram_dev->read(address + 1);
			fg = bitswap<8>(attr, 7, 6, 5, 3, 4, 2, 1, 0) & 15;
			if (BIT(attr, 5))
			{
				plot_text_character(bitmap, col * 16, row, 2, ch, m_char_ptr, m_char_size, fg, bg);
			}
			else
			{
				plot_text_character(bitmap, col * 16, row, 2, ch, m_char_ptr, m_char_size, bg, fg);
			}
		}
	}
}

void agat7video_device::text_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch;
	int fg, bg;

	beginrow = std::max(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = std::min(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	if (m_start_address & 0x800)
	{
		fg = 7;
		bg = 0;
	}
	else
	{
		fg = 0;
		bg = 7;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 64; col++)
		{
			/* calculate address */
			address = m_start_address + col + (row * 8);
			ch = m_ram_dev->read(address);
			plot_text_character(bitmap, col * 8, row, 1, ch, m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void agat7video_device::graph_update_mono(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int fg = 7, bg = 0;

	beginrow = std::max(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = std::min(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 32; col++)
		{
			uint32_t const address = m_start_address + col + (row * 0x20);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 8; b++)
			{
				uint8_t const v = (gfx & 0x80);
				gfx <<= 1;
				*(p++) = v ? fg : bg;
				*(p++) = v ? fg : bg;
			}
		}
	}
}

void agat7video_device::graph_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	beginrow = std::max(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = std::min(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 0x40; col++)
		{
			uint32_t const address = m_start_address + col + ((row / 2) * 0x40);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 2; b++)
			{
				uint8_t const v = (gfx & 0xf0) >> 4;
				gfx <<= 4;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}

void agat7video_device::graph_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	beginrow = std::max(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = std::min(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 0x20; col++)
		{
			uint32_t const address = m_start_address + col + ((row / 4) * 0x20);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 2; b++)
			{
				uint8_t const v = (gfx & 0xf0) >> 4;
				gfx <<= 4;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}


uint32_t agat7video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_video_mode)
	{
	case TEXT_LORES:
		text_update_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case TEXT_HIRES:
		text_update_hires(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_MONO:
		graph_update_mono(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_LORES:
		graph_update_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_HIRES:
		graph_update_hires(screen, bitmap, cliprect, 0, 255);
		break;

	default:
		graph_update_mono(screen, bitmap, cliprect, 0, 255);
		break;
	}

	return 0;
}
