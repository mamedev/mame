// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  video/apple2.c

***************************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "machine/ram.h"

#include "video/apple2.h"

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

#define ALWAYS_REFRESH          0
#define PROFILER_VIDEOTOUCH     PROFILER_USER3

/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    effective_a2 - calculates the effective a2
    register
-------------------------------------------------*/

inline UINT32 apple2_state::effective_a2()
{
	return m_flags & m_a2_videomask;
}


/*-------------------------------------------------
    compute_video_address - performs funky Apple II
    video address lookup
-------------------------------------------------*/

UINT32 apple2_state::compute_video_address(int col, int row)
{
	/* special Apple II addressing - gotta love it */
	return (((row & 0x07) << 7) | ((row & 0x18) * 5 + col));
}



/*-------------------------------------------------
    adjust_begin_and_end_row - processes the cliprect
-------------------------------------------------*/

void apple2_state::adjust_begin_and_end_row(const rectangle &cliprect, int *beginrow, int *endrow)
{
	/* assumptions of the code */
	assert((*beginrow % 8) == 0);
	assert((*endrow % 8) == 7);

	*beginrow = MAX(*beginrow, cliprect.min_y - (cliprect.min_y % 8));
	*endrow = MIN(*endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	/* sanity check again */
	assert((*beginrow % 8) == 0);
	assert((*endrow % 8) == 7);
}



/***************************************************************************
    TEXT
***************************************************************************/

/*-------------------------------------------------
    apple2_plot_text_character - plots a single
    textual character
-------------------------------------------------*/

inline void apple2_state::apple2_plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen, UINT32 my_a2)
{
	int x, y, i;
	int fg = m_fgcolor;
	int bg = m_bgcolor;
	const UINT8 *chardata;
	UINT16 color;

	if (m_sysconfig != nullptr)
	{
		switch (m_sysconfig->read() & 0x03)
		{
			case 0:
				break;  // leave alone

			case 1:
				if ((m_machinetype == APPLE_II) || (m_machinetype == LABA2P) || (m_machinetype == SPACE84))
				{
					bg = WHITE;
				}
				else
				{
					fg = WHITE;
				}
				break;

			case 2:
				if ((m_machinetype == APPLE_II) || (m_machinetype == LABA2P) || (m_machinetype == SPACE84))
				{
					bg = GREEN;
				}
				else
				{
					fg = GREEN;
				}
				break;

			case 3:
				if ((m_machinetype == APPLE_II) || (m_machinetype == LABA2P) || (m_machinetype == SPACE84))
				{
					bg = ORANGE;
				}
				else
				{
					fg = ORANGE;
				}
				break;
		}
	}


	if (my_a2 & VAR_ALTCHARSET)
	{
		/* we're using an alternate charset */
		code |= m_alt_charset_value;
	}
	else if (m_flash && (code >= 0x40) && (code <= 0x7f))
	{
		/* we're flashing; swap */
		i = fg;
		fg = bg;
		bg = i;
	}

	/* look up the character data */
	chardata = &textgfx_data[(code * 8) % textgfx_datalen];

	/* and finally, plot the character itself */
	if ((m_machinetype == SPACE84) || (m_machinetype == LABA2P))
	{
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 7; x++)
			{
				color = (chardata[y] & (1 << (6-x))) ? bg : fg;

				for (i = 0; i < xscale; i++)
				{
					bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
				}
			}
		}
	}
	else
	{
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
}



/*-------------------------------------------------
    apple2_text_draw - renders text (either 40
    column or 80 column)
-------------------------------------------------*/

void apple2_state::apple2_text_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow)
{
	int row, col;
	UINT32 start_address = (page ? 0x0800 : 0x0400);
	UINT32 address;
	UINT32 my_a2 = effective_a2();

	/* perform adjustments */
	adjust_begin_and_end_row(cliprect, &beginrow, &endrow);

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate adderss */
			address = start_address + compute_video_address(col, row / 8);

			if (my_a2 & VAR_80COL)
			{
				apple2_plot_text_character(bitmap, col * 14 + 0, row, 1, m_a2_videoaux[address],
					m_textgfx_data, m_textgfx_datalen, my_a2);
				apple2_plot_text_character(bitmap, col * 14 + 7, row, 1, m_a2_videoram[address],
					m_textgfx_data, m_textgfx_datalen, my_a2);
			}
			else
			{
				apple2_plot_text_character(bitmap, col * 14, row, 2, m_a2_videoram[address],
					m_textgfx_data, m_textgfx_datalen, my_a2);
			}
		}
	}
}


