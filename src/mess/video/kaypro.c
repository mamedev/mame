
#include "includes/kaypro.h"



/***********************************************************

    Video

************************************************************/

PALETTE_INIT( kaypro )
{
	palette_set_color(machine, 0, RGB_BLACK); /* black */
	palette_set_color(machine, 1, MAKE_RGB(0, 220, 0)); /* green */
	palette_set_color(machine, 2, MAKE_RGB(0, 110, 0)); /* low intensity green */
}

SCREEN_UPDATE_IND16( kayproii )
{
	kaypro_state *state = screen.machine().driver_data<kaypro_state>();
/* The display consists of 80 columns and 24 rows. Each row is allocated 128 bytes of ram,
    but only the first 80 are used. The total video ram therefore is 0x0c00 bytes.
    There is one video attribute: bit 7 causes blinking. The first half of the
    character generator is blank, with the visible characters in the 2nd half.
    During the "off" period of blanking, the first half is used. Only 5 pixels are
    connected from the rom to the shift register, the remaining pixels are held high. */

	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	state->m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 8)
				{
					chr = state->m_p_videoram[x]^0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (state->m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = state->m_p_chargen[(chr<<3) | ra ];
				}

				/* Display a scanline of a character (7 pixels) */
				*p++ = 0;
				*p++ = BIT( gfx, 4 );
				*p++ = BIT( gfx, 3 );
				*p++ = BIT( gfx, 2 );
				*p++ = BIT( gfx, 1 );
				*p++ = BIT( gfx, 0 );
				*p++ = 0;
			}
		}
		ma+=128;
	}
	return 0;
}

SCREEN_UPDATE_IND16( omni2 )
{
	kaypro_state *state = screen.machine().driver_data<kaypro_state>();
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	state->m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 8)
				{
					chr = state->m_p_videoram[x];

					/* Take care of flashing characters */
					if ((chr > 0x7f) && (state->m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = state->m_p_chargen[(chr<<3) | ra ];
				}

				/* Display a scanline of a character (7 pixels) */
				*p++ = BIT( gfx, 6 );
				*p++ = BIT( gfx, 5 );
				*p++ = BIT( gfx, 4 );
				*p++ = BIT( gfx, 3 );
				*p++ = BIT( gfx, 2 );
				*p++ = BIT( gfx, 1 );
				*p++ = BIT( gfx, 0 );
			}
		}
		ma+=128;
	}
	return 0;
}

SCREEN_UPDATE_RGB32( kaypro2x )
{
	kaypro_state *state = screen.machine().driver_data<kaypro_state>();
	state->m_framecnt++;
	state->m_speed = state->m_mc6845_reg[10]&0x20;
	state->m_flash = state->m_mc6845_reg[10]&0x40;				// cursor modes
	state->m_cursor = (state->m_mc6845_reg[14]<<8) | state->m_mc6845_reg[15];					// get cursor position
	state->m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

/* bit 6 of kaypro2x_system_port selects alternate characters (A12 on character generator rom).
    The diagram specifies a 2732 with 28 pins, and more address pins. Possibly a 2764 or 27128.
    Since our dump only goes up to A11, the alternate character set doesn't exist.

    0000-07FF of videoram is memory-mapped characters; 0800-0FFF is equivalent attribute bytes.
    d3 Underline
    d2 blinking (at unknown rate)
    d1 low intensity
    d0 reverse video

    Not sure how the attributes interact, for example does an underline blink? */


MC6845_UPDATE_ROW( kaypro2x_update_row )
{
	kaypro_state *state = device->machine().driver_data<kaypro_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32 *p = &bitmap.pix32(y);
	UINT16 x;
	UINT8 gfx,fg,bg;

	for (x = 0; x < x_count; x++)				// for each character
	{
		UINT8 inv=0;
		//      if (x == cursor_x) inv=0xff;    /* uncomment when mame fixed */
		UINT16 mem = (ma + x) & 0x7ff;
		UINT8 chr = state->m_p_videoram[mem];
		UINT8 attr = state->m_p_videoram[mem | 0x800];

		if ((attr & 3) == 3)
		{
			fg = 0;
			bg = 2;
		}
		else
		if ((attr & 3) == 2)
		{
			fg = 2;
			bg = 0;
		}
		else
		if ((attr & 3) == 1)
		{
			fg = 0;
			bg = 1;
		}
		else
		{
			fg = 1;
			bg = 0;
		}

		/* Take care of flashing characters */
		if ( (BIT(attr, 2)) & (BIT(state->m_framecnt, 3)) )
			fg = bg;

		/* process cursor - remove when mame fixed */
		if ((((!state->m_flash) && (!state->m_speed)) ||
			((state->m_flash) && (state->m_speed) && (state->m_framecnt & 0x10)) ||
			((state->m_flash) && (!state->m_speed) && (state->m_framecnt & 8))) &&
			(mem == state->m_cursor))
				inv ^= state->m_mc6845_cursor[ra];

		/* get pattern of pixels for that character scanline */
		if ( (ra == 15) & (BIT(attr, 3)) )	/* underline */
			gfx = 0xff;
		else
			gfx = state->m_p_chargen[(chr<<4) | ra ] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT( gfx, 7 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 6 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 5 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 4 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 3 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 2 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 1 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 0 ) ? fg : bg];
	}
}

