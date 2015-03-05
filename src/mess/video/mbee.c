/***************************************************************************
    microbee.c

    video hardware
    Juergen Buchmueller <pullmoll@t-online.de>, Dec 1999

    Rewritten by Robbbert

****************************************************************************/


#include "includes/mbee.h"

WRITE_LINE_MEMBER( mbee_state::crtc_vs )
{
	m_b7_vs = state;
	if ((m_io_config->read() & 0xc0) == 0) // VS selected in config menu
		m_pio->port_b_write(pio_port_b_r(generic_space(),0,0xff));
}

/***********************************************************

    The 6845 can produce a variety of cursor shapes - all
    are emulated here.

    Need to find out if the 6545 works the same way.

************************************************************/

void mbee_state::sy6545_cursor_configure()
{
	UINT8 i,curs_type=0,r9,r10,r11;

	/* curs_type holds the general cursor shape to be created
	    0 = no cursor
	    1 = partial cursor (only shows on a block of scan lines)
	    2 = full cursor
	    3 = two-part cursor (has a part at the top and bottom with the middle blank) */

	for ( i = 0; i < ARRAY_LENGTH(m_sy6545_cursor); i++) m_sy6545_cursor[i] = 0;        // prepare cursor by erasing old one

	r9  = m_sy6545_reg[9];                  // number of scan lines - 1
	r10 = m_sy6545_reg[10] & 0x1f;              // cursor start line = last 5 bits
	r11 = m_sy6545_reg[11]+1;               // cursor end line incremented to suit for-loops below

	/* decide the curs_type by examining the registers */
	if (r10 < r11) curs_type=1;             // start less than end, show start to end
	else
	if (r10 == r11) curs_type=2;                // if equal, show full cursor
	else curs_type=3;                   // if start greater than end, it's a two-part cursor

	if ((r11 - 1) > r9) curs_type=2;            // if end greater than scan-lines, show full cursor
	if (r10 > r9) curs_type=0;              // if start greater than scan-lines, then no cursor
	if (r11 > 16) r11=16;                   // truncate 5-bit register to fit our 4-bit hardware

	/* create the new cursor */
	if (curs_type > 1) for (i = 0;i < ARRAY_LENGTH(m_sy6545_cursor);i++) m_sy6545_cursor[i]=0xff; // turn on full cursor

	if (curs_type == 1) for (i = r10;i < r11;i++) m_sy6545_cursor[i]=0xff; // for each line that should show, turn on that scan line

	if (curs_type == 3) for (i = r11; i < r10;i++) m_sy6545_cursor[i]=0; // now take a bite out of the middle
}


/***********************************************************

    Handlers of video, colour, and attribute RAM

************************************************************/


READ8_MEMBER( mbee_state::video_low_r )
{
	if (m_is_premium && ((m_1c & 0x9f) == 0x90))
		return m_p_attribram[offset];
	else
	if (m_0b)
		return m_p_gfxram[offset];
	else
		return m_p_videoram[offset];
}

WRITE8_MEMBER( mbee_state::video_low_w )
{
	if BIT(m_1c, 4)
	{
		// non-premium attribute writes are discarded
		if (m_is_premium && BIT(m_1c, 7))
			m_p_attribram[offset] = data;
	}
	else
		m_p_videoram[offset] = data;
}

READ8_MEMBER( mbee_state::video_high_r )
{
	if BIT(m_08, 6)
		return m_p_colorram[offset];
	else
		return m_p_gfxram[(((m_1c & 15) + 1) << 11) | offset];
}

WRITE8_MEMBER( mbee_state::video_high_w )
{
	if (BIT(m_08, 6) && (~m_0b & 1))
		m_p_colorram[offset] = data;
	else
		m_p_gfxram[(((m_1c & 15) + 1) << 11) | offset] = data;
}

WRITE8_MEMBER( mbee_state::port0b_w )
{
	m_0b = data & 1;
}

READ8_MEMBER( mbee_state::port08_r )
{
	return m_08;
}

WRITE8_MEMBER( mbee_state::port08_w )
{
	m_08 = data & 0x4e;
}

READ8_MEMBER( mbee_state::port1c_r )
{
	return m_1c;
}

WRITE8_MEMBER( mbee_state::port1c_w )
{
/*  d7 extended graphics (1=allow attributes and pcg banks)
    d5 bankswitch basic rom
    d4 select attribute ram
    d3..d0 select m_videoram bank */

	if (m_is_premium && BIT(data, 7))
		m_1c = data;
	else
		m_1c = data & 0x30;

	if (m_basic)
		m_basic->set_entry(BIT(data, 5));
}


/***********************************************************

    CRTC-driven keyboard

************************************************************/