/*-------------------------------------------------
    apple2_lores_draw - renders lo-res text
-------------------------------------------------*/

void apple2_state::apple2_lores_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow)
{
	int row, col, y, x;
	UINT8 code;
	UINT32 start_address = (page ? 0x0800 : 0x0400);
	UINT32 address;

	/* perform adjustments */
	adjust_begin_and_end_row(cliprect, &beginrow, &endrow);

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 40; col++)
		{
			/* calculate adderss */
			address = start_address + compute_video_address(col, row / 8);

			/* perform the lookup */
			code = m_a2_videoram[address];

			/* and now draw */
			for (y = 0; y < 4; y++)
			{
				for (x = 0; x < 14; x++)
					bitmap.pix16(row + y, col * 14 + x) = (code >> 0) & 0x0F;
			}
			for (y = 4; y < 8; y++)
			{
				for (x = 0; x < 14; x++)
					bitmap.pix16(row + y, col * 14 + x) = (code >> 4) & 0x0F;
			}
		}
	}
}


/***************************************************************************
    HIGH RESOLUTION GRAPHICS
***************************************************************************/

void apple2_state::apple2_hires_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow)
{
	const UINT8 *vram, *vaux;
	int row, col, b;
	int offset;
	int columns;
	UINT8 vram_row[82];
	UINT16 v;
	UINT16 *p;
	UINT32 w;
	UINT16 *artifact_map_ptr;
	int mon_type = 0;

	if (m_sysconfig != nullptr)
	{
		mon_type = m_sysconfig->read() & 0x03;
	}

	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
	if (endrow < beginrow)
		return;

	if (m_machinetype == TK2000)
	{
		vram = m_a2_videoram + (page ? 0xa000 : 0x2000);
		vaux = m_a2_videoaux + (page ? 0xa000 : 0x2000);
	}
	else
	{
		vram = m_a2_videoram + (page ? 0x4000 : 0x2000);
		vaux = m_a2_videoaux + (page ? 0x4000 : 0x2000);
	}
	columns     = ((effective_a2() & (VAR_DHIRES|VAR_80COL)) == (VAR_DHIRES|VAR_80COL)) ? 80 : 40;

	vram_row[0] = 0;
	vram_row[columns + 1] = 0;

	for (row = beginrow; row <= endrow; row++)
	{
		for (col = 0; col < 40; col++)
		{
			offset = compute_video_address(col, row / 8) | ((row & 7) << 10);

			switch(columns)
			{
				case 40:
					vram_row[1+col] = vram[offset];
					break;

				case 80:
					vram_row[1+(col*2)+0] = vaux[offset];
					vram_row[1+(col*2)+1] = vram[offset];
					break;

				default:
					fatalerror("Invalid column count\n");
			}
		}

		p = &bitmap.pix16(row);

		for (col = 0; col < columns; col++)
		{
			w =     (((UINT32) vram_row[col+0] & 0x7f) <<  0)
				|   (((UINT32) vram_row[col+1] & 0x7f) <<  7)
				|   (((UINT32) vram_row[col+2] & 0x7f) << 14);

			switch(columns)
			{
				case 40:
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
					break;

				case 80:
					if (m_monochrome_dhr)
					{
						w >>= 7;
						for (b = 0; b < 7; b++)
						{
							v = (w & 1);
							w >>= 1;
							*(p++) = v ? WHITE : BLACK;
						}
					}
					else
					{
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
					break;

				default:
					fatalerror("Invalid column count\n");
			}
		}
	}
}



/***************************************************************************
    VIDEO CORE
***************************************************************************/

void apple2_state::apple2_video_start(const UINT8 *vram, const UINT8 *aux_vram, UINT32 ignored_softswitches, int hires_modulo)
{
	int i, j;
	UINT16 c;
	UINT8 *apple2_font;

	static const UINT8 hires_artifact_color_table[] =
	{
		BLACK,  PURPLE, GREEN,  WHITE,
		BLACK,  BLUE,   ORANGE, WHITE
	};

	static const UINT8 dhires_artifact_color_table[] =
	{
		BLACK,      DKGREEN,    BROWN,  GREEN,
		DKRED,      DKGRAY,     ORANGE, YELLOW,
		DKBLUE,     BLUE,       GRAY,   AQUA,
		PURPLE,     LTBLUE,     PINK,   WHITE
	};

	m_fgcolor = 15;
	m_bgcolor = 0;
	m_flash = 0;
	apple2_font = memregion("gfx1")->base();
	m_alt_charset_value = memregion("gfx1")->bytes() / 16;
	m_a2_videoram = vram;
	m_a2_videoaux = aux_vram;

	m_textgfx_data = memregion("gfx1")->base();
	m_textgfx_datalen = memregion("gfx1")->bytes();

	/* 2^3 dependent pixels * 2 color sets * 2 offsets */
	m_hires_artifact_map = std::make_unique<UINT16[]>(8 * 2 * 2);

	/* 2^4 dependent pixels */
	m_dhires_artifact_map = std::make_unique<UINT16[]>(16);

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
			m_hires_artifact_map[ 0 + j*8 + i] = hires_artifact_color_table[(c + 0) % hires_modulo];
			m_hires_artifact_map[16 + j*8 + i] = hires_artifact_color_table[(c + 4) % hires_modulo];
		}
	}

	/* Fix for Ivel Ultra */
	if (!strcmp(machine().system().name, "ivelultr")) {
		int len = memregion("gfx1")->bytes();
		for (i = 0; i < len; i++)
		{
			apple2_font[i] = BITSWAP8(apple2_font[i],  7, 7, 6, 5, 4, 3, 2, 1);
		}
	}

	/* do we need to flip the gfx? */
	if (!strcmp(machine().system().name, "apple2")
		|| !strcmp(machine().system().name, "apple2p")
		|| !strcmp(machine().system().name, "prav82")
		|| !strcmp(machine().system().name, "prav8m")
		|| !strcmp(machine().system().name, "ivelultr")
		|| !strcmp(machine().system().name, "apple2jp"))
	{
		int len = memregion("gfx1")->bytes();
		for (i = 0; i < len; i++)
		{
			apple2_font[i] = BITSWAP8(apple2_font[i], 7, 0, 1, 2, 3, 4, 5, 6);
		}
	}


	/* build double hires artifact map */
	for (i = 0; i < 16; i++)
	{
		m_dhires_artifact_map[i] = dhires_artifact_color_table[i];
	}

	memset(&m_old_a2, 0, sizeof(m_old_a2));
	m_a2_videomask = ~ignored_softswitches;
}



