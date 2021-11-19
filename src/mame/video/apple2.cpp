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
	save_item(NAME(m_rgbmode));
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
	m_rgbmode = 3;  // default to color DHGR
	m_monochrome = 0; // TODO: never set, but if left uninitialized could cause the emulation to start in monochrome by accident. Default to color for now
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

	// RGB cards shift in a mode bit on the rising edge
	if ((m_dhires) && (state))
	{
		m_rgbmode = (m_rgbmode << 1) & 3;
		m_rgbmode |= m_80col ? 1 : 0;
	}

	m_dhires = !state;
}

WRITE_LINE_MEMBER(a2_video_device::an2_w)
{
	m_an2 = state;
}

void a2_video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if (!m_altcharset)
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
	}
	else
	{
		if ((code >= 0x60) && (code <= 0x7f))
		{
			code |= 0x80;   // map to lowercase normal
			using std::swap;// and flip the color
			std::swap(fg, bg);
		}
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << x)) ? bg : fg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_dodo(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if (!m_altcharset)
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
	}
	else
	{
		if ((code >= 0x60) && (code <= 0x7f))
		{
			code |= 0x80;   // map to lowercase normal
			using std::swap;// and flip the color
			swap(fg, bg);
		}
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << (x+1))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_orig(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if ((code >= 0x40) && (code <= 0x7f))
	{
		if (m_flash)
		{
			using std::swap;
			swap(fg, bg);
		}
	}
	else if (code < 0x40)   // inverse: flip FG and BG
	{
		using std::swap;
		swap(fg, bg);
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_jplus(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
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
	else if (code < 0x40)   // inverse: flip FG and BG
	{
		using std::swap;
		swap(fg, bg);
	}

	if (m_an2)
	{
		code |= 0x80;
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if ((code >= 0x40) && (code <= 0x7f))
	{
		if (m_flash)
		{
			using std::swap;
			swap(fg, bg);
		}
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 1; x < 8; x++)
		{
			uint16_t const color = (chardata[y] & (1 << x)) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + ((x-1) * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::plot_text_characterGS(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if (!m_altcharset)
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
	}
	else
	{
		code |= 0x100;
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t const color = (chardata[y] & (1 << x)) ? bg : fg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = m_80store ? 0x400 : m_page2 ? 0x0800 : 0x0400;
	int fg = 0;

	switch (m_sysconfig & 0x03)
	{
		case 0: case 4: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());
	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	//printf("GR: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	if (!(m_sysconfig & 0x03))  // color
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];

				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							bitmap.pix(row + y, col * 14 + x) = (code >> 0) & 0x0F;
						}
					}
				}
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							bitmap.pix(row + y, col * 14 + x) = (code >> 4) & 0x0F;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				uint8_t bits;

				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];

				bits = (code >> 0) & 0x0F;
				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							if (col & 1)
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << ((x+2) % 4)) ? fg : 0;
							}
							else
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
							}
						}
					}
				}

				bits = (code >> 4) & 0x0F;
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							if (col & 1)
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << ((x+2) % 4)) ? fg : 0;
							}
							else
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
							}
						}
					}
				}
			}
		}
	}
}

