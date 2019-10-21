// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  video/apple2.cpp

***************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "video/apple2.h"
#include "screen.h"

/***************************************************************************/


#define BLACK   0
#define DKRED   1
#define DKBLUE  2
#define PURPLE  3
#define DKGREEN 4
#define DKGRAY  5
#define BLUE    6
#define LTBLUE  7
#define BROWN   8
#define ORANGE  9
#define GRAY    10
#define PINK    11
#define GREEN   12
#define YELLOW  13
#define AQUA    14
#define WHITE   15

DEFINE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device, "a2video", "Apple II video")

//-------------------------------------------------
//  a2_video_device - constructor
//-------------------------------------------------

a2_video_device::a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APPLE2_VIDEO, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
{
}

void a2_video_device::device_start()
{
	static const uint8_t hires_artifact_color_table[] =
	{
		BLACK,  PURPLE, GREEN,  WHITE,
		BLACK,  BLUE,   ORANGE, WHITE
	};
	static const uint8_t dhires_artifact_color_table[] =
	{
		BLACK,      DKGREEN,    BROWN,  GREEN,
		DKRED,      DKGRAY,     ORANGE, YELLOW,
		DKBLUE,     BLUE,       GRAY,   AQUA,
		PURPLE,     LTBLUE,     PINK,   WHITE
	};

	// generate hi-res artifact data
	int i, j;
	uint16_t c;

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
				if ((i & 0x05) == 0x05)
					c = j ? 1 : 2;
				else
					c = 0;
			}
			m_hires_artifact_map[ 0 + j*8 + i] = hires_artifact_color_table[(c + 0) % 8];
			m_hires_artifact_map[16 + j*8 + i] = hires_artifact_color_table[(c + 4) % 8];
		}
	}

	/* 2^4 dependent pixels */
	m_dhires_artifact_map = std::make_unique<uint16_t[]>(16);

	/* build double hires artifact map */
	for (i = 0; i < 16; i++)
	{
		m_dhires_artifact_map[i] = dhires_artifact_color_table[i];
	}

	// initialise for device_palette_interface
	init_palette();

	save_item(NAME(m_page2));
	save_item(NAME(m_flash));
	save_item(NAME(m_mix));
	save_item(NAME(m_graphics));
	save_item(NAME(m_hires));
	save_item(NAME(m_dhires));
	save_item(NAME(m_80col));
	save_item(NAME(m_altcharset));
	save_item(NAME(m_an2));
	save_item(NAME(m_80store));
	save_item(NAME(m_monohgr));
	save_item(NAME(m_GSfg));
	save_item(NAME(m_GSbg));
	save_item(NAME(m_GSborder));
	save_item(NAME(m_newvideo));
	save_item(NAME(m_monochrome));
	save_item(NAME(m_shr_palette));
}

void a2_video_device::device_reset()
{
	m_page2 = false;
	m_graphics = false;
	m_hires = false;
	m_80col = false;
	m_altcharset = false;
	m_dhires = false;
	m_flash = false;
	m_mix = false;
	m_sysconfig = 0;
	m_an2 = false;
	m_80store = false;
	m_monohgr = false;
	m_newvideo = 0x01;
}

WRITE_LINE_MEMBER(a2_video_device::txt_w)
{
	if (m_graphics == state) // avoid flickering from II+ refresh polling
	{
		// select graphics or text mode
		screen().update_now();
		m_graphics = !state;
	}
}

WRITE_LINE_MEMBER(a2_video_device::mix_w)
{
	// select mixed mode or nomix
	screen().update_now();
	m_mix = state;
}

WRITE_LINE_MEMBER(a2_video_device::scr_w)
{
	// select primary or secondary page
	if (!m_80col)
		screen().update_now();
	m_page2 = state;
}

WRITE_LINE_MEMBER(a2_video_device::res_w)
{
	// select lo-res or hi-res
	screen().update_now();
	m_hires = state;
}

WRITE_LINE_MEMBER(a2_video_device::dhires_w)
{
	// select double hi-res
	screen().update_now();
	m_dhires = !state;
}

WRITE_LINE_MEMBER(a2_video_device::an2_w)
{
	m_an2 = state;
}