VIDEO_START_MEMBER(apple2_state,apple2)
{
	apple2_video_start(m_ram->pointer(), m_ram->pointer()+0x10000, VAR_80COL | VAR_ALTCHARSET | VAR_DHIRES, 4);

	/* hack to fix the colors on apple2/apple2p */
	m_fgcolor = 0;
	m_bgcolor = 15;

	m_monochrome_dhr = false;
}


VIDEO_START_MEMBER(apple2_state,apple2p)
{
	apple2_video_start(m_ram->pointer(), m_ram->pointer(), VAR_80COL | VAR_ALTCHARSET | VAR_DHIRES, 8);

	/* hack to fix the colors on apple2/apple2p */
	m_fgcolor = 0;
	m_bgcolor = 15;

	m_monochrome_dhr = false;
}


VIDEO_START_MEMBER(apple2_state,apple2e)
{
	device_a2eauxslot_card_interface *auxslotdevice = m_a2eauxslot->get_a2eauxslot_card();
	if (auxslotdevice)
	{
		apple2_video_start(m_ram->pointer(), auxslotdevice->get_vram_ptr(), auxslotdevice->allow_dhr() ? 0 : VAR_DHIRES, 8);
	}
	else
	{
		apple2_video_start(m_ram->pointer(), m_ram->pointer(), VAR_80COL | VAR_DHIRES, 8);
	}
}


VIDEO_START_MEMBER(apple2_state,apple2c)
{
	apple2_video_start(m_ram->pointer(), m_ram->pointer()+0x10000, 0, 8);
}

