/***************************************************************************

    video/oric.c

    All graphic effects are supported including mid-line changes.
    There may be some small bugs.

    TODO:
    - speed up this code a bit?

***************************************************************************/

#include "includes/oric.h"

TIMER_CALLBACK_MEMBER(oric_state::oric_vh_timer_callback)
{
	/* update flash count */
	m_vh_state.flash_count++;
}

void oric_state::oric_vh_update_flash()
{
	/* flash active? */
	if (BIT(m_vh_state.text_attributes, 2))
	{
		/* yes */

		/* show or hide text? */
		if (BIT(m_vh_state.flash_count, 4))
		{
			/* hide */
			/* set foreground and background to be the same */
			m_vh_state.active_foreground_colour = m_vh_state.background_colour;
			m_vh_state.active_background_colour = m_vh_state.background_colour;
			return;
		}
	}


	/* show */
	m_vh_state.active_foreground_colour = m_vh_state.foreground_colour;
	m_vh_state.active_background_colour = m_vh_state.background_colour;
}

/* the alternate charset follows from the standard charset.
Each charset holds 128 chars with 8 bytes for each char.

The start address for the standard charset is dependant on the video mode */
void oric_state::oric_refresh_charset()
{
	/* alternate char set? */
	if (BIT(m_vh_state.text_attributes, 0))
	{
		/* yes */
		m_vh_state.char_data = m_vh_state.char_base + (128*8);
	}
	else
	{
		/* no */
		m_vh_state.char_data = m_vh_state.char_base;
	}
}

/* update video hardware state depending on the new attribute */
void oric_state::oric_vh_update_attribute(UINT8 c)
{
	/* attribute */
	UINT8 attribute = c & 0x03f;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	switch ((attribute>>3) & 0x03)
	{
		case 0:
		{
			/* set foreground colour 00-07 = black,red,green,yellow,blue,magenta,cyan,white */
			m_vh_state.foreground_colour = attribute & 0x07;
			oric_vh_update_flash();
		}
		break;

		case 1:
		{
			m_vh_state.text_attributes = attribute & 0x07;

			oric_refresh_charset();

			/* text attributes */
			oric_vh_update_flash();
		}
		break;

		case 2:
		{
			/* set background colour */
			m_vh_state.background_colour = attribute & 0x07;
			oric_vh_update_flash();
		}
		break;

		case 3:
		{
			/* set video mode */
			m_vh_state.mode = attribute & 0x07;

			// a different charset base is used depending on the video mode
			// hires takes all the data from 0x0a000 through to about 0x0bf68,
			// so the charset is moved to 0x09800 */
			// text mode starts at 0x0bb80 and so the charset is in a different location
			if (BIT(m_vh_state.mode, 2))
			{
				/* set screen memory base and standard charset location for this mode */
				m_vh_state.read_addr = 0x0a000;
				if (m_ram)
					m_vh_state.char_base = m_ram + (offs_t)0x09800;
				else
					m_vh_state.char_base = (UINT8 *)space.get_read_ptr(0x09800);
			}
			else
			{
				/* set screen memory base and standard charset location for this mode */
				m_vh_state.read_addr = 0x0bb80;
				if (m_ram)
					m_vh_state.char_base = m_ram + (offs_t)0x0b400;
				else
					m_vh_state.char_base = (UINT8 *)space.get_read_ptr(0x0b400);
			}
			/* changing the mode also changes the position of the standard charset and alternative charset */
			oric_refresh_charset();
		}
		break;

		default:
			break;
	}
}


/* render 6-pixels using foreground and background colours specified */
/* used in hires and text mode */
void oric_state::oric_vh_render_6pixels(bitmap_ind16 &bitmap, int x, UINT8 y, UINT8 fg, UINT8 bg, UINT8 data, bool invert_flag)
{
	/* invert? */
	if (invert_flag)
	{
		fg ^=0x07;
		bg ^=0x07;
	}

	bitmap.pix16(y, x++) = BIT(data, 5) ? fg : bg;
	bitmap.pix16(y, x++) = BIT(data, 4) ? fg : bg;
	bitmap.pix16(y, x++) = BIT(data, 3) ? fg : bg;
	bitmap.pix16(y, x++) = BIT(data, 2) ? fg : bg;
	bitmap.pix16(y, x++) = BIT(data, 1) ? fg : bg;
	bitmap.pix16(y, x++) = BIT(data, 0) ? fg : bg;
}





