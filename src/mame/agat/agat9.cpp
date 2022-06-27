// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat9video.cpp

    Implementation of Agat-9 onboard video.

    6 native video modes:
    - 32x32 color text
    - 64x32 mono text with reverse video
    - 128x128 16 color graphics
    - 256x256 4 color graphics
    - 256x256 and 512x256 mono graphics

    2 apple video modes: 40col text and HGR.

    C7xx: video mode select

*********************************************************************/

#include "emu.h"

#include "video/agat9.h"

#include "screen.h"


#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define PURPLE  5
#define CYAN    6
#define WHITE   7


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(AGAT9VIDEO, agat9video_device, "agat9video", "Agat-9 Video")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void agat9video_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(10'500'000), 672, 0, 512, 312, 0, 256);
	m_screen->set_screen_update(FUNC(agat9video_device::screen_update));
	m_screen->set_palette(DEVICE_SELF);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

agat9video_device::agat9video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AGAT9VIDEO, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_screen(*this, "a9screen")
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

void agat9video_device::device_start()
{
	int i, j;
	uint16_t c;

	static const uint8_t hires_artifact_color_table[] =
	{
		BLACK,  PURPLE, GREEN,  WHITE,
		BLACK,  BLUE,   RED,    WHITE
	};

	m_char_ptr = m_char_region->base();
	m_char_size = m_char_region->bytes();

	/* 2^3 dependent pixels * 2 color sets * 2 offsets */
	m_hires_artifact_map = std::make_unique<uint16_t[]>(8 * 2 * 2);

	/* build hires artifact map */
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (i & 0x02)
			{
				if ((i & 0x05) != 0)
					c = 3;
				else
					c = j ? 2 : 1;
			}
			else
			{
				c = 0;
			}
			m_hires_artifact_map[0 + j * 8 + i] = hires_artifact_color_table[(c + 0) % 8];
			m_hires_artifact_map[16 + j * 8 + i] = hires_artifact_color_table[(c + 4) % 8];
		}
	}

	// per http://agatcomp.ru/Reading/IiO/87-2-077.djvu
	for (int i = 0; 8 > i; ++i)
	{
		set_pen_color(i + 0, rgb_t(BIT(i, 0) ? 0xff : 0, BIT(i, 1) ? 0xff : 0, BIT(i, 2) ? 0xff : 0));
		set_pen_color(i + 8, rgb_t(BIT(i, 0) ? 0x7f : 0, BIT(i, 1) ? 0x7f : 0, BIT(i, 2) ? 0x7f : 0));
	}

	save_item(NAME(m_start_address));

	save_item(NAME(m_page2));
	save_item(NAME(m_flash));
	save_item(NAME(m_mix));
	save_item(NAME(m_graphics));
}

void agat9video_device::device_reset()
{
	// TODO to be confirmed
	m_video_mode = TEXT_LORES;
	m_start_address = 0x7800;
	m_mode = palette_index = 0;

	// apple
	m_page2 = false;
	m_flash = false;
	m_mix = false;
	m_graphics = false;
}


uint8_t agat9video_device::read(offs_t offset)
{
	if(!machine().side_effects_disabled())
		do_io(offset);
	// FIXME only 'Moscow' revision
	return m_mode;
}

void agat9video_device::write(offs_t offset, uint8_t data)
{
	do_io(offset);
}

uint8_t agat9video_device::apple_read(offs_t offset)
{
	logerror("%s: %04x read (%s)\n", machine().describe_context(), 0xc050 + offset, offset<8?"apple":"palette");

	if(!machine().side_effects_disabled())
		do_apple_io(offset);
	// XXX
	return m_mode;
}

void agat9video_device::apple_write(offs_t offset, uint8_t data)
{
	logerror("%s: %04x write (%s)\n", machine().describe_context(), 0xc050 + offset, offset<8?"apple":"palette");

	do_apple_io(offset);
}

void agat9video_device::do_apple_io(int offset)
{
	if (offset < 0x8)
	{
		m_video_mode = APPLE;
		m_screen->set_visible_area(0, 280 - 1, 0, 192 - 1);
	}

	switch (offset)
	{
	case 0x0:
		m_graphics = true;
		break;

	case 0x1:
		m_graphics = false;
		break;

	case 0x2:
		m_mix = false;
		break;

	case 0x3:
		m_mix = true;
		break;

	case 0x4:
		m_page2 = false;
		break;

	case 0x5:
		m_page2 = true;
		break;

	case 0x8:
		palette_index &= 0xfe;
		break;

	case 0x9:
		palette_index |= 0x01;
		break;

	case 0xa:
		palette_index &= 0xfd;
		break;

	case 0xb:
		palette_index |= 0x02;
		break;
	}
}

