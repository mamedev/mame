/* Super80.c written by Robbbert, 2005-2010. See the MESS wiki for documentation. */

/* Notes on using MAME MC6845 Device (MMD).
    1. Speed of MMD is about 20% slower than pre-MMD coding
    2. Undocumented cursor start and end-lines is not supported by MMD, so we do it here. */


#include "emu.h"
#include "includes/super80.h"



/**************************** PALETTES for super80m and super80v ******************************************/

static const UINT8 super80_rgb_palette[16*3] =
{
	0x00, 0x00, 0x00,	/*  0 Black     */
	0x00, 0x00, 0x00,	/*  1 Black     */
	0x00, 0x00, 0x7f,	/*  2 Blue      */
	0x00, 0x00, 0xff,	/*  3 Light Blue    */
	0x00, 0x7f, 0x00,	/*  4 Green     */
	0x00, 0xff, 0x00,	/*  5 Bright Green  */
	0x00, 0x7f, 0x7f,	/*  6 Cyan      */
	0x00, 0xff, 0xff,	/*  7 Turquoise     */
	0x7f, 0x00, 0x00,	/*  8 Dark Red      */
	0xff, 0x00, 0x00,	/*  9 Red       */
	0x7f, 0x00, 0x7f,	/* 10 Purple        */
	0xff, 0x00, 0xff,	/* 11 Magenta       */
	0x7f, 0x7f, 0x00,	/* 12 Lime      */
	0xff, 0xff, 0x00,	/* 13 Yellow        */
	0xbf, 0xbf, 0xbf,	/* 14 Off White     */
	0xff, 0xff, 0xff,	/* 15 White     */
};

static const UINT8 super80_comp_palette[16*3] =
{
	0x00, 0x00, 0x00,	/*  0 Black     */
	0x80, 0x80, 0x80,	/*  1 Grey      */
	0x00, 0x00, 0xff,	/*  2 Blue      */
	0xff, 0xff, 0x80,	/*  3 Light Yellow  */
	0x00, 0xff, 0x00,	/*  4 Green     */
	0xff, 0x80, 0xff,	/*  5 Light Magenta */
	0x00, 0xff, 0xff,	/*  6 Cyan      */
	0xff, 0x40, 0x40,	/*  7 Light Red     */
	0xff, 0x00, 0x00,	/*  8 Red       */
	0x00, 0x80, 0x80,	/*  9 Dark Cyan     */
	0xff, 0x00, 0xff,	/* 10 Magenta       */
	0x80, 0xff, 0x80,	/* 11 Light Green   */
	0xff, 0xff, 0x00,	/* 12 Yellow        */
	0x00, 0x00, 0x80,	/* 13 Dark Blue     */
	0xff, 0xff, 0xff,	/* 14 White     */
	0x00, 0x00, 0x00,	/* 15 Black     */
};

static void palette_set_colors_rgb(running_machine &machine, const UINT8 *colors)
{
	UINT8 r, b, g, color_count = 16;

	while (color_count--)
	{
		r = *colors++; g = *colors++; b = *colors++;
		palette_set_color(machine, 15-color_count, MAKE_RGB(r, g, b));
	}
}

PALETTE_INIT_MEMBER(super80_state,super80m)
{
	palette_set_colors_rgb(machine(), super80_rgb_palette);
}



SCREEN_VBLANK( super80m )
{
	// rising edge
	if (vblank_on)
	{
		super80_state *state = screen.machine().driver_data<super80_state>();
		/* if we chose another palette or colour mode, enable it */
		UINT8 chosen_palette = (screen.machine().root_device().ioport("CONFIG")->read() & 0x60)>>5;				// read colour dipswitches

		if (chosen_palette != state->m_current_palette)						// any changes?
		{
			state->m_current_palette = chosen_palette;					// save new palette
			if (!state->m_current_palette)
				palette_set_colors_rgb(screen.machine(), super80_comp_palette);		// composite colour
			else
				palette_set_colors_rgb(screen.machine(), super80_rgb_palette);		// rgb and b&w
		}
	}
}