void a2_video_device::dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = m_page2 ? 0x0800 : 0x0400;
	static const int aux_colors[16] = { 0, 2, 4, 6, 8, 0xa, 0xc, 0xe, 1, 3, 5, 7, 9, 0xb, 0xd, 0xf };
	int fg = 0;

	switch (m_sysconfig & 0x03)
	{
		case 0: case 4: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	if (!(m_sysconfig & 0x03))
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];
				uint8_t const auxcode = m_aux_ptr[address];

				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

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
				}
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

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
	}
	else
	{
		for (int row = startrow; row < stoprow; row += 8)
		{
			for (int col = 0; col < 40; col++)
			{
				uint8_t bits, abits;

				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];
				uint8_t const auxcode = m_aux_ptr[address];

				bits = (code >> 0) & 0x0F;
				abits = (auxcode >> 0) & 0x0F;

				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

						if (col & 1)
						{
							*vram++ = abits & (1 << 2) ? fg : 0;
							*vram++ = abits & (1 << 3) ? fg : 0;
							*vram++ = abits & (1 << 0) ? fg : 0;
							*vram++ = abits & (1 << 1) ? fg : 0;
							*vram++ = abits & (1 << 2) ? fg : 0;
							*vram++ = abits & (1 << 3) ? fg : 0;
							*vram++ = abits & (1 << 0) ? fg : 0;
							*vram++ = bits & (1 << 1) ? fg : 0;
							*vram++ = bits & (1 << 2) ? fg : 0;
							*vram++ = bits & (1 << 3) ? fg : 0;
							*vram++ = bits & (1 << 0) ? fg : 0;
							*vram++ = bits & (1 << 1) ? fg : 0;
							*vram++ = bits & (1 << 2) ? fg : 0;
							*vram++ = bits & (1 << 3) ? fg : 0;
						}
						else
						{
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

				bits = (code >> 4) & 0x0F;
				abits = (auxcode >> 4) & 0x0F;

				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

						if (col & 1)
						{
							*vram++ = abits & (1 << 2) ? fg : 0;
							*vram++ = abits & (1 << 3) ? fg : 0;
							*vram++ = abits & (1 << 0) ? fg : 0;
							*vram++ = abits & (1 << 1) ? fg : 0;
							*vram++ = abits & (1 << 2) ? fg : 0;
							*vram++ = abits & (1 << 3) ? fg : 0;
							*vram++ = abits & (1 << 0) ? fg : 0;
							*vram++ = bits & (1 << 1) ? fg : 0;
							*vram++ = bits & (1 << 2) ? fg : 0;
							*vram++ = bits & (1 << 3) ? fg : 0;
							*vram++ = bits & (1 << 0) ? fg : 0;
							*vram++ = bits & (1 << 1) ? fg : 0;
							*vram++ = bits & (1 << 2) ? fg : 0;
							*vram++ = bits & (1 << 3) ? fg : 0;
						}
						else
						{
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
	}
}

void a2_video_device::text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint8_t const *const aux_page = m_aux_ptr ? m_aux_ptr : m_ram_ptr;

	uint32_t const start_address = m_page2 ? 0x800 : 0x400;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	//printf("TXT: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	int fg = 0;
	int bg = 0;
	switch (m_sysconfig & 0x03)
	{
		case 0: case 4: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (int row = startrow; row < stoprow; row += 8)
	{
		if (m_80col)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				plot_text_character(bitmap, col * 14, row, 1, aux_page[address],
					fg, bg);
				plot_text_character(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address],
					fg, bg);
			}
		}
		else
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
				if (((m_sysconfig & 7) == 4) && (m_dhires))
				{
					u8 tmp = aux_page[address];
					fg = tmp>>4;
					bg = tmp & 0xf;
				}

				plot_text_character(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
			}
		}
	}
}

void a2_video_device::text_update_inverse(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint8_t const *const aux_page = m_aux_ptr ? m_aux_ptr : m_ram_ptr;

	uint32_t const start_address = m_page2 ? 0x800 : 0x400;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	//printf("TXT: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	int fg = 0;
	int bg = 0;
	switch (m_sysconfig & 0x03)
	{
		case 0: case 4: bg = WHITE; break;
		case 1: bg = WHITE; break;
		case 2: bg = GREEN; break;
		case 3: bg = ORANGE; break;
	}

	for (int row = startrow; row < stoprow; row += 8)
	{
		if (m_80col)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				plot_text_character(bitmap, col * 14, row, 1, aux_page[address],
					fg, bg);
				plot_text_character(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address],
					fg, bg);
			}
		}
		else
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
				if (((m_sysconfig & 7) == 4) && (m_dhires))
				{
					u8 tmp = aux_page[address];
					fg = tmp>>4;
					bg = tmp & 0xf;
				}

				plot_text_character(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
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

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	switch (m_sysconfig & 0x03)
	{
		case 0: case 4: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = startrow; row <= stoprow; row += 8)
	{
		for (col = startcol; col < stopcol; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_orig(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
		}
	}
}

void a2_video_device::text_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address = m_page2 ? 0x800 : 0x400;
	uint32_t address;
	int fg = 0;
	int bg = 0;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = startrow; row <= stoprow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_orig(bitmap, col * 14, row, 2, m_ram_ptr[address], bg, fg);
		}
	}
}