void agat9video_device::do_io(int offset)
{
	logerror("%s: %04x access, ", machine().describe_context(), 0xc700 + offset);

	m_mode = offset;

	m_screen->set_visible_area(0, 2 * 256 - 1, 0, 256 - 1);

	switch (offset & 3)
	{
	case 0:
		// 256x256, 4 colors, 64 bytes per scanline, 0x4000 page length, odd scanlines at 0x2000
		m_video_mode = GRAPHICS_COLOR_HIRES;
		m_start_address = ((offset & 0x60) << 9) + ((offset & 0x08) << 13);
		logerror("offset %04X, video mode 0 (COLOR_HIRES)\n", m_start_address);
		break;

	case 1:
		// 128x128, 16 colors, 64 bytes per scanline, 0x2000 page length, linear
		m_video_mode = GRAPHICS_COLOR_LORES;
		m_start_address = ((offset & 0x70) << 9) + ((offset & 0x08) << 13);
		logerror("offset %04X, video mode 1 (COLOR_LORES)\n", m_start_address);
		break;

	// b6..b4 == page number, b3..b2 == subpage number
	case 2:
		// 64x32, 0x1000 page length
		if (offset > 0x80)
		{
			m_video_mode = TEXT_HIRES;
			m_start_address = (offset - 0x82) << 9;
			logerror("offset %04X, video mode 2 (TEXT_HIRES)\n", m_start_address);
		}
		// 32x32, 0x1000 page length
		else
		{
			m_video_mode = TEXT_LORES;
			m_start_address = (offset - 0x02) << 9;
			logerror("offset %04X, video mode 2 (TEXT_LORES)\n", m_start_address);
		}
		break;

	case 3:
		// 512x256, 64 bytes per scanline, 0x4000 page length, odd scanlines at 0x2000
		if (offset > 0x80)
		{
			m_video_mode = GRAPHICS_MONO_HIRES;
			m_start_address = ((offset & 0x60) << 9) + ((offset & 0x08) << 13);
			logerror("offset %04X, video mode 3 (MONO_HIRES)\n", m_start_address);
		}
		// 256x256, 32 bytes per scanline, 0x2000 page length, linear
		else
		{
			m_video_mode = GRAPHICS_MONO_LORES;
			m_start_address = ((offset & 0x70) << 9) + ((offset & 0x08) << 13);
			logerror("offset %04X, video mode 3 (MONO_LORES)\n", m_start_address);
		}
		break;
	}
}


void agat9video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
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

static constexpr int color_1_p[4] =
{
	0, 4, 0, 5
};

static constexpr int color_2_p[4][2] =
{
	{ 0, 7 },
	{ 7, 0 },
	{ 0, 2 },
	{ 2, 0 },
};

void agat9video_device::text_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch, attr;
	int fg = 0;
	int bg = color_1_p[palette_index];

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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

void agat9video_device::text_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch;
	int fg, bg;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	if (m_start_address & 0x800)
	{
		fg = color_2_p[palette_index][1];
		bg = color_2_p[palette_index][0];
	}
	else
	{
		fg = color_2_p[palette_index][0];
		bg = color_2_p[palette_index][1];
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

void agat9video_device::graph_update_mono_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int fg = 7, bg = 0;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 32; col++)
		{
			uint32_t const address = m_start_address + col + (row * 0x20);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 8; b++)
			{
				uint8_t v = (gfx & 0x80) >> 7;
				v = color_2_p[palette_index][v];
				gfx <<= 1;
				*(p++) = v ? fg : bg;
				*(p++) = v ? fg : bg;
			}
		}
	}
}

void agat9video_device::graph_update_mono_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int fg = 7, bg = 0;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 64; col++)
		{
			uint32_t const address = m_start_address + col + ((row / 2) * 0x40) + 0x2000 * (row & 1);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 8; b++)
			{
				uint8_t v = (gfx & 0x80) >> 7;
				v = color_2_p[palette_index][v];
				gfx <<= 1;
				*(p++) = v ? fg : bg;
			}
		}
	}
}

static constexpr int color_4_p[4][4] =
{
	{ 0, 1, 2, 4 },
	{ 7, 1, 2, 4 },
	{ 0, 0, 2, 4 },
	{ 0, 1, 0, 4 }
};