void mbee_state::oldkb_matrix_r(UINT16 offs)
{
	if (m_has_oldkb)
	{
		UINT8 port = (offs >> 7) & 7;
		UINT8 bit = (offs >> 4) & 7;
		UINT8 data = m_io_oldkb[port]->read();
		bool keydown  = ( data >> bit ) & 1;

		// This adds premium-style cursor keys to the old keyboard
		// They are used by the pc85 & ppc menu, and the 128k shell.
		if (!keydown)
		{
			UINT8 extra = m_io_extra->read();

			if (extra && port == 7 && bit == 1) keydown = 1;   /* Control */

			if (BIT(extra, 0) && ( port == 0 && bit == 5 )) keydown = 1; // cursor up = ^E
			else
			if (BIT(extra, 1) && ( port == 3 && bit == 0 )) keydown = 1; // cursor down = ^X
			else
			if (BIT(extra, 2) && ( port == 2 && bit == 3 )) keydown = 1; // cursor left = ^S
			else
			if (BIT(extra, 3) && ( port == 0 && bit == 4 )) keydown = 1; // cursor right = ^D
			else
			if (BIT(extra, 4) && ( port == 2 && bit == 6 )) keydown = 1; // insert = ^V
		}

		if( keydown )
		{
			m_sy6545_reg[17] = offs;
			m_sy6545_reg[16] = (offs >> 8) & 0x3f;
			m_sy6545_status |= 0x40; //lpen_strobe
		}
	}
}


void mbee_state::oldkb_scan( UINT16 param )
{
	if (m_0b) return; // IC5 (pins 11,12,13)
	if (param & 15) return; // only scan once per row instead of 16 times
	oldkb_matrix_r(param);
}


/***********************************************************

    CRTC registers

************************************************************/

READ8_MEMBER( mbee_state::m6545_status_r )
{
	const rectangle &visarea = m_screen->visible_area();

	UINT8 data = m_sy6545_status; // bit 6 = lpen strobe, bit 7 = update strobe
	int y = m_screen->vpos();

	if( y < visarea.min_y || y > visarea.max_y )
		data |= 0x20;   /* vertical blanking */

	return data;
}

READ8_MEMBER( mbee_state::m6545_data_r )
{
	UINT16 addr;
	UINT8 data = m_crtc->register_r( space, 0 );

	switch( m_sy6545_ind )
	{
	case 16:
	case 17:
		m_sy6545_status &= 0x80; // turn off lpen_strobe
		break;
	case 31:
		// This firstly pushes the contents of the transparent registers onto the MA lines,
		// then increments the address, then sets update strobe on.
		addr = (m_sy6545_reg[18] << 8) | m_sy6545_reg[19];
		oldkb_matrix_r(addr);
		m_sy6545_reg[19]++;
		if (!m_sy6545_reg[19]) m_sy6545_reg[18]++;
		m_sy6545_status |= 0x80; // update_strobe
		break;
	}
	return data;
}

WRITE8_MEMBER ( mbee_state::m6545_index_w )
{
	data &= 0x1f;
	m_sy6545_ind = data;
	m_crtc->address_w( space, 0, data );
}

WRITE8_MEMBER ( mbee_state::m6545_data_w )
{
	static const UINT8 sy6545_mask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0,0x3f,0xff};
	UINT16 addr = 0;

	switch( m_sy6545_ind )
	{
	case 12:
		data &= 0x3f; // select alternate character set
		if( m_sy6545_reg[12] != data )
			memcpy(m_p_gfxram, memregion("gfx")->base() + (((data & 0x30) == 0x20) << 11), 0x800);
		break;
	case 31:
		// This firstly pushes the contents of the transparent registers onto the MA lines,
		// then increments the address, then sets update strobe on.
		addr = (m_sy6545_reg[18] << 8) | m_sy6545_reg[19];
		oldkb_matrix_r(addr);
		m_sy6545_reg[19]++;
		if (!m_sy6545_reg[19]) m_sy6545_reg[18]++;
		m_sy6545_status |= 0x80; // update_strobe
		break;
	}
	m_sy6545_reg[m_sy6545_ind] = data & sy6545_mask[m_sy6545_ind];  /* save data in register */
	m_crtc->register_w( space, 0, data );
	if ((m_sy6545_ind > 8) && (m_sy6545_ind < 12)) sy6545_cursor_configure();       /* adjust cursor shape - remove when mame fixed */
}




/***********************************************************

    Video

************************************************************/

VIDEO_START_MEMBER( mbee_state, mono )
{
	m_p_videoram = memregion("videoram")->base();
	m_p_gfxram = memregion("gfx")->base()+0x1000;
	m_is_premium = 0;
}

VIDEO_START_MEMBER( mbee_state, standard )
{
	m_p_videoram = memregion("videoram")->base();
	m_p_colorram = memregion("colorram")->base();
	m_p_gfxram = memregion("gfx")->base()+0x1000;
	m_is_premium = 0;
}

VIDEO_START_MEMBER( mbee_state, premium )
{
	m_p_videoram = memregion("videoram")->base();
	m_p_colorram = memregion("colorram")->base();
	m_p_gfxram = memregion("gfx")->base()+0x1000;
	m_p_attribram = memregion("attrib")->base();
	m_is_premium = 1;
}

