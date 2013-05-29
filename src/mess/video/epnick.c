/*****************************************************************************
 *
 * video/epnick.c
 *
 * Nick Graphics Chip - found in Enterprise
 *
 * this is a display list graphics chip, with bitmap,
 * character and attribute graphics modes. Each entry in the
 * display list defines a char line, with variable number of
 * scanlines. Colour modes are 2,4, 16 and 256 colour.
 * Nick has 256 colours, 3 bits for R and G, with 2 bits for Blue.
 * It's a nice and flexible graphics processor..........
 *
 ****************************************************************************/

#include "emu.h"
#include "video/epnick.h"

/* colour mode types */
#define NICK_2_COLOUR_MODE  0
#define NICK_4_COLOUR_MODE  1
#define NICK_16_COLOUR_MODE 2
#define NICK_256_COLOUR_MODE    3

/* Display mode types */
#define NICK_VSYNC_MODE 0
#define NICK_PIXEL_MODE 1
#define NICK_ATTR_MODE  2
#define NICK_CH256_MODE 3
#define NICK_CH128_MODE 4
#define NICK_CH64_MODE  5
#define NICK_UNUSED_MODE    6
#define NICK_LPIXEL_MODE    7

/* MODEBYTE defines */
#define NICK_MB_VIRQ            (1<<7)
#define NICK_MB_VRES            (1<<4)
#define NICK_MB_LPT_RELOAD      (1<<0)

/* Left margin defines */
#define NICK_LM_MSBALT          (1<<7)
#define NICK_LM_LSBALT          (1<<6)

/* Right margin defines */
#define NICK_RM_ALTIND1         (1<<7)
#define NICK_RM_ALTIND0         (1<<6)

/* useful macros */
#define NICK_GET_LEFT_MARGIN(x)     (x & 0x03f)
#define NICK_GET_RIGHT_MARGIN(x)    (x & 0x03f)
#define NICK_GET_DISPLAY_MODE(x)    ((x>>1) & 0x07)
#define NICK_GET_COLOUR_MODE(x)     ((x>>5) & 0x03)

#define NICK_RELOAD_LPT(x)          (x & 0x080)
#define NICK_CLOCK_LPT(x)           (x & 0x040)

/* Macros to generate memory address is CHx modes */
/* x = LD2, y = buf1 */
#define ADDR_CH256(x,y)     (((x & 0x0ff)<<8) | (y & 0x0ff))
#define ADDR_CH128(x,y)     (((x & 0x01ff)<<7) | (y & 0x07f))
#define ADDR_CH64(x,y)      (((x & 0x03ff)<<6) | (y & 0x03f))


const device_type NICK = &device_creator<nick_device>;

//-------------------------------------------------
//  stic_device - constructor
//-------------------------------------------------

nick_device::nick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
			device_t(mconfig, NICK, "Nick Graphics Chip", tag, owner, clock, "epnick", __FILE__),
			m_videoram(NULL)
{
}


//-------------------------------------------------
//  ~stic_device - destructor
//-------------------------------------------------

nick_device::~nick_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nick_device::device_start()
{
	machine().primary_screen->register_screen_bitmap(m_bitmap);
	
	calc_visible_clocks(ENTERPRISE_SCREEN_WIDTH);

	bitmap_ind16 m_bitmap;
	
	save_item(NAME(m_scanline_count));
	save_item(NAME(m_FIXBIAS));
	save_item(NAME(m_BORDER));
	save_item(NAME(m_LPL));
	save_item(NAME(m_LPH));
	save_item(NAME(m_LD1));
	save_item(NAME(m_LD2));
	save_item(NAME(m_LPT.SC));
	save_item(NAME(m_LPT.MB));
	save_item(NAME(m_LPT.LM));
	save_item(NAME(m_LPT.RM));
	save_item(NAME(m_LPT.LD1L));
	save_item(NAME(m_LPT.LD1H));
	save_item(NAME(m_LPT.LD2L));
	save_item(NAME(m_LPT.LD2H));
	save_item(NAME(m_LPT.COL));
	save_item(NAME(m_dest_pos));
	save_item(NAME(m_dest_max_pos));
	save_item(NAME(m_reg));
	save_item(NAME(m_first_visible_clock));
	save_item(NAME(m_last_visible_clock));
}