void a2_video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	if (!m_altcharset)
	{
		if ((code >= 0x40) && (code <= 0x7f))
		{
			code &= 0x3f;

			if (m_flash)
			{
				i = fg;
				fg = bg;
				bg = i;
			}
		}
	}
	else
	{
		if ((code >= 0x60) && (code <= 0x7f))
		{
			code |= 0x80;   // map to lowercase normal
			i = fg;         // and flip the color
			fg = bg;
			bg = i;
		}
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << x)) ? bg : fg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_orig(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	if ((code >= 0x40) && (code <= 0x7f))
	{
		if (m_flash)
		{
			i = fg;
			fg = bg;
			bg = i;
		}
	}
	else if (code < 0x40)   // inverse: flip FG and BG
	{
			i = fg;
			fg = bg;
			bg = i;
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_jplus(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	if ((code >= 0x40) && (code <= 0x7f))
	{
		code &= 0x3f;
		if (m_flash)
		{
			i = fg;
			fg = bg;
			bg = i;
		}
	}
	else if (code < 0x40)   // inverse: flip FG and BG
	{
			i = fg;
			fg = bg;
			bg = i;
	}

	if (m_an2)
	{
		code |= 0x80;
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	if ((code >= 0x40) && (code <= 0x7f))
	{
		if (m_flash)
		{
			i = fg;
			fg = bg;
			bg = i;
		}
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 1; x < 8; x++)
		{
			color = (chardata[y] & (1 << x)) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + ((x-1) * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_characterGS(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	if (!m_altcharset)
	{
		if ((code >= 0x40) && (code <= 0x7f))
		{
			code &= 0x3f;

			if (m_flash)
			{
				i = fg;
				fg = bg;
				bg = i;
			}
		}
	}
	else
	{
		code |= 0x100;
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << x)) ? bg : fg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, y, x;
	uint8_t code;
	uint32_t start_address = m_page2 ? 0x0800 : 0x0400;
	uint32_t address;
	int fg = 0;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());

	if (!(m_sysconfig & 0x03))
	{
		for (row = beginrow; row <= endrow; row += 8)
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate adderss */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				code = m_ram_ptr[address];

				/* and now draw */
				for (y = 0; y < 4; y++)
				{
					if ((row + y) <= endrow)
					{
						for (x = 0; x < 14; x++)
						{
							bitmap.pix16(row + y, col * 14 + x) = (code >> 0) & 0x0F;
						}
					}
				}
				for (y = 4; y < 8; y++)
				{
					if ((row + y) <= endrow)
					{
						for (x = 0; x < 14; x++)
						{
							bitmap.pix16(row + y, col * 14 + x) = (code >> 4) & 0x0F;
						}
					}
				}
			}
		}
	}
	else
	{
		for (row = beginrow; row <= endrow; row += 8)
		{
			for (col = 0; col < 40; col++)
			{
				uint8_t bits;

				/* calculate adderss */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				code = m_ram_ptr[address];

				bits = (code >> 0) & 0x0F;
				/* and now draw */
				for (y = 0; y < 4; y++)
				{
					if ((row + y) <= endrow)
					{
						for (x = 0; x < 14; x++)
						{
							bitmap.pix16(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
						}
					}
				}

				bits = (code >> 4) & 0x0F;
				for (y = 4; y < 8; y++)
				{
					if ((row + y) <= endrow)
					{
						for (x = 0; x < 14; x++)
						{
							bitmap.pix16(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
						}
					}
				}
			}
		}
	}
}

void a2_video_device::dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, y;
	uint8_t code, auxcode;
	uint32_t start_address = m_page2 ? 0x0800 : 0x0400;
	uint32_t address;
	static const int aux_colors[16] = { 0, 2, 4, 6, 8, 0xa, 0xc, 0xe, 1, 3, 5, 7, 9, 0xb, 0xd, 0xf };
	int fg = 0;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	if (!(m_sysconfig & 0x03))
	{
		for (row = beginrow; row <= endrow; row += 8)
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate adderss */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				code = m_ram_ptr[address];
				auxcode = m_aux_ptr[address];

				/* and now draw */
				for (y = 0; y < 4; y++)
				{
					uint16_t *vram = &bitmap.pix16(row + y, (col * 14));

					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
					*vram++ = (code >> 0) & 0x0F;
				}
				for (y = 4; y < 8; y++)
				{
					uint16_t *vram = &bitmap.pix16(row + y, (col * 14));

					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
					*vram++ = (code >> 4) & 0x0F;
				}
			}
		}
	}
	else
	{
		for (row = beginrow; row <= endrow; row += 8)
		{
			for (col = 0; col < 40; col++)
			{
				uint8_t bits, abits;

				/* calculate adderss */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				code = m_ram_ptr[address];
				auxcode = m_aux_ptr[address];

				bits = (code >> 0) & 0x0F;
				abits = (auxcode >> 0) & 0x0F;

				/* and now draw */
				for (y = 0; y < 4; y++)
				{
					uint16_t *vram = &bitmap.pix16(row + y, (col * 14));

					*vram++ = abits & (1 << 0) ? fg : 0;
					*vram++ = abits & (1 << 1) ? fg : 0;
					*vram++ = abits & (1 << 2) ? fg : 0;
					*vram++ = abits & (1 << 3) ? fg : 0;
					*vram++ = abits & (1 << 0) ? fg : 0;
					*vram++ = abits & (1 << 1) ? fg : 0;
					*vram++ = abits & (1 << 2) ? fg : 0;
					*vram++ = bits & (1 << 0) ? fg : 0;
					*vram++ = bits & (1 << 1) ? fg : 0;
					*vram++ = bits & (1 << 2) ? fg : 0;
					*vram++ = bits & (1 << 3) ? fg : 0;
					*vram++ = bits & (1 << 0) ? fg : 0;
					*vram++ = bits & (1 << 1) ? fg : 0;
					*vram++ = bits & (1 << 2) ? fg : 0;
				}

				bits = (code >> 4) & 0x0F;
				abits = (auxcode >> 4) & 0x0F;

				for (y = 4; y < 8; y++)
				{
					uint16_t *vram = &bitmap.pix16(row + y, (col * 14));

					*vram++ = abits & (1 << 0) ? fg : 0;
					*vram++ = abits & (1 << 1) ? fg : 0;
					*vram++ = abits & (1 << 2) ? fg : 0;
					*vram++ = abits & (1 << 3) ? fg : 0;
					*vram++ = abits & (1 << 0) ? fg : 0;
					*vram++ = abits & (1 << 1) ? fg : 0;
					*vram++ = abits & (1 << 2) ? fg : 0;
					*vram++ = bits & (1 << 0) ? fg : 0;
					*vram++ = bits & (1 << 1) ? fg : 0;
					*vram++ = bits & (1 << 2) ? fg : 0;
					*vram++ = bits & (1 << 3) ? fg : 0;
					*vram++ = bits & (1 << 0) ? fg : 0;
					*vram++ = bits & (1 << 1) ? fg : 0;
					*vram++ = bits & (1 << 2) ? fg : 0;
				}
			}
		}
	}
}

void a2_video_device::text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address;
	uint32_t address;
	uint8_t *aux_page = m_ram_ptr;
	int fg = 0;
	int bg = 0;

	if (m_80col)
	{
		start_address = 0x400;
		if (m_aux_ptr)
		{
			aux_page = m_aux_ptr;
		}
	}
	else
	{
		start_address = m_page2 ? 0x800 : 0x400;
	}

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		if (m_80col)
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				plot_text_character(bitmap, col * 14, row, 1, aux_page[address],
					m_char_ptr, m_char_size, fg, bg);
				plot_text_character(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address],
					m_char_ptr, m_char_size, fg, bg);
			}
		}
		else
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
				plot_text_character(bitmap, col * 14, row, 2, m_ram_ptr[address],
					m_char_ptr, m_char_size, fg, bg);
			}
		}
	}
}

