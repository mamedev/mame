// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intelligent Designs NICK emulation

**********************************************************************/

#include "nick.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

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

/* No of highest resolution pixels per "clock" */
#define NICK_PIXELS_PER_CLOCK   16

/* "clocks" per line */
#define NICK_TOTAL_CLOCKS_PER_LINE  64



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NICK = &device_creator<nick_device>;


DEVICE_ADDRESS_MAP_START( vram_map, 8, nick_device )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(vram_r, vram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( vio_map, 8, nick_device )
	AM_RANGE(0x00, 0x00) AM_WRITE(fixbias_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(border_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(lpl_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(lph_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nick_map, AS_0, 8, nick_device )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nick_device - constructor
//-------------------------------------------------

nick_device::nick_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NICK, "NICK", tag, owner, clock, "nick", __FILE__),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_space_config("vram", ENDIANNESS_LITTLE, 8, 16, 0, *ADDRESS_MAP_NAME(nick_map)),
		m_write_virq(*this),
		m_scanline_count(0),
		m_FIXBIAS(0),
		m_BORDER(0),
		m_LPL(0),
		m_LPH(0),
		m_LD1(0),
		m_LD2(0),
		m_virq(CLEAR_LINE)
{
	memset(&m_LPT, 0x00, sizeof(m_LPT));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nick_device::device_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
	calc_visible_clocks(ENTERPRISE_SCREEN_WIDTH);

	// initialize palette
	initialize_palette();

	// resolve callbacks
	m_write_virq.resolve_safe();

	// allocate timers
	m_timer_scanline = timer_alloc();
	m_timer_scanline->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->scan_period());

	// state saving
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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nick_device::device_reset()
{
	m_write_virq(CLEAR_LINE);
	m_virq = 0;

	m_scanline_count = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nick_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int scanline = m_screen->vpos();

	if (scanline < ENTERPRISE_SCREEN_HEIGHT)
	{
		/* set write address for line */
		m_dest = &m_bitmap.pix32(scanline);
		m_dest_pos = 0;
		m_dest_max_pos = m_bitmap.width();

		/* write line */
		do_line();
	}
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *nick_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

UINT32 nick_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  vram_r - video RAM read
//-------------------------------------------------

READ8_MEMBER( nick_device::vram_r )
{
	return this->space().read_byte(offset);
}


//-------------------------------------------------
//  vram_w - video RAM write
//-------------------------------------------------

WRITE8_MEMBER( nick_device::vram_w )
{
	this->space().write_byte(offset, data);
}


//-------------------------------------------------
//  fixbias_w -
//-------------------------------------------------

WRITE8_MEMBER( nick_device::fixbias_w )
{
	m_FIXBIAS = data;
}


//-------------------------------------------------
//  border_w -
//-------------------------------------------------

WRITE8_MEMBER( nick_device::border_w )
{
	m_BORDER = data;
}


//-------------------------------------------------
//  lpl_w -
//-------------------------------------------------

WRITE8_MEMBER( nick_device::lpl_w )
{
	m_LPL = m_reg[2] = data;

	update_lpt();
}


//-------------------------------------------------
//  lph_w -
//-------------------------------------------------

WRITE8_MEMBER( nick_device::lph_w )
{
	m_LPH = m_reg[3] = data;

	update_lpt();
}


//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

void nick_device::initialize_palette()
{
	const int resistances_rg[] = { (int) RES_R(470), (int) RES_R(220), (int) RES_R(100) };
	const int resistances_b[] = { (int) RES_R(220), (int) RES_R(82) };

	double color_weights_rg[3], color_weights_b[2];

	compute_resistor_weights(0, 0xff, -1.0,
								3, resistances_rg, color_weights_rg, 0, 0,
								2, resistances_b,  color_weights_b,  0, 0,
								0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < 256; i++)
	{
		/*

		    bit     description

		    PC0     100R -- RED
		    PC1     100R -- GREEN
		    PC2      82R -- BLUE
		    PC3     220R -- RED
		    PC4     220R -- GREEN
		    PC5     220R -- BLUE
		    PC6     470R -- RED
		    PC7     470R -- GREEN

		*/

		int ra = BIT(i, 0);
		int rb = BIT(i, 3);
		int rc = BIT(i, 6);

		int ga = BIT(i, 1);
		int gb = BIT(i, 4);
		int gc = BIT(i, 7);

		int ba = BIT(i, 2);
		int bb = BIT(i, 5);

		UINT8 r = combine_3_weights(color_weights_rg, rc, rb, ra);
		UINT8 g = combine_3_weights(color_weights_rg, gc, gb, ga);
		UINT8 b = combine_2_weights(color_weights_b, bb, ba);

		m_palette[i] = rgb_t(r, g, b);
	}

	for (int i = 0; i < 256; i++)
	{
		int pen_index;

		pen_index = (BIT(i, 7) << 0) | (BIT(i, 3) << 1);
		m_pen_idx_4col[i] = pen_index;

		pen_index =  (BIT(i, 7) << 0) | (BIT(i, 3) << 1) |  (BIT(i, 5) << 2) | (BIT(i, 1) << 3);
		m_pen_idx_16col[i] = pen_index;
	}
}

// MESS specific
/* 8-bit pixel write! */
void nick_device::write_pixel(int ci)
{
	if (m_dest_pos < m_dest_max_pos)
	{
		m_dest[m_dest_pos++] = m_palette[ci];
	}
}

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
			//osd_printf_info("4 colour\r\n");

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
			//osd_printf_info("16 colour\r\n");

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
			//osd_printf_info("4 colour\r\n");

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
			//osd_printf_info("16 colour\r\n");

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
		buf1 = space().read_byte(m_LD1);
		m_LD1++;

		buf2 = space().read_byte(m_LD1);
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
		buf1 = space().read_byte(m_LD1);
		m_LD1++;

		write_pixels_lpixel(buf1, buf1);
	}
}

void nick_device::do_attr(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = space().read_byte(m_LD1);
		m_LD1++;

		buf2 = space().read_byte(m_LD2);
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
		buf1 = space().read_byte(m_LD1);
		m_LD1++;
		buf2 = space().read_byte(ADDR_CH256(m_LD2, buf1));

		write_pixels_lpixel(buf2, buf1);
	}
}

void nick_device::do_ch128(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = space().read_byte(m_LD1);
		m_LD1++;
		buf2 = space().read_byte(ADDR_CH128(m_LD2, buf1));

		write_pixels_lpixel(buf2, buf1);
	}
}