void nick_device::device_reset()
{	
	for (int i = 0; i < 256; i++)
	{
		int pen_index;
		
		pen_index = (BIT(i, 7) << 0) | (BIT(i, 3) << 1);
		m_pen_idx_4col[i] = pen_index;
		
		pen_index =  (BIT(i, 7) << 0) | (BIT(i, 3) << 1) |  (BIT(i, 5) << 2) | (BIT(i, 1) << 3);
		m_pen_idx_16col[i] = pen_index;
	}
	
	//m_BORDER = 0;
	//m_FIXBIAS = 0;
}


// MESS specific
/* 8-bit pixel write! */
void nick_device::write_pixel(int ci)
{
	if (m_dest_pos < m_dest_max_pos)
	{
		m_dest[m_dest_pos++] = ci;
	}
}


/* No of highest resolution pixels per "clock" */
#define NICK_PIXELS_PER_CLOCK   16

/* "clocks" per line */
#define NICK_TOTAL_CLOCKS_PER_LINE  64

/* we align based on the clocks */
void nick_device::calc_visible_clocks(int width)
{
	/* number of clocks we can see */
	int no_visible_clocks = width / NICK_PIXELS_PER_CLOCK;
	m_first_visible_clock = (NICK_TOTAL_CLOCKS_PER_LINE - no_visible_clocks) >> 1;
	m_last_visible_clock = m_first_visible_clock + no_visible_clocks;
}


/* write border colour */
void nick_device::write_border(int clocks)
{
	int col_index = m_BORDER;

	for (int i = 0; i < (clocks << 4); i++)
		write_pixel(col_index);
}


void nick_device::do_left_margin()
{
	UINT8 left = NICK_GET_LEFT_MARGIN(m_LPT.LM);

	if (left > m_first_visible_clock)
	{
		/* some of the left margin is visible */
		UINT8 left_visible = left - m_first_visible_clock;

		/* render the border */
		write_border(left_visible);
	}
}

void nick_device::do_right_margin()
{
	UINT8 right = NICK_GET_RIGHT_MARGIN(m_LPT.RM);

	if (right < m_last_visible_clock)
	{
		/* some of the right margin is visible */
		UINT8 right_visible = m_last_visible_clock - right;

		/* render the border */
		write_border(right_visible);
	}
}

int nick_device::get_color_index(int pen_index)
{
	if (pen_index & 0x08)
		return ((m_FIXBIAS & 0x01f) << 3) | (pen_index & 0x07);
	else
		return m_LPT.COL[pen_index];
}

void nick_device::write_pixels2color(UINT8 pen0, UINT8 pen1, UINT8 data_byte)
{
	int col_index[2];
	int pen_index;
	UINT8 data = data_byte;

	col_index[0] = get_color_index(pen0);
	col_index[1] = get_color_index(pen1);

	for (int i = 0; i < 8; i++)
	{
		pen_index = col_index[BIT(data, 7)];
		write_pixel(pen_index);
		data <<= 1;
	}
}

void nick_device::write_pixels2color_lpixel(UINT8 pen0, UINT8 pen1, UINT8 data_byte)
{
	int col_index[2];
	int pen_index;
	UINT8 data = data_byte;
	
	col_index[0] = get_color_index(pen0);
	col_index[1] = get_color_index(pen1);
	
	for (int i = 0; i < 8; i++)
	{
		pen_index = col_index[BIT(data, 7)];
		write_pixel(pen_index);
		write_pixel(pen_index);
		data <<= 1;
	}
}