void a2_video_device::text_update_dodo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address = m_page2 ? 0x800 : 0x400;
	uint32_t address;
	int fg = 0;
	int bg = 0;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = startrow; row <= stoprow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_dodo(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
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

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = startrow; row <= stoprow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_jplus(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
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

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	for (row = startrow; row <= stoprow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate address */
			address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
			plot_text_character_ultr(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
		}
	}
}

void a2_video_device::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = &m_ram_ptr[(m_page2 ? 0x4000 : 0x2000)];
	uint8_t vram_row[42];
	vram_row[0] = 0;
	vram_row[41] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int const offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+col] = vram[offset];
		}

		uint16_t *p = &bitmap.pix(row);

		for (int col = 0; col < 40; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			// verified on h/w: setting dhires w/o 80col emulates a rev. 0 Apple ][ with no orange/blue
			uint16_t const *artifact_map_ptr;
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

			// IIgs $C021 monochrome HGR
			if (m_monochrome & 0x80)
			{
				mon_type = 1;
			}

			switch (mon_type)
			{
				case 0:
					for (int b = 0; b < 7; b++)
					{
						if ((((col*14) + b) >= cliprect.left()) && (((col*14) + b) <= cliprect.right()))
						{
							uint16_t const v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
							*(p++) = v;
							*(p++) = v;
						}
						else
						{
							p++;
							p++;
						}
					}
					break;

				case 1:
					w >>= 7;
					if (vram_row[col+1] & 0x80)
					{
						p++;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						if ((((col*14) + b) >= cliprect.left()) && (((col*14) + b) <= cliprect.right()))
						{
							*(p++) = v ? WHITE : BLACK;
							*(p++) = v ? WHITE : BLACK;
						}
						else
						{
							p++;
							p++;
						}
					}
					if (vram_row[col+1] & 0x80)
					{
						p--;
					}
					break;

				case 2:
					w >>= 7;
					if (vram_row[col+1] & 0x80)
					{
						p++;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						if ((((col*14) + b) >= cliprect.left()) && (((col*14) + b) <= cliprect.right()))
						{
							*(p++) = v ? GREEN : BLACK;
							*(p++) = v ? GREEN : BLACK;
						}
						else
						{
							p++;
							p++;
						}
					}
					if (vram_row[col+1] & 0x80)
					{
						p--;
					}
					break;

				case 3:
					w >>= 7;
					if (vram_row[col+1] & 0x80)
					{
						p++;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						if ((((col*14) + b) >= cliprect.left()) && (((col*14) + b) <= cliprect.right()))
						{
							*(p++) = v ? ORANGE : BLACK;
							*(p++) = v ? ORANGE : BLACK;
						}
						else
						{
							p++;
							p++;
						}
					}
					if (vram_row[col+1] & 0x80)
					{
						p--;
					}
					break;

			}
		}
	}
}

// similar to regular A2 except page 2 is at $A000
void a2_video_device::hgr_update_tk2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int const mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = &m_ram_ptr[(m_page2 ? 0xa000 : 0x2000)];

	uint8_t vram_row[42];
	vram_row[0] = 0;
	vram_row[41] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int const offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+col] = vram[offset];
		}

		uint16_t *p = &bitmap.pix(row);

		for (int col = 0; col < 40; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			uint16_t const *artifact_map_ptr;
			switch (mon_type)
			{
				case 0:
					artifact_map_ptr = &m_hires_artifact_map[((vram_row[col+1] & 0x80) >> 7) * 16];
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
						*(p++) = v;
						*(p++) = v;
					}
					break;

			   case 1:
					w >>= 7;
					if (vram_row[col] & 0x80)
					{
						p--;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						*(p++) = v ? WHITE : BLACK;
						*(p++) = v ? WHITE : BLACK;
					}
					if (vram_row[col] & 0x80)
					{
						p++;
					}
					break;

				case 2:
					w >>= 7;
					if (vram_row[col] & 0x80)
					{
						p--;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						*(p++) = v ? GREEN : BLACK;
						*(p++) = v ? GREEN : BLACK;
					}
					if (vram_row[col] & 0x80)
					{
						p++;
					}
					break;

				case 3:
					w >>= 7;
					if (vram_row[col] & 0x80)
					{
						p--;
					}
					for (int b = 0; b < 7; b++)
					{
						uint16_t const v = (w & 1);
						w >>= 1;
						*(p++) = v ? ORANGE : BLACK;
						*(p++) = v ? ORANGE : BLACK;
					}
					if (vram_row[col] & 0x80)
					{
						p++;
					}
					break;
			}
		}
	}
}