/***************************************************************************
  oric_vh_screenrefresh
***************************************************************************/
UINT32 oric_state::screen_update_oric(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *RAM, y;
	offs_t byte_offset, read_addr_base;
	bool hires_active;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	RAM = m_ram;

	/* set initial base */
	read_addr_base = m_vh_state.read_addr;

	/* is hires active? */
	hires_active = BIT(m_vh_state.mode, 2);

	for (y = 0; y < 224; y++)
	{
		int x = 0;

		/* foreground colour white */
		oric_vh_update_attribute(7);
		/* background colour black */
		oric_vh_update_attribute((1<<3));
		oric_vh_update_attribute((1<<4));

		for (byte_offset=0; byte_offset<40; byte_offset++)
		{
			UINT8 c;
			offs_t read_addr;

			/* after line 200 all rendering is done in text mode */
			if (y<200)
			{
				/* calculate fetch address based on current line and current mode */
				if (hires_active)
				{
					read_addr = read_addr_base + byte_offset + (offs_t)(y*40);
				}
				else
				{
					UINT8 char_line = y>>3;
					read_addr = read_addr_base + byte_offset + (offs_t)(char_line*40);
				}
			}
			else
			{
				UINT8 char_line = (y-200)>>3;
				read_addr = read_addr_base + byte_offset + (offs_t)(char_line*40);
			}

			/* fetch data */
			c = RAM ? RAM[read_addr] : space.read_byte(read_addr);

			/* if bits 6 and 5 are zero, the byte contains a serial attribute */
			if ((c & ((1 << 6) | (1 << 5))) == 0)
			{
				oric_vh_update_attribute(c);

				/* display background colour when attribute has been found */
				oric_vh_render_6pixels(bitmap, x, y, m_vh_state.active_foreground_colour, m_vh_state.active_background_colour, 0, (c & 0x080));

				if (y < 200)
				{
					/* is hires active? */
					hires_active = BIT(m_vh_state.mode, 2);
					read_addr_base = m_vh_state.read_addr;
				}
			}
			else
			{
				/* hires? */
				if (hires_active)
				{
					UINT8 pixel_data = c & 0x03f;
					/* plot hires pixels */
					oric_vh_render_6pixels(bitmap,x,y,m_vh_state.active_foreground_colour, m_vh_state.active_background_colour, pixel_data, BIT(c, 7));
				}
				else
				{
					UINT8 char_index, char_data, ch_line;

					char_index = (c & 0x07f);

					ch_line = y & 7;

					/* is double height set? */
					if (BIT(m_vh_state.text_attributes, 1))
					{
					/* if char line is even, top half of character is displayed else bottom half */
						UINT8 double_height_flag = BIT(y, 3);

						/* calculate line to fetch */
						ch_line = (ch_line>>1) + (double_height_flag<<2);
					}

					/* fetch pixel data for this char line */
					char_data = m_vh_state.char_data[(char_index<<3) | ch_line] & 0x03f;

					/* draw! */
					oric_vh_render_6pixels(bitmap,x,y,
						m_vh_state.active_foreground_colour,
						m_vh_state.active_background_colour, char_data, BIT(c, 7));
				}

			}

			x+=6;
		}

		/* after 200 lines have been drawn, force a change of the read address */
		/* there are 200 lines of hires/text mode, then 24 lines of text mode */
		/* the mode can't be changed in the last 24 lines. */
		if (y==199)
		{
			/* mode */
			read_addr_base = (offs_t)0x0bf68;
			hires_active = 0;
		}
	}
	return 0;
}


void oric_state::video_start()
{
	// initialise variables
	m_vh_state.active_foreground_colour = 0;
	m_vh_state.active_background_colour = 0;
	m_vh_state.foreground_colour = 0;
	m_vh_state.background_colour = 0;
	m_vh_state.mode = 0;
	m_vh_state.text_attributes = 0;
	m_vh_state.read_addr = 0;
	m_vh_state.char_data = 0;
	m_vh_state.char_base = 0;
	/* initialise flash timer */
	m_vh_state.flash_count = 0;
	machine().scheduler().timer_pulse(attotime::from_hz(50), timer_expired_delegate(FUNC(oric_state::oric_vh_timer_callback),this));
	/* mode */
	oric_vh_update_attribute((1<<3)|(1<<4));
}