void nick_device::write_pixels(UINT8 data_byte, UINT8 char_idx)
{
	/* pen index colour 2-C (0,1), 4-C (0..3) 16-C (0..16) */
	int pen_idx;
	/* Col index = EP colour value */
	int pal_idx;
	UINT8 color_mode = NICK_GET_COLOUR_MODE(m_LPT.MB);
	UINT8 data = data_byte;

	switch (color_mode)
	{
		case NICK_2_COLOUR_MODE:
		{
			int pen_offs = 0;

			/* do before displaying byte */

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				if (data & 0x080)
				{
					pen_offs |= 2;
				}

				data &=~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				if (data & 0x001)
				{
					pen_offs |= 4;
				}

				data &=~0x01;
			}

			if (m_LPT.RM & NICK_RM_ALTIND1)
			{
				if (char_idx & 0x080)
				{
					pen_offs |= 0x02;
				}
			}

#if 0
			if (m_LPT.RM & NICK_RM_ALTIND0)
			{
				if (data & 0x040)
				{
					pen_offs |= 0x04;
				}
			}
#endif


			write_pixels2color(pen_offs, (pen_offs | 0x01), data);
		}
		break;

		case NICK_4_COLOUR_MODE:
		{
			//mame_printf_info("4 colour\r\n");

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			for (int i = 0; i < 4; i++)
			{
				pen_idx = m_pen_idx_4col[data];
				pal_idx = m_LPT.COL[pen_idx & 0x03];

				write_pixel(pal_idx);
				write_pixel(pal_idx);

				data <<= 1;
			}
		}
		break;

		case NICK_16_COLOUR_MODE:
		{
			//mame_printf_info("16 colour\r\n");

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			for (int i = 0; i < 2; i++)
			{
				pen_idx = m_pen_idx_16col[data];

				pal_idx = get_color_index(pen_idx);

				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);

				data <<= 1;
			}
		}
		break;

		case NICK_256_COLOUR_MODE:
		{
			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			pal_idx = data;

			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);


		}
		break;
	}
}

void nick_device::write_pixels_lpixel(UINT8 data_byte, UINT8 char_idx)
{
	/* pen index colour 2-C (0,1), 4-C (0..3) 16-C (0..16) */
	int pen_idx;
	/* Col index = EP colour value */
	int pal_idx;
	UINT8 color_mode = NICK_GET_COLOUR_MODE(m_LPT.MB);
	UINT8 data = data_byte;

	switch (color_mode)
	{
		case NICK_2_COLOUR_MODE:
		{
			int pen_offs = 0;

			/* do before displaying byte */

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				if (data & 0x080)
				{
					pen_offs |= 2;
				}

				data &=~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				if (data & 0x001)
				{
					pen_offs |= 4;
				}

				data &=~0x01;
			}

			if (m_LPT.RM & NICK_RM_ALTIND1)
			{
				if (char_idx & 0x080)
				{
					pen_offs |= 0x02;
				}
			}

#if 0
			if (m_LPT.RM & NICK_RM_ALTIND0)
			{
				if (data & 0x040)
				{
					pen_offs |= 0x04;
				}
			}
#endif


			write_pixels2color_lpixel(pen_offs, (pen_offs | 0x01), data);
		}
		break;

		case NICK_4_COLOUR_MODE:
		{
			//mame_printf_info("4 colour\r\n");

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			for (int i = 0; i < 4; i++)
			{
				pen_idx = m_pen_idx_4col[data];
				pal_idx = m_LPT.COL[pen_idx & 0x03];

				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);

				data <<= 1;
			}
		}
		break;

		case NICK_16_COLOUR_MODE:
		{
			//mame_printf_info("16 colour\r\n");

			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			for (int i = 0; i < 2; i++)
			{
				pen_idx = m_pen_idx_16col[data];
				pal_idx = get_color_index(pen_idx);

				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);
				write_pixel(pal_idx);

				data <<= 1;
			}
		}
		break;

		case NICK_256_COLOUR_MODE:
		{
			/* left margin attributes */
			if (m_LPT.LM & NICK_LM_MSBALT)
			{
				data &= ~0x080;
			}

			if (m_LPT.LM & NICK_LM_LSBALT)
			{
				data &= ~0x01;
			}


			pal_idx = data;

			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);

			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);
			write_pixel(pal_idx);


		}
		break;
	}
}