void a2_video_device::dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint16_t v;
	int const page = m_page2 ? 0x4000 : 0x2000;
	int mon_type = m_sysconfig & 0x03;
	bool const bIsRGB = ((m_sysconfig & 7) == 4);
	bool const bIsRGBMixed = ((bIsRGB) && (m_rgbmode == 1));

	// IIgs force-monochrome-DHR setting
	if (m_newvideo & 0x20)
	{
		mon_type = 1;
	}

	// IIe RGB card monochrome DHR
	if ((bIsRGB) && (m_rgbmode == 0))
	{
		mon_type = 1;
	}

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = &m_ram_ptr[page];
	uint8_t const *const vaux = (m_aux_ptr ? m_aux_ptr : vram) + page;

	uint8_t vram_row[82];
	vram_row[0] = 0;
	vram_row[81] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int const offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+(col*2)+0] = vaux[offset];
			vram_row[1+(col*2)+1] = vram[offset];
		}

		uint16_t *p = &bitmap.pix(row);

		// RGB DHR 160-wide mode
		if ((bIsRGB) && (m_rgbmode == 2))
		{
			mon_type = 4;
		}

		for (int col = 0; col < 80; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			/*
			    DHGR pixel layout:
			    column & 3 =  0        1        2        3
			               nBBBAAAA nDDCCCCB nFEEEEDD nGGGGFFF

			    n is don't care on the stock hardware's NTSC output.

			    On RGB cards, in mixed mode (DHGR with special mode value == 1), n
			    controls if a pixel quad starting in that byte is color or monochrome.
			    Pixel quads A&B are controlled by n in byte 0, C&D by n in byte 1,
			    E&F by n in byte 2, and G by n in byte 3.
			*/

			switch (mon_type)
			{
				case 0:
					// every 3rd column, the first pixel quad is controlled by the previous
					// byte's MSB, because we always draw 2 quads per column.
					if ((bIsRGBMixed) && ((col & 3) == 3))
					{
						uint32_t tw = (w >> 6);

						if (!(vram_row[col-1] & 0x80))
						{
							for (int b = 0; b < 4; b++)
							{
								v = (tw & 1);
								tw >>= 1;
								*(p++) = v ? WHITE : BLACK;
							}
						}
						else
						{
							for (int b = 0; b < 4; b++)
							{
								v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
								*(p++) = v;
							}
						}

						if (!(vram_row[col] & 0x80))
						{
							for (int b = 4; b < 7; b++)
							{
								v = (tw & 1);
								tw >>= 1;
								*(p++) = v ? WHITE : BLACK;
							}
						}
						else
						{
							for (int b = 4; b < 7; b++)
							{
								v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
								*(p++) = v;
							}
						}
					}
					else
					{
						if ((bIsRGBMixed) && !(vram_row[col] & 0x80))
						{
							uint32_t tw = (w >> 6);
							for (int b = 0; b < 7; b++)
							{
								v = (tw & 1);
								tw >>= 1;
								*(p++) = v ? WHITE : BLACK;
							}
						}
						else
						{
							for (int b = 0; b < 7; b++)
							{
								v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
								*(p++) = v;
							}
						}
					}
					break;

				case 1:
					w >>= 6;
					for (int b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? WHITE : BLACK;
					}
					break;

				case 2:
					w >>= 6;
					for (int b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? GREEN : BLACK;
					}
					break;

				case 3:
					w >>= 6;
					for (int b = 0; b < 7; b++)
					{
						v = (w & 1);
						w >>= 1;
						*(p++) = v ? ORANGE : BLACK;
					}
					break;

				// RGB 160-wide mode (which has a much simpler VRAM layout)
				case 4:
					if (col == 0)
					{
						// Center the 480-wide image in the 560-wide display.
						// Aspect ratio won't be perfect, but it's in range.
						for (int b = 0; b < 40; b++)
						{
							*(p++) = BLACK;
						}
					}
					v = vram_row[col];
					*(p++) = v & 0xf;
					*(p++) = v & 0xf;
					*(p++) = v & 0xf;
					v >>= 4;
					*(p++) = v & 0xf;
					*(p++) = v & 0xf;
					*(p++) = v & 0xf;
					break;
			}
		}

		if (mon_type == 4)
		{
			for (int b = 0; b < 40; b++)
			{
				*(p++) = BLACK;
			}
		}
	}
}