void a2_video_device::text_update_orig(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address = m_page2 ? 0x800 : 0x400;
	uint32_t address;
	int fg = 0;
	int bg = 0;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_orig(bitmap, col * 14, row, 2, m_ram_ptr[address],
				m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void a2_video_device::text_update_jplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address = m_page2 ? 0x800 : 0x400;
	uint32_t address;
	int fg = 0;
	int bg = 0;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_jplus(bitmap, col * 14, row, 2, m_ram_ptr[address],
				m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void a2_video_device::text_update_ultr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address = m_page2 ? 0x800 : 0x400;
	uint32_t address;
	int fg = 0;
	int bg = 0;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_ultr(bitmap, col * 14, row, 2, m_ram_ptr[address],
				m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void a2_video_device::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	const uint8_t *vram;
	int row, col, b;
	int offset;
	uint8_t vram_row[42];
	uint16_t v;
	uint16_t *p;
	uint32_t w;
	uint16_t *artifact_map_ptr;
	int mon_type = m_sysconfig & 0x03;
	int begincol = 0, endcol = 40;

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	// we generate 2 pixels per "column" so adjust
	if (begincol < (cliprect.left()/14))
		begincol = (cliprect.left()/14);
	if (endcol > (cliprect.right()/14))
		endcol = (cliprect.right()/14);
	if (cliprect.right() > 39*14)
		endcol = 40;
	if (endcol < begincol)
		return;

	//printf("HGR draw: page %c, rows %d-%d cols %d-%d\n", m_page2 ? '2' : '1', beginrow, endrow, begincol, endcol);

	vram = &m_ram_ptr[(m_page2 ? 0x4000 : 0x2000)];

	vram_row[0] = 0;
	vram_row[41] = 0;

	for (row = beginrow; row <= endrow; row++)
	{
		for (col = begincol; col < endcol; col++)
		{
			offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+col] = vram[offset];
		}

		p = &bitmap.pix16(row);

		for (col = 0; col < 40; col++)
		{
			w =     (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
				|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
				|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);


			// verified on h/w: setting dhires w/o 80col emulates a rev. 0 Apple ][ with no orange/blue
			if (m_dhires)
			{
				artifact_map_ptr = m_hires_artifact_map.get();
			}
			else
			{
				artifact_map_ptr = &m_hires_artifact_map[((vram_row[col + 1] & 0x80) >> 7) * 16];
			}

			// CEC mono HGR mode
			if ((m_monohgr) && (mon_type == 0))
			{
				mon_type = 1;
			}

			switch (mon_type)
			{
				case 0:
					for (b = 0; b < 7; b++)
					{
						v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
						*(p++) = v;
						*(p++) = v;
					}
					break;

				case 1:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? WHITE : BLACK;
						*(p++) = v ? WHITE : BLACK;
					}
					break;

				case 2:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? GREEN : BLACK;
						*(p++) = v ? GREEN : BLACK;
					}
					break;

				case 3:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? ORANGE : BLACK;
						*(p++) = v ? ORANGE : BLACK;
					}
					break;
			}
		}
	}
}