void nick_device::do_pixel(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;

		buf2 = fetch_byte(m_LD1);
		m_LD1++;

		write_pixels(buf1, buf1);
		write_pixels(buf2, buf1);
	}
}


void nick_device::do_lpixel(int clocks_visible)
{
	UINT8 buf1;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;

		write_pixels_lpixel(buf1, buf1);
	}
}

void nick_device::do_attr(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;

		buf2 = fetch_byte(m_LD2);
		m_LD2++;

		{
			UINT8 bg_color = ((buf1 >> 4) & 0x0f);
			UINT8 fg_color = (buf1 & 0x0f);

			write_pixels2color_lpixel(bg_color, fg_color, buf2);
		}
	}
}

void nick_device::do_ch256(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;
		buf2 = fetch_byte(ADDR_CH256(m_LD2, buf1));

		write_pixels_lpixel(buf2, buf1);
	}
}

void nick_device::do_ch128(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;
		buf2 = fetch_byte(ADDR_CH128(m_LD2, buf1));

		write_pixels_lpixel(buf2, buf1);
	}
}

void nick_device::do_ch64(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = fetch_byte(m_LD1);
		m_LD1++;
		buf2 = fetch_byte(ADDR_CH64(m_LD2, buf1));

		write_pixels_lpixel(buf2, buf1);
	}
}


void nick_device::do_display()
{
	LPT_ENTRY *pLPT = &m_LPT;
	UINT8 clocks_visible;
	UINT8 right_margin = NICK_GET_RIGHT_MARGIN(pLPT->RM);
	UINT8 left_margin = NICK_GET_LEFT_MARGIN(pLPT->LM);

	clocks_visible = right_margin - left_margin;

	if (clocks_visible)
	{
		/* get display mode */
		UINT8 display_mode = NICK_GET_DISPLAY_MODE(pLPT->MB);

		if (m_scanline_count == 0)   // ||
			//((pLPT->MB & NICK_MB_VRES)==0))
		{
			/* doing first line */
			/* reload LD1, and LD2 (if necessary) regardless of display mode */
			m_LD1 = (pLPT->LD1L & 0xff) | ((pLPT->LD1H & 0xff) << 8);

			if ((display_mode != NICK_LPIXEL_MODE) && (display_mode != NICK_PIXEL_MODE))
			{
				/* lpixel and pixel modes don't use LD2 */
				m_LD2 = (pLPT->LD2L & 0xff) | ((pLPT->LD2H & 0xff) << 8);
			}
		}
		else
		{
			/* not first line */
			switch (display_mode)
			{
				case NICK_ATTR_MODE:
				{
					/* reload LD1 */
					m_LD1 = (pLPT->LD1L & 0xff) | ((pLPT->LD1H & 0xff) << 8);
				}
				break;

				case NICK_CH256_MODE:
				case NICK_CH128_MODE:
				case NICK_CH64_MODE:
				{
					/* reload LD1 */
					m_LD1 = (pLPT->LD1L & 0xff) | ((pLPT->LD1H & 0xff) << 8);
					m_LD2++;
				}
				break;

				default:
					break;
			}
		}

		switch (display_mode)
		{
			case NICK_PIXEL_MODE:
			{
				do_pixel(clocks_visible);
			}
			break;

			case NICK_ATTR_MODE:
			{
				//mame_printf_info("attr mode\r\n");
				do_attr(clocks_visible);
			}
			break;

			case NICK_CH256_MODE:
			{
				//mame_printf_info("ch256 mode\r\n");
				do_ch256(clocks_visible);
			}
			break;

			case NICK_CH128_MODE:
			{
				do_ch128(clocks_visible);
			}
			break;

			case NICK_CH64_MODE:
			{
				//mame_printf_info("ch64 mode\r\n");
				do_ch64(clocks_visible);
			}
			break;

			case NICK_LPIXEL_MODE:
			{
				do_lpixel(clocks_visible);
			}
			break;

			default:
				break;
		}
	}
}