SCREEN_UPDATE_IND16( super80 )
{
	super80_state *state = screen.machine().driver_data<super80_state>();
	UINT8 y,ra,chr=32,gfx,screen_on=0;
	UINT16 sy=0,ma=state->m_vidpg,x;
	UINT8 *RAM = state->memregion("maincpu")->base();

	output_set_value("cass_led",(state->m_shared & 0x20) ? 1 : 0);

	if ((state->m_shared & 4) || (!(screen.machine().root_device().ioport("CONFIG")->read() & 4)))	/* bit 2 of port F0 is high, OR user turned on config switch */
		screen_on++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 32; x++)	// done this way to avoid x overflowing on page FF
			{
				if (screen_on)
					chr = RAM[ma | x] & 0x3f;

				/* get pattern of pixels for that character scanline */
				gfx = state->m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

SCREEN_UPDATE_IND16( super80d )
{
	super80_state *state = screen.machine().driver_data<super80_state>();
	UINT8 y,ra,chr=32,gfx,screen_on=0;
	UINT16 sy=0,ma=state->m_vidpg,x;
	UINT8 *RAM = state->memregion("maincpu")->base();

	output_set_value("cass_led",(state->m_shared & 0x20) ? 1 : 0);

	if ((state->m_shared & 4) || (!(screen.machine().root_device().ioport("CONFIG")->read() & 4)))	/* bit 2 of port F0 is high, OR user turned on config switch */
		screen_on++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 32; x++)
			{
				if (screen_on)
					chr = RAM[ma | x];

				/* get pattern of pixels for that character scanline */
				gfx = state->m_p_chargen[((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

SCREEN_UPDATE_IND16( super80e )
{
	super80_state *state = screen.machine().driver_data<super80_state>();
	UINT8 y,ra,chr=32,gfx,screen_on=0;
	UINT16 sy=0,ma=state->m_vidpg,x;
	UINT8 *RAM = state->memregion("maincpu")->base();

	output_set_value("cass_led",(state->m_shared & 0x20) ? 1 : 0);

	if ((state->m_shared & 4) || (!(screen.machine().root_device().ioport("CONFIG")->read() & 4)))	/* bit 2 of port F0 is high, OR user turned on config switch */
		screen_on++;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 32; x++)
			{
				if (screen_on)
					chr = RAM[ma | x];

				/* get pattern of pixels for that character scanline */
				gfx = state->m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

SCREEN_UPDATE_IND16( super80m )
{
	super80_state *state = screen.machine().driver_data<super80_state>();
	UINT8 y,ra,chr=32,gfx,screen_on=0;
	UINT16 sy=0,ma=state->m_vidpg,x;
	UINT8 col, bg=0, fg=0, options=screen.machine().root_device().ioport("CONFIG")->read();
	UINT8 *RAM = state->memregion("maincpu")->base();

	/* get selected character generator */
	UINT8 cgen = state->m_current_charset ^ ((options & 0x10)>>4);	/* bit 0 of port F1 and cgen config switch */

	output_set_value("cass_led",(state->m_shared & 0x20) ? 1 : 0);

	if ((state->m_shared & 4) || (!(options & 4)))	/* bit 2 of port F0 is high, OR user turned on config switch */
		screen_on++;

	if (screen_on)
	{
		if ((options & 0x60) == 0x60)
			fg = 15;	/* b&w */
		else
			fg = 5;		/* green */
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 32; x++)
			{
				if (screen_on)
					chr = RAM[ma | x];

				if (!(options & 0x40))
				{
					col = RAM[0xfe00 | ma | x];	/* byte of colour to display */
					fg = col & 0x0f;
					bg = (col & 0xf0) >> 4;
				}

				/* get pattern of pixels for that character scanline */
				if (cgen)
					gfx = state->m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];
				else
					gfx = state->m_p_chargen[0x1000 | ((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
			}
		}
		ma+=32;
	}
	return 0;
}

VIDEO_START_MEMBER(super80_state,super80)
{
	m_vidpg = 0xfe00;
	m_p_chargen = memregion("chargen")->base();
}

/**************************** I/O PORTS *****************************************************************/

WRITE8_MEMBER( super80_state::super80_f1_w )
{
	m_vidpg = (data & 0xfe) << 8;
	m_current_charset = data & 1;
}

/*---------------------------------------------------------------

    Super-80R and Super-80V

---------------------------------------------------------------*/

static const UINT8 mc6845_mask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0};

READ8_MEMBER( super80_state::super80v_low_r )
{
	if (m_shared & 4)
		return m_p_videoram[offset];
	else
		return m_p_colorram[offset];
}

WRITE8_MEMBER( super80_state::super80v_low_w )
{
	if (m_shared & 4)
		m_p_videoram[offset] = data;
	else
		m_p_colorram[offset] = data;
}

READ8_MEMBER( super80_state::super80v_high_r )
{
	if (~m_shared & 4)
		return m_p_colorram[0x800 | offset];
	else
	if (m_shared & 0x10)
		return m_p_pcgram[0x800 | offset];
	else
		return m_p_pcgram[offset];
}

WRITE8_MEMBER( super80_state::super80v_high_w )
{
	if (~m_shared & 4)
		m_p_colorram[0x800 | offset] = data;
	else
	{
		m_p_videoram[0x800 | offset] = data;

		if (m_shared & 0x10)
			m_p_pcgram[0x800 | offset] = data;
	}
}

/* The 6845 can produce a variety of cursor shapes - all are emulated here - remove when mame fixed */
void super80_state::mc6845_cursor_configure()
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

VIDEO_START_MEMBER(super80_state,super80v)
{
	m_p_pcgram = machine().root_device().memregion("maincpu")->base()+0xf000;
	m_p_videoram = machine().root_device().memregion("videoram")->base();
	m_p_colorram = memregion("colorram")->base();
}

SCREEN_UPDATE_RGB32( super80v )
{
	super80_state *state = screen.machine().driver_data<super80_state>();
	state->m_framecnt++;
	state->m_speed = state->m_mc6845_reg[10]&0x20, state->m_flash = state->m_mc6845_reg[10]&0x40; // cursor modes
	state->m_cursor = (state->m_mc6845_reg[14]<<8) | state->m_mc6845_reg[15]; // get cursor position
	state->m_s_options=screen.machine().root_device().ioport("CONFIG")->read();
	output_set_value("cass_led",(state->m_shared & 0x20) ? 1 : 0);
	state->m_6845->screen_update(screen, bitmap, cliprect);
	return 0;
}

MC6845_UPDATE_ROW( super80v_update_row )
{
	super80_state *state = device->machine().driver_data<super80_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,col,gfx,fg,bg=0;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)				// for each character
	{
		UINT8 inv=0;
		//      if (x == cursor_x) inv=0xff;    /* uncomment when mame fixed */
		mem = (ma + x) & 0xfff;
		chr = state->m_p_videoram[mem];

		/* get colour or b&w */
		fg = 5;						/* green */
		if ((state->m_s_options & 0x60) == 0x60) fg = 15;		/* b&w */

		if (~state->m_s_options & 0x40)
		{
			col = state->m_p_colorram[mem];					/* byte of colour to display */
			fg = col & 0x0f;
			bg = (col & 0xf0) >> 4;
		}

		/* if inverse mode, replace any pcgram chrs with inverse chrs */
		if ((~state->m_shared & 0x10) && (chr & 0x80))			// is it a high chr in inverse mode
		{
			inv ^= 0xff;						// invert the chr
			chr &= 0x7f;						// and drop bit 7
		}

		/* process cursor - remove when mame fixed */
		if ((((!state->m_flash) && (!state->m_speed)) ||
			((state->m_flash) && (state->m_speed) && (state->m_framecnt & 0x10)) ||
			((state->m_flash) && (!state->m_speed) && (state->m_framecnt & 8))) &&
			(mem == state->m_cursor))
				inv ^= state->m_mc6845_cursor[ra];

		/* get pattern of pixels for that character scanline */
		gfx = state->m_p_pcgram[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
	}
}

/**************************** I/O PORTS *****************************************************************/

WRITE8_MEMBER( super80_state::super80v_10_w )
{
	data &= 0x1f;
	m_mc6845_ind = data;
	m_6845->address_w( space, 0, data );
}

WRITE8_MEMBER( super80_state::super80v_11_w )
{
	m_mc6845_reg[m_mc6845_ind] = data & mc6845_mask[m_mc6845_ind];	/* save data in register */
	m_6845->register_w( space, 0, data );
	if ((m_mc6845_ind > 8) && (m_mc6845_ind < 12)) mc6845_cursor_configure();		/* adjust cursor shape - remove when mame fixed */
}