// similar to regular A2 except page 2 is at $A000
void a2_video_device::hgr_update_tk2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	const uint8_t *vram;
	int row, col, b;
	int offset;
	uint8_t vram_row[42];
	uint16_t v;
	uint16_t *p;
	uint32_t w;
	uint16_t *artifact_map_ptr;
	int mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	vram = &m_ram_ptr[(m_page2 ? 0xa000 : 0x2000)];

	vram_row[0] = 0;
	vram_row[41] = 0;

	for (row = beginrow; row <= endrow; row++)
	{
		for (col = 0; col < 40; col++)
		{
			offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+col] = vram[offset];
		}

		p = &bitmap.pix16(row);

		for (col = 0; col < 40; col++)
		{
			w =     (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
				|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
				|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			switch (mon_type)
			{
				case 0:
					artifact_map_ptr = &m_hires_artifact_map[((vram_row[col+1] & 0x80) >> 7) * 16];
					for (b = 0; b < 7; b++)
					{
						v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
						*(p++) = v;
						*(p++) = v;
					}
					break;

				case 1:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? WHITE : BLACK;
						*(p++) = v ? WHITE : BLACK;
					}
					break;

				case 2:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? GREEN : BLACK;
						*(p++) = v ? GREEN : BLACK;
					}
					break;

				case 3:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? ORANGE : BLACK;
						*(p++) = v ? ORANGE : BLACK;
					}
					break;
			}
		}
	}
}