UINT32 apple2_state::screen_update_apple2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int page;
	UINT32 new_a2;

	/* calculate the m_flash value */
	m_flash = ((machine().time() * 4).seconds() & 1) ? 1 : 0;

	/* read out relevant softswitch variables; to see what has changed */
	new_a2 = effective_a2();
	if (new_a2 & VAR_80STORE)
		new_a2 &= ~VAR_PAGE2;
	new_a2 &= VAR_TEXT | VAR_MIXED | VAR_HIRES | VAR_DHIRES | VAR_80COL | VAR_PAGE2 | VAR_ALTCHARSET;

	if (ALWAYS_REFRESH || (new_a2 != m_old_a2))
	{
		m_old_a2 = new_a2;
	}

	/* choose which page to use */
	page = (new_a2 & VAR_PAGE2) ? 1 : 0;

	/* choose the video mode to draw */
	if (effective_a2() & VAR_TEXT)
	{
		/* text screen - TK2000 uses HGR for text */
		if (m_machinetype == TK2000)
		{
			apple2_hires_draw(bitmap, cliprect, page, 0, 191);
		}
		else
		{
			apple2_text_draw(bitmap, cliprect, page, 0, 191);
		}
	}
	else if ((effective_a2() & VAR_HIRES) && (effective_a2() & VAR_MIXED))
	{
		/* hi-res on top; text at bottom */
		apple2_hires_draw(bitmap, cliprect, page, 0, 159);
		apple2_text_draw(bitmap, cliprect, page, 160, 191);
	}
	else if (effective_a2() & VAR_HIRES)
	{
		/* hi-res screen */
		apple2_hires_draw(bitmap, cliprect, page, 0, 191);
	}
	else if (effective_a2() & VAR_MIXED)
	{
		/* lo-res on top; text at bottom */
		apple2_lores_draw(bitmap, cliprect, page, 0, 159);
		apple2_text_draw(bitmap, cliprect, page, 160, 191);
	}
	else
	{
		/* lo-res screen */
		apple2_lores_draw(bitmap, cliprect, page, 0, 191);
	}
	return 0;
}

/*
    New implementation
*/

const device_type APPLE2_VIDEO = &device_creator<a2_video_device>;

//-------------------------------------------------
//  a2_video_device - constructor
//-------------------------------------------------

a2_video_device::a2_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, APPLE2_VIDEO, "Apple II video", tag, owner, clock, "a2video", __FILE__)
{
}

void a2_video_device::device_start()
{
	static const UINT8 hires_artifact_color_table[] =
	{
		BLACK,  PURPLE, GREEN,  WHITE,
		BLACK,  BLUE,   ORANGE, WHITE
	};
	static const UINT8 dhires_artifact_color_table[] =
	{
		BLACK,      DKGREEN,    BROWN,  GREEN,
		DKRED,      DKGRAY,     ORANGE, YELLOW,
		DKBLUE,     BLUE,       GRAY,   AQUA,
		PURPLE,     LTBLUE,     PINK,   WHITE
	};

	// generate hi-res artifact data
	int i, j;
	UINT16 c;

	/* 2^3 dependent pixels * 2 color sets * 2 offsets */
	m_hires_artifact_map = std::make_unique<UINT16[]>(8 * 2 * 2);

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
	m_dhires_artifact_map = std::make_unique<UINT16[]>(16);

	/* build double hires artifact map */
	for (i = 0; i < 16; i++)
	{
		m_dhires_artifact_map[i] = dhires_artifact_color_table[i];
	}

	save_item(NAME(m_page2));
	save_item(NAME(m_flash));
	save_item(NAME(m_mix));
	save_item(NAME(m_graphics));
	save_item(NAME(m_hires));
	save_item(NAME(m_dhires));
	save_item(NAME(m_80col));
	save_item(NAME(m_altcharset));
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
}

void a2_video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const UINT8 *chardata;
	UINT16 color;

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

void a2_video_device::plot_text_character_orig(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const UINT8 *chardata;
	UINT16 color;

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

void a2_video_device::plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const UINT8 *chardata;
	UINT16 color;

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

void a2_video_device::lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, y, x;
	UINT8 code;
	UINT32 start_address = m_page2 ? 0x0800 : 0x0400;
	UINT32 address;
	int fg = 0;

	switch (m_sysconfig & 0x03)
	{
		case 0: fg = WHITE; break;
		case 1: fg = WHITE; break;
		case 2: fg = GREEN; break;
		case 3: fg = ORANGE; break;
	}

	/* perform adjustments */
	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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
					for (x = 0; x < 14; x++)
						bitmap.pix16(row + y, col * 14 + x) = (code >> 0) & 0x0F;
				}
				for (y = 4; y < 8; y++)
				{
					for (x = 0; x < 14; x++)
						bitmap.pix16(row + y, col * 14 + x) = (code >> 4) & 0x0F;
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
				UINT8 bits;

				/* calculate adderss */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				code = m_ram_ptr[address];

				bits = (code >> 0) & 0x0F;
				/* and now draw */
				for (y = 0; y < 4; y++)
				{
					for (x = 0; x < 14; x++)
					{
						bitmap.pix16(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
					}
				}

				bits = (code >> 4) & 0x0F;
				for (y = 4; y < 8; y++)
				{
					for (x = 0; x < 14; x++)
						bitmap.pix16(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
				}
			}
		}
	}
}