void agat9video_device::graph_update_color_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 0x40; col++)
		{
			uint32_t const address = m_start_address + col + ((row / 2) * 0x40) + 0x2000 * (row & 1);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 4; b++)
			{
				uint8_t v = (gfx & 0xc0) >> 6;
				v = color_4_p[palette_index][v];
				gfx <<= 2;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}

void agat9video_device::graph_update_color_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (int row = beginrow; row <= endrow; row++)
	{
		uint16_t *p = &bitmap.pix(row);
		for (int col = 0; col < 0x40; col++)
		{
			uint32_t const address = m_start_address + col + ((row / 2) * 0x40);
			uint8_t gfx = m_ram_dev->read(address);

			for (int b = 0; b < 2; b++)
			{
				uint8_t v = (gfx & 0xf0) >> 4;
				gfx <<= 4;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}

// Apple ][ video modes

void agat9video_device::plot_text_character_apple(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	if ((code >= 0x40) && (code <= 0x7f))
	{
		code &= 0x3f;

		if (m_flash)
		{
			using std::swap;
			swap(fg, bg);
		}
	}

	/* look up the character data */
	uint8_t const *const chardata = &textgfx_data[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << (6 - x))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void agat9video_device::text_update_apple(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = m_page2 ? 0x800 : 0x400;

	int const fg = color_2_p[palette_index][1];
	int const bg = color_2_p[palette_index][0];

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (int row = beginrow; row <= endrow; row += 8)
	{
		for (int col = 0; col < 40; col++)
		{
			/* calculate address */
			uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_apple(bitmap, col * 7, row, 1, m_ram_dev->read(address),
					m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void agat9video_device::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint8_t vram_row[42];
	int begincol = 0, endcol = 40;

	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
	if (endrow < beginrow)
		return;

	// we generate 2 pixels per "column" so adjust
	if (begincol < (cliprect.min_x/7))
		begincol = (cliprect.min_x/7);
	if (endcol > (cliprect.max_x/7))
		endcol = (cliprect.max_x/7);
	if (cliprect.max_x > 39*7)
		endcol = 40;
	if (endcol < begincol)
		return;

	int const va = m_page2 ? 0x4000 : 0x2000;

	vram_row[0] = 0;
	vram_row[41] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = begincol; col < endcol; col++)
		{
			int const offset = ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1 + col] = m_ram_dev->read(va + offset);
		}

		uint16_t *p = &bitmap.pix(row);

		/*
		 * p. 50 of technical manual:
		 *
		 * 1. a 'zero' bit is always drawn as 'black' pixel
		 * 2. two consecutive 'one' bits are always drawn as 'white' pixel, even bits from different bytes
		 * 3. even pixels can be 'black', 'purple' (5) or 'blue' (4)
		 * 4. odd pixels can be 'black', 'green' (2) or 'red' (1)
		 * 5. bit 7 affects color of pixels in this byte.  zero = 'blue' or 'red', one = 'purple' or 'green'.
		 */
		for (int col = 0; col < 40; col++)
		{
			uint32_t const w =  (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
							|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
							|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			uint16_t const *const artifact_map_ptr = &m_hires_artifact_map[((vram_row[col + 1] & 0x80) >> 7) * 16];

			for (int b = 0; b < 7; b++)
			{
				uint16_t const v = artifact_map_ptr[((w >> (b + 7 - 1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
				*(p++) = v;
			}
		}
	}
}


uint32_t agat9video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_video_mode)
	{
	case APPLE: // from apple2e.cpp
		// always update the flash timer here so it's smooth regardless of mode switches
		m_flash = ((machine().time() * 4).seconds() & 1) ? true : false;

		if (m_graphics)
		{
			if (m_mix)
			{
				hgr_update(screen, bitmap, cliprect, 0, 159);
				text_update_apple(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				hgr_update(screen, bitmap, cliprect, 0, 191);
			}
		}
		else
		{
			text_update_apple(screen, bitmap, cliprect, 0, 191);
		}
		break;

	case TEXT_LORES:
		text_update_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case TEXT_HIRES:
		text_update_hires(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_MONO_LORES:
		graph_update_mono_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_MONO_HIRES:
		graph_update_mono_hires(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_COLOR_LORES:
		graph_update_color_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_COLOR_HIRES:
		graph_update_color_hires(screen, bitmap, cliprect, 0, 255);
		break;

	default:
		graph_update_mono_lores(screen, bitmap, cliprect, 0, 255);
		break;
	}

	return 0;
}