void a2_video_device::dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	const uint8_t *vram, *vaux;
	int row, col, b;
	int offset;
	uint8_t vram_row[82];
	uint16_t v;
	uint16_t *p;
	uint32_t w;
	int page = m_page2 ? 0x4000 : 0x2000;
	int mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	vram = &m_ram_ptr[page];
	if (m_aux_ptr)
	{
		vaux = m_aux_ptr;
	}
	else
	{
		vaux = vram;
	}
	vaux += page;

	vram_row[0] = 0;
	vram_row[81] = 0;

	for (row = beginrow; row <= endrow; row++)
	{
		for (col = 0; col < 40; col++)
		{
			offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+(col*2)+0] = vaux[offset];
			vram_row[1+(col*2)+1] = vram[offset];
		}

		p = &bitmap.pix16(row);

		for (col = 0; col < 80; col++)
		{
			w =     (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
				|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
				|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			switch (mon_type)
			{
				case 0:
					for (b = 0; b < 7; b++)
					{
						v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
						*(p++) = v;
					}
					break;

				case 1:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? WHITE : BLACK;
					}
					break;

				case 2:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? GREEN : BLACK;
					}
					break;

				case 3:
					w >>= 7;
					for (b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? ORANGE : BLACK;
					}
					break;
			}
		}
	}
}

/* according to Steve Nickolas (author of Dapple), our original palette would
 * have been more appropriate for an Apple IIgs.  So we've substituted in the
 * Robert Munafo palette instead, which is more accurate on 8-bit Apples
 */
static const rgb_t apple2_palette[] =
{
	rgb_t::black(),
	rgb_t(0xE3, 0x1E, 0x60), /* Dark Red */
	rgb_t(0x60, 0x4E, 0xBD), /* Dark Blue */
	rgb_t(0xFF, 0x44, 0xFD), /* Purple */
	rgb_t(0x00, 0xA3, 0x60), /* Dark Green */
	rgb_t(0x9C, 0x9C, 0x9C), /* Dark Gray */
	rgb_t(0x14, 0xCF, 0xFD), /* Medium Blue */
	rgb_t(0xD0, 0xC3, 0xFF), /* Light Blue */
	rgb_t(0x60, 0x72, 0x03), /* Brown */
	rgb_t(0xFF, 0x6A, 0x3C), /* Orange */
	rgb_t(0x9C, 0x9C, 0x9C), /* Light Grey */
	rgb_t(0xFF, 0xA0, 0xD0), /* Pink */
	rgb_t(0x14, 0xF5, 0x3C), /* Light Green */
	rgb_t(0xD0, 0xDD, 0x8D), /* Yellow */
	rgb_t(0x72, 0xFF, 0xD0), /* Aquamarine */
	rgb_t(0xFF, 0xFF, 0xFF)  /* White */
};

void a2_video_device::init_palette()
{
	for (int i = 0; i < ARRAY_LENGTH(apple2_palette); i++)
		set_pen_color(i, apple2_palette[i]);
}

uint32_t a2_video_device::palette_entries() const
{
	return ARRAY_LENGTH(apple2_palette);
}