/************************************* MC6845 SUPPORT ROUTINES ***************************************/

/* The 6845 can produce a variety of cursor shapes - all are emulated here - remove when mame fixed */
void kaypro_state::mc6845_cursor_configure()
{
	UINT8 i,curs_type=0,r9,r10,r11;

	/* curs_type holds the general cursor shape to be created
        0 = no cursor
        1 = partial cursor (only shows on a block of scan lines)
        2 = full cursor
        3 = two-part cursor (has a part at the top and bottom with the middle blank) */

	for ( i = 0; i < ARRAY_LENGTH(m_mc6845_cursor); i++) m_mc6845_cursor[i] = 0;		// prepare cursor by erasing old one

	r9  = m_mc6845_reg[9];					// number of scan lines - 1
	r10 = m_mc6845_reg[10] & 0x1f;				// cursor start line = last 5 bits
	r11 = m_mc6845_reg[11]+1;					// cursor end line incremented to suit for-loops below

	/* decide the curs_type by examining the registers */
	if (r10 < r11) curs_type=1;				// start less than end, show start to end
	else
	if (r10 == r11) curs_type=2;				// if equal, show full cursor
	else curs_type=3;					// if start greater than end, it's a two-part cursor

	if ((r11 - 1) > r9) curs_type=2;			// if end greater than scan-lines, show full cursor
	if (r10 > r9) curs_type=0;				// if start greater than scan-lines, then no cursor
	if (r11 > 16) r11=16;					// truncate 5-bit register to fit our 4-bit hardware

	/* create the new cursor */
	if (curs_type > 1) for (i = 0;i < ARRAY_LENGTH(m_mc6845_cursor);i++) m_mc6845_cursor[i]=0xff; // turn on full cursor

	if (curs_type == 1) for (i = r10;i < r11;i++) m_mc6845_cursor[i]=0xff; // for each line that should show, turn on that scan line

	if (curs_type == 3) for (i = r11; i < r10;i++) m_mc6845_cursor[i]=0; // now take a bite out of the middle
}

/* Resize the screen within the limits of the hardware. Expand the image to fill the screen area.
    Standard screen is 640 x 400 = 0x7d0 bytes. */

void kaypro_state::mc6845_screen_configure()
{
	UINT16 width = m_mc6845_reg[1]*8-1;							// width in pixels
	UINT16 height = m_mc6845_reg[6]*(m_mc6845_reg[9]+1)-1;					// height in pixels
	UINT16 bytes = m_mc6845_reg[1]*m_mc6845_reg[6]-1;						// video ram needed -1

	/* Resize the screen */
	if ((width < 640) && (height < 400) && (bytes < 0x800))	/* bounds checking to prevent an assert or violation */
		machine().primary_screen->set_visible_area(0, width, 0, height);
}


/**************************** I/O PORTS *****************************************************************/

READ8_MEMBER( kaypro_state::kaypro2x_status_r )
{
/* Need bit 7 high or computer hangs */

	return 0x80 | m_crtc->register_r(space, 0);
}

WRITE8_MEMBER( kaypro_state::kaypro2x_index_w )
{
	m_mc6845_ind = data & 0x1f;
	m_crtc->address_w( space, 0, data );
}

WRITE8_MEMBER( kaypro_state::kaypro2x_register_w )
{
	static const UINT8 mcmask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0};

	if (m_mc6845_ind < 16)
		m_mc6845_reg[m_mc6845_ind] = data & mcmask[m_mc6845_ind];	/* save data in register */
	else
		m_mc6845_reg[m_mc6845_ind] = data;

	m_crtc->register_w( space, 0, data );

	if ((m_mc6845_ind == 1) || (m_mc6845_ind == 6) || (m_mc6845_ind == 9))
		mc6845_screen_configure();			/* adjust screen size */

	if ((m_mc6845_ind > 8) && (m_mc6845_ind < 12))
		mc6845_cursor_configure();		/* adjust cursor shape - remove when mame fixed */

	if ((m_mc6845_ind > 17) && (m_mc6845_ind < 20))
		m_mc6845_video_address = m_mc6845_reg[19] | ((m_mc6845_reg[18] & 0x3f) << 8);	/* internal ULA address */
}

READ8_MEMBER( kaypro_state::kaypro_videoram_r )
{
	return m_p_videoram[offset];
}

WRITE8_MEMBER( kaypro_state::kaypro_videoram_w )
{
	m_p_videoram[offset] = data;
}

READ8_MEMBER( kaypro_state::kaypro2x_videoram_r )
{
	return m_p_videoram[m_mc6845_video_address];
}

WRITE8_MEMBER( kaypro_state::kaypro2x_videoram_w )
{
	m_p_videoram[m_mc6845_video_address] = data;
}

VIDEO_START( kaypro )
{
	kaypro_state *state = machine.driver_data<kaypro_state>();
	state->m_p_chargen = state->memregion("chargen")->base();
}
