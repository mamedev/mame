/***************************************************************************

  video/apple2.c

***************************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "machine/ram.h"

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
					break;
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
					artifact_map_ptr = &m_hires_artifact_map[((vram_row[col+1] & 0x80) >> 7) * 16];
					for (b = 0; b < 7; b++)
					{
						v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
						*(p++) = v;
						*(p++) = v;
					}
					break;

				case 80:
					if (m_monochrome_dhr)
					{
						for (b = 0; b < 7; b++)
						{
							v = (w & 1);
							w >>= 1;
							*(p++) = v ? WHITE : BLACK;
						}
					}
					else
					{
						for (b = 0; b < 7; b++)
						{
							v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
							*(p++) = v;
						}
					}
					break;

				default:
					fatalerror("Invalid column count\n");
					break;
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
	m_hires_artifact_map = auto_alloc_array(machine(), UINT16, 8 * 2 * 2);

	/* 2^4 dependent pixels */
	m_dhires_artifact_map = auto_alloc_array(machine(), UINT16, 16);

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
		|| !strcmp(machine().system().name, "ace100")
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
	m_flash = ((machine().time() * 4).seconds & 1) ? 1 : 0;

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