uint32_t a2_video_device::screen_update_GS(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint8_t *vram;
	uint32_t *scanline;
	uint8_t scb, b;
	int col, palette;
	uint32_t last_pixel = 0, pixel;
	int beamy;
	uint16_t *a2pixel;

	beamy = cliprect.top();

	if (m_newvideo & 0x80)
	{
		// in top or bottom border?
		if ((beamy < BORDER_TOP) || (beamy >= 200+BORDER_TOP))
		{
			// don't draw past the bottom border
			if (beamy >= 231+BORDER_TOP)
			{
				return 0;
			}

			scanline = &bitmap.pix32(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else    // regular screen area
		{
			int shrline = beamy - BORDER_TOP;

			scb = m_aux_ptr[0x9D00 + shrline];
			palette = ((scb & 0x0f) << 4);

			vram = &m_aux_ptr[0x2000 + (shrline * 160)];
			scanline = &bitmap.pix32(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+640] = m_GSborder_colors[m_GSborder];
			}

			if (scb & 0x80) // 640 mode
			{
				for (col = 0; col < 160; col++)
				{
					b = vram[col];
					scanline[col * 4 + 0 + BORDER_LEFT] = m_shr_palette[palette +  0 + ((b >> 6) & 0x03)];
					scanline[col * 4 + 1 + BORDER_LEFT] = m_shr_palette[palette +  4 + ((b >> 4) & 0x03)];
					scanline[col * 4 + 2 + BORDER_LEFT] = m_shr_palette[palette +  8 + ((b >> 2) & 0x03)];
					scanline[col * 4 + 3 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];
				}
			}
			else        // 320 mode
			{
				for (col = 0; col < 160; col++)
				{
					b = vram[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 4 + 0 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 4 + 1 + BORDER_LEFT] = m_shr_palette[pixel];

					b = vram[col];
					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 4 + 2 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 4 + 3 + BORDER_LEFT] = m_shr_palette[pixel];
				}
			}
		}
	}
	else
	{
		/* call legacy Apple II video rendering at scanline 0 to draw into the off-screen buffer */
		if (beamy == 0)
		{
			rectangle new_cliprect(0, 559, 0, 191);
			screen_update_GS_8bit(screen, *m_8bit_graphics, new_cliprect);
		}

		if ((beamy < (BORDER_TOP+4)) || (beamy >= (192+4+BORDER_TOP)))
		{
			if (beamy >= (231+BORDER_TOP))
			{
				return 0;
			}

			scanline = &bitmap.pix32(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else
		{
			scanline = &bitmap.pix32(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT + 40; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+600] = m_GSborder_colors[m_GSborder];
			}

			a2pixel = &m_8bit_graphics->pix16(beamy-(BORDER_TOP+4));
			for (int x = 0; x < 560; x++)
			{
				scanline[40 + BORDER_LEFT + x] = m_GSborder_colors[*a2pixel++];
			}
		}
	}
	return 0;
}

uint32_t a2_video_device::screen_update_GS_8bit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool old_page2 = m_page2;

	// don't display page2 if 80store is set (we just saved the previous value, don't worry)
	if (m_80store)
	{
		m_page2 = false;
	}

	// always update the flash timer here so it's smooth regardless of mode switches
	m_flash = ((machine().time() * 4).seconds() & 1) ? true : false;

	if (m_graphics)
	{
		if (m_hires)
		{
			if (m_mix)
			{
				if ((m_dhires) && (m_80col))
				{
					dhgr_update(screen, bitmap, cliprect, 0, 159);
				}
				else
				{
					hgr_update(screen, bitmap, cliprect, 0, 159);
				}
				text_updateGS(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				if ((m_dhires) && (m_80col))
				{
					dhgr_update(screen, bitmap, cliprect, 0, 191);
				}
				else
				{
					hgr_update(screen, bitmap, cliprect, 0, 191);
				}
			}
		}
		else    // lo-res
		{
			if (m_mix)
			{
				if ((m_dhires) && (m_80col))
				{
					dlores_update(screen, bitmap, cliprect, 0, 159);
				}
				else
				{
					lores_update(screen, bitmap, cliprect, 0, 159);
				}

				text_updateGS(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				if ((m_dhires) && (m_80col))
				{
					dlores_update(screen, bitmap, cliprect, 0, 191);
				}
				else
				{
					lores_update(screen, bitmap, cliprect, 0, 191);
				}
			}
		}
	}
	else
	{
		text_updateGS(screen, bitmap, cliprect, 0, 191);
	}

	m_page2 = old_page2;

	return 0;
}

void a2_video_device::text_updateGS(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address;
	uint32_t address;
	uint8_t *aux_page = m_ram_ptr;

	if (m_80col)
	{
		start_address = 0x400;
		if (m_aux_ptr)
		{
			aux_page = m_aux_ptr;
		}
	}
	else
	{
		start_address = m_page2 ? 0x800 : 0x400;
	}

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	for (row = beginrow; row <= endrow; row += 8)
	{
		if (m_80col)
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				plot_text_characterGS(bitmap, col * 14, row, 1, aux_page[address],
					m_char_ptr, m_char_size, m_GSfg, m_GSbg);
				plot_text_characterGS(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address],
					m_char_ptr, m_char_size, m_GSfg, m_GSbg);
			}
		}
		else
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
				plot_text_characterGS(bitmap, col * 14, row, 2, m_ram_ptr[address],
					m_char_ptr, m_char_size, m_GSfg, m_GSbg);
			}
		}
	}
}