void nick_device::do_ch64(int clocks_visible)
{
	UINT8 buf1, buf2;

	for (int i = 0; i < clocks_visible; i++)
	{
		buf1 = space().read_byte(m_LD1);
		m_LD1++;
		buf2 = space().read_byte(ADDR_CH64(m_LD2, buf1));

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
				//osd_printf_info("attr mode\r\n");
				do_attr(clocks_visible);
			}
			break;

			case NICK_CH256_MODE:
			{
				//osd_printf_info("ch256 mode\r\n");
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
				//osd_printf_info("ch64 mode\r\n");
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
	m_LPT.SC = space().read_byte(LPT_Addr);
	m_LPT.MB = space().read_byte(LPT_Addr + 1);
	m_LPT.LM = space().read_byte(LPT_Addr + 2);
	m_LPT.RM = space().read_byte(LPT_Addr + 3);
	m_LPT.LD1L = space().read_byte(LPT_Addr + 4);
	m_LPT.LD1H = space().read_byte(LPT_Addr + 5);
	m_LPT.LD2L = space().read_byte(LPT_Addr + 6);
	m_LPT.LD2H = space().read_byte(LPT_Addr + 7);
	m_LPT.COL[0] = space().read_byte(LPT_Addr + 8);
	m_LPT.COL[1] = space().read_byte(LPT_Addr + 9);
	m_LPT.COL[2] = space().read_byte(LPT_Addr + 10);
	m_LPT.COL[3] = space().read_byte(LPT_Addr + 11);
	m_LPT.COL[4] = space().read_byte(LPT_Addr + 12);
	m_LPT.COL[5] = space().read_byte(LPT_Addr + 13);
	m_LPT.COL[6] = space().read_byte(LPT_Addr + 14);
	m_LPT.COL[7] = space().read_byte(LPT_Addr + 15);
}

/* call here to render a line of graphics */
void nick_device::do_line()
{
	UINT8 scanline;

	m_write_virq((m_LPT.MB & NICK_MB_VIRQ) ? ASSERT_LINE : CLEAR_LINE);

	if (m_virq && !(m_LPT.MB & NICK_MB_VIRQ))
	{
		m_timer_scanline->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->scan_period());
	}

	m_virq = (m_LPT.MB & NICK_MB_VIRQ) ? 1 : 0;

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