void nick_device::update_lpt()
{
	UINT16 CurLPT = (m_LPL & 0x0ff) | ((m_LPH & 0x0f) << 8);
	CurLPT++;
	m_LPL = CurLPT & 0x0ff;
	m_LPH = (m_LPH & 0x0f0) | ((CurLPT >> 8) & 0x0f);
}


void nick_device::reload_lpt()
{
	/* get addr of LPT */
	UINT32 LPT_Addr = ((m_LPL & 0x0ff) << 4) | ((m_LPH & 0x0f) << (8+4));

	/* update internal LPT state */
	m_LPT.SC = fetch_byte(LPT_Addr);
	m_LPT.MB = fetch_byte(LPT_Addr + 1);
	m_LPT.LM = fetch_byte(LPT_Addr + 2);
	m_LPT.RM = fetch_byte(LPT_Addr + 3);
	m_LPT.LD1L = fetch_byte(LPT_Addr + 4);
	m_LPT.LD1H = fetch_byte(LPT_Addr + 5);
	m_LPT.LD2L = fetch_byte(LPT_Addr + 6);
	m_LPT.LD2H = fetch_byte(LPT_Addr + 7);
	m_LPT.COL[0] = fetch_byte(LPT_Addr + 8);
	m_LPT.COL[1] = fetch_byte(LPT_Addr + 9);
	m_LPT.COL[2] = fetch_byte(LPT_Addr + 10);
	m_LPT.COL[3] = fetch_byte(LPT_Addr + 11);
	m_LPT.COL[4] = fetch_byte(LPT_Addr + 12);
	m_LPT.COL[5] = fetch_byte(LPT_Addr + 13);
	m_LPT.COL[6] = fetch_byte(LPT_Addr + 14);
	m_LPT.COL[7] = fetch_byte(LPT_Addr + 15);
}

/* call here to render a line of graphics */
void nick_device::do_line()
{
	UINT8 scanline;

	if ((m_LPT.MB & NICK_MB_LPT_RELOAD)!=0)
	{
		/* reload LPT */

		m_LPL = m_reg[2];
		m_LPH = m_reg[3];

		reload_lpt();
	}

	/* left border */
	do_left_margin();

	/* do visible part */
	do_display();

	/* right border */
	do_right_margin();

	// 0x0f7 is first!
	/* scan line count for this LPT */
	scanline = ((~m_LPT.SC) + 1) & 0x0ff;

	//printf("scanline %02x\r\n", scanline);

	/* update count of scanlines done so far */
	m_scanline_count++;

	if (m_scanline_count == scanline)
	{
		/* done all scanlines of this Line Parameter Table, get next */

		m_scanline_count = 0;

		update_lpt();
		reload_lpt();
	}
}

WRITE8_MEMBER( nick_device::reg_w )
{
	//mame_printf_info("Nick write %02x %02x\r\n",offset, data);

	/* write to a nick register */
	m_reg[offset & 0x0f] = data;

	if ((offset == 0x03) || (offset == 0x02))
	{
		/* write LPH */

		/* reload LPT base? */
		//if (NICK_RELOAD_LPT(data))
		{
			/* reload LPT base pointer */
			m_LPL = m_reg[2];
			m_LPH = m_reg[3];

			reload_lpt();
		}
	}

	if (offset == 0x01)
	{
		m_BORDER = data;
	}

	if (offset == 0x00)
	{
		m_FIXBIAS = data;
	}
}

void nick_device::do_screen(bitmap_ind16 &bm)
{
	int line = 0;

	do
	{
		/* set write address for line */
		m_dest = &bm.pix16(line);
		m_dest_pos = 0;
		m_dest_max_pos = bm.width();

		/* write line */
		do_line();

		/* next line */
		line++;
	}
	while (((m_LPT.MB & 0x080) == 0) && (line < ENTERPRISE_SCREEN_HEIGHT));

}


UINT32 nick_device::screen_update_epnick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	do_screen(m_bitmap);
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