void a2_video_device::dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, y;
	UINT8 code, auxcode;
	UINT32 start_address = m_page2 ? 0x0800 : 0x0400;
	UINT32 address;
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
	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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
					UINT16 *vram = &bitmap.pix16(row + y, (col * 14));

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
					UINT16 *vram = &bitmap.pix16(row + y, (col * 14));

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
				UINT8 bits, abits;

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
					UINT16 *vram = &bitmap.pix16(row + y, (col * 14));

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
					UINT16 *vram = &bitmap.pix16(row + y, (col * 14));

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
	UINT32 start_address;
	UINT32 address;
	UINT8 *aux_page = m_ram_ptr;
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

	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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
	UINT32 start_address = m_page2 ? 0x800 : 0x400;
	UINT32 address;
	int fg = 0;
	int bg = 0;

	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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

void a2_video_device::text_update_ultr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	UINT32 start_address = m_page2 ? 0x800 : 0x400;
	UINT32 address;
	int fg = 0;
	int bg = 0;

	beginrow = MAX(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = MIN(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

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
	const UINT8 *vram;
	int row, col, b;
	int offset;
	UINT8 vram_row[42];
	UINT16 v;
	UINT16 *p;
	UINT32 w;
	UINT16 *artifact_map_ptr;
	int mon_type = m_sysconfig & 0x03;
	int begincol = 0, endcol = 40;

	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
	if (endrow < beginrow)
		return;

	// we generate 2 pixels per "column" so adjust
	if (begincol < (cliprect.min_x/14))
		begincol = (cliprect.min_x/14);
	if (endcol > (cliprect.max_x/14))
		endcol = (cliprect.max_x/14);
	if (cliprect.max_x > 39*14)
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
			w =     (((UINT32) vram_row[col+0] & 0x7f) <<  0)
				|   (((UINT32) vram_row[col+1] & 0x7f) <<  7)
				|   (((UINT32) vram_row[col+2] & 0x7f) << 14);


			// verified on h/w: setting dhires w/o 80col emulates a rev. 0 Apple ][ with no orange/blue
			if (m_dhires)
			{
				artifact_map_ptr = m_hires_artifact_map.get();
			}
			else
			{
				artifact_map_ptr = &m_hires_artifact_map[((vram_row[col + 1] & 0x80) >> 7) * 16];
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
	const UINT8 *vram;
	int row, col, b;
	int offset;
	UINT8 vram_row[42];
	UINT16 v;
	UINT16 *p;
	UINT32 w;
	UINT16 *artifact_map_ptr;
	int mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
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
			w =     (((UINT32) vram_row[col+0] & 0x7f) <<  0)
				|   (((UINT32) vram_row[col+1] & 0x7f) <<  7)
				|   (((UINT32) vram_row[col+2] & 0x7f) << 14);

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
	const UINT8 *vram, *vaux;
	int row, col, b;
	int offset;
	UINT8 vram_row[82];
	UINT16 v;
	UINT16 *p;
	UINT32 w;
	int page = m_page2 ? 0x4000 : 0x2000;
	int mon_type = m_sysconfig & 0x03;

	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
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
			w =     (((UINT32) vram_row[col+0] & 0x7f) <<  0)
				|   (((UINT32) vram_row[col+1] & 0x7f) <<  7)
				|   (((UINT32) vram_row[col+2] & 0x7f) << 14);

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
	rgb_t::black,
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

/* Initialize the palette */
PALETTE_INIT_MEMBER(a2_video_device, apple2)
{
	palette.set_pen_colors(0, apple2_palette, ARRAY_LENGTH(apple2_palette));
}