UINT32 mbee_state::screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_framecnt++;
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}


MC6845_ON_UPDATE_ADDR_CHANGED( mbee_state::crtc_update_addr )
{
// not sure what goes in here - parameters passed are device, address, strobe
// not used on 256TC
}


/* monochrome bee */
MC6845_UPDATE_ROW( mbee_state::mono_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)           // for each character
	{
		UINT8 inv=0;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];

		oldkb_scan(x+ma);

		/* process cursor */
		if (x == cursor_x)
			inv ^= m_sy6545_cursor[ra];          // cursor scan row

		/* get pattern of pixels for that character scanline */
		gfx = m_p_gfxram[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

/* colour bee */
MC6845_UPDATE_ROW( mbee_state::colour_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 inv,attr,gfx,fg,bg,col;
	UINT16 mem,x,chr;
	UINT8 colourm = (m_08 & 0x0e) >> 1;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)           // for each character
	{
		inv = 0;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		col = m_p_colorram[mem];                     // read a byte of colour

		if BIT(m_1c, 7) // premium graphics enabled?
		{
			attr = m_p_attribram[mem];

			if BIT(chr, 7)
				chr += ((attr & 15) << 7);          // bump chr to its particular pcg definition

			if BIT(attr, 6)
				inv ^= 0xff;                    // inverse attribute

			if (BIT(attr, 7) & BIT(m_framecnt, 4))            // flashing attribute
				chr = 0x20;
		}

		oldkb_scan(x+ma);

		/* process cursor */
		if (x == cursor_x)
			inv ^= m_sy6545_cursor[ra];          // cursor scan row

		/* get pattern of pixels for that character scanline */
		gfx = m_p_gfxram[(chr<<4) | ra] ^ inv;

		// get colours
		if (m_is_premium)
		{
			fg = col & 15;
			bg = col >> 4;
		}
		else
		{
			fg = (col & 0x1f) | 64;
			bg = ((col & 0xe0) >> 2) | colourm;
		}

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
		*p++ = palette[BIT(gfx, 0) ? fg : bg];
	}
}


/*****************************************************************************************************

    Palette

    Standard Palette: The 8 bits from the colour RAM are divided into 5 bits for foreground, and 3
    bits for background. The 5 foreground bits pass through a PAL, which has 6 outputs. A write to
    port 8 produces 3 more background lines, giving 6 in total. These 12 lines travel to a pair of
    74LS157 switching chips, where the foreground or background lines are selected by the absence or
    or presence of a pixel. The 6 chosen lines pass through a 7407 then are merged to the final 3 RGB
    lines. Each pair is merged like this:
    VCC --- 330 ohm --- primary colour line + out to monitor ---- 120 ohm --- secondary colour line

*****************************************************************************************************/

PALETTE_INIT_MEMBER( mbee_state, standard )
{
	const UINT8 *color_prom = memregion("proms")->base();
	UINT8 i=0, r, b, g, k, r1, g1, b1;
	UINT8 bglevel[] = { 0, 0x54, 0xa0, 0xff };
	UINT8 fglevel[] = { 0, 0xa0, 0xff, 0xff };

	// set up background colours (00-63)
	for (b1 = 0; b1 < 4; b1++)
	{
		b = bglevel[b1];
		for (g1 = 0; g1 < 4; g1++)
		{
			g = bglevel[g1];
			for (r1 = 0; r1 < 4; r1++)
			{
				r = bglevel[r1];
				k = BITSWAP8(i, 7, 6, 5, 3, 1, 4, 2, 0);
				palette.set_pen_color(k, rgb_t(r, g, b));
				i++;
			}
		}
	}

	// set up foreground palette (64-95) by reading the prom
	for (i = 0; i < 32; i++)
	{
		k = color_prom[i];
		r = fglevel[(BIT(k, 2))|(BIT(k, 5)<<1)];
		g = fglevel[(BIT(k, 1))|(BIT(k, 4)<<1)];
		b = fglevel[(BIT(k, 0))|(BIT(k, 3)<<1)];
		palette.set_pen_color(i|64, rgb_t(r, g, b));
	}
}


PALETTE_INIT_MEMBER( mbee_state, premium )
{
	UINT8 i, r, b, g;

	/* set up 8 low intensity colours */
	for (i = 0; i < 8; i++)
	{
		r = BIT(i, 0) ? 0xa0 : 0;
		g = BIT(i, 1) ? 0xa0 : 0;
		b = BIT(i, 2) ? 0xa0 : 0;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	/* set up 8 high intensity colours */
	for (i = 9; i < 16; i++)
	{
		r = BIT(i, 0) ? 0xff : 0;
		g = BIT(i, 1) ? 0xff : 0;
		b = BIT(i, 2) ? 0xff : 0;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}