/*
    Latest Apple II NTSC palette from "Apple II Video Display Theory"
    https://docs.google.com/spreadsheets/d/1rKR6A_bVniSCtIP_rrv8QLWJdj4h6jEU1jJj0AebWwg/
*/
static const rgb_t apple2_palette[] =
{
	rgb_t::black(),
	rgb_t(0xa7, 0x0b, 0x40), /* Dark Red */
	rgb_t(0x40, 0x1c, 0xf7), /* Dark Blue */
	rgb_t(0xe6, 0x28, 0xff), /* Purple */
	rgb_t(0x00, 0x74, 0x40), /* Dark Green */
	rgb_t(0x80, 0x80, 0x80), /* Dark Gray */
	rgb_t(0x19, 0x90, 0xff), /* Medium Blue */
	rgb_t(0xbf, 0x9c, 0xff), /* Light Blue */
	rgb_t(0x40, 0x63, 0x00), /* Brown */
	rgb_t(0xe6, 0x6f, 0x00), /* Orange */
	rgb_t(0x80, 0x80, 0x80), /* Light Grey */
	rgb_t(0xff, 0x8b, 0xbf), /* Pink */
	rgb_t(0x19, 0xd7, 0x00), /* Light Green */
	rgb_t(0xbf, 0xe3, 0x08), /* Yellow */
	rgb_t(0x58, 0xf4, 0xbf), /* Aquamarine */
	rgb_t(0xff, 0xff, 0xff)  /* White */
};

void a2_video_device::init_palette()
{
	for (int i = 0; i < std::size(apple2_palette); i++)
		set_pen_color(i, apple2_palette[i]);
}

uint32_t a2_video_device::palette_entries() const
{
	return std::size(apple2_palette);
}

uint32_t a2_video_device::screen_update_GS(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int const beamy = cliprect.top();

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

			uint32_t *const scanline = &bitmap.pix(beamy);
			for (int col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else    // regular screen area
		{
			int const shrline = beamy - BORDER_TOP;

			uint8_t scb;
			if (shrline & 1)
			{
				scb = m_aux_ptr[0x9e80 + (shrline >> 1)];
			}
			else
			{
				scb = m_aux_ptr[0x5e80 + (shrline >> 1)];
			}
			int const palette = ((scb & 0x0f) << 4);

			uint8_t const *const vram = &m_aux_ptr[0x2000 + (shrline * 80)];
			uint8_t const *const vram2 = &m_aux_ptr[0x6000 + (shrline * 80)];

			uint32_t *const scanline = &bitmap.pix(beamy);

			// draw left and right borders
			for (int col = 0; col < BORDER_LEFT; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+640] = m_GSborder_colors[m_GSborder];
			}

			if (scb & 0x80) // 640 mode
			{
				for (int col = 0; col < 80; col++)
				{
					uint8_t b = vram[col];
					scanline[col * 8 + 0 + BORDER_LEFT] = m_shr_palette[palette +  0 + ((b >> 6) & 0x03)];
					scanline[col * 8 + 1 + BORDER_LEFT] = m_shr_palette[palette +  4 + ((b >> 4) & 0x03)];
					scanline[col * 8 + 2 + BORDER_LEFT] = m_shr_palette[palette +  8 + ((b >> 2) & 0x03)];
					scanline[col * 8 + 3 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];

					b = vram2[col];
					scanline[col * 8 + 4 + BORDER_LEFT] = m_shr_palette[palette + 0 + ((b >> 6) & 0x03)];
					scanline[col * 8 + 5 + BORDER_LEFT] = m_shr_palette[palette + 4 + ((b >> 4) & 0x03)];
					scanline[col * 8 + 6 + BORDER_LEFT] = m_shr_palette[palette + 8 + ((b >> 2) & 0x03)];
					scanline[col * 8 + 7 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];
				}
			}
			else        // 320 mode
			{
				// the low 5 bits of the SCB determine the initial fillmode color
				// for the scanline (hardware testing by John Brooks)
				static const uint32_t fillmode_init[32] =
				{
					2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
				};

				uint32_t last_pixel = fillmode_init[scb & 0x1f];
				for (int col = 0; col < 80; col++)
				{
					uint8_t b;
					uint32_t pixel;

					b = vram[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 0 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 1 + BORDER_LEFT] = m_shr_palette[pixel];

					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 2 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 3 + BORDER_LEFT] = m_shr_palette[pixel];

					b = vram2[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 4 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 5 + BORDER_LEFT] = m_shr_palette[pixel];

					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 6 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 7 + BORDER_LEFT] = m_shr_palette[pixel];
				}
			}
		}
	}
	else
	{
		if (beamy >= BORDER_TOP)
		{
			rectangle const new_cliprect(0, 559, cliprect.top() - BORDER_TOP, cliprect.bottom() - BORDER_TOP);
			screen_update_GS_8bit(screen, *m_8bit_graphics, new_cliprect);
		}

		if ((beamy < (BORDER_TOP+4)) || (beamy >= (192+4+BORDER_TOP)))
		{
			if (beamy >= (231+BORDER_TOP))
			{
				return 0;
			}

			uint32_t *const scanline = &bitmap.pix(beamy);
			for (int col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else
		{
			uint32_t *const scanline = &bitmap.pix(beamy);

			// draw left and right borders
			for (int col = 0; col < BORDER_LEFT + 40; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+600] = m_GSborder_colors[m_GSborder];
			}

			uint16_t *a2pixel = &m_8bit_graphics->pix(beamy-(BORDER_TOP+4));
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

	if (cliprect.bottom() > 191)
	{
		return 0;
	}

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
					dhgr_update(screen, bitmap, cliprect, cliprect.top(), cliprect.bottom());
				}
				else
				{
					hgr_update(screen, bitmap, cliprect, cliprect.top(), cliprect.bottom());
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
					dlores_update(screen, bitmap, cliprect, cliprect.top(), cliprect.bottom());
				}
				else
				{
					lores_update(screen, bitmap, cliprect, cliprect.top(), cliprect.bottom());
				}
			}
		}
	}
	else
	{
		text_updateGS(screen, bitmap, cliprect, cliprect.top(), cliprect.bottom());
	}

	m_page2 = old_page2;

	return 0;
}

void a2_video_device::text_updateGS(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint8_t const *const aux_page = m_aux_ptr ? m_aux_ptr : m_ram_ptr;

	uint32_t const start_address = m_page2 ? 0x800 : 0x400;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	//printf("TXT: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	for (int row = startrow; row < stoprow; row += 8)
	{
		if (m_80col)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5 + col));

				plot_text_characterGS(bitmap, col * 14, row, 1, aux_page[address],
									m_GSfg, m_GSbg);
				plot_text_characterGS(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address],
									m_GSfg, m_GSbg);
			}
		}
		else
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5 + col));

				plot_text_characterGS(bitmap, col * 14, row, 2, m_ram_ptr[address], m_GSfg, m_GSbg);
			}
		}
	}
}
