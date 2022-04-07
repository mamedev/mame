// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Microbee video hardware
Originally written by Juergen Buchmueller, Dec 1999

Rewritten by Robbbert (see notes in driver file).

Operation of old keyboard:
    The design is taken from the SY6545 Application Note AN3. As the CRTC
    renders the picture, the MA lines also scan the keyboard. When a keydown
    is detected, the lightpen pin is activated, which sets bit 6 of the status
    register. The CPU continuously monitors this register. Once bit 6 is set,
    the CPU needs to determine which key is pressed. It firstly clears bit 6,
    then sets port 0B to 1. This prevents the CRTC from scanning the keyboard
    and causing interference. Next, the CPU writes an address to regs 18,19 and
    then instigates the transparent access. This checks one keyboard row, which
    if the pressed key is found, will once again set bit 6. If not found, the
    next row will be checked. Once found, the CPU clears bit 6 and port 0B, then
    works out the ascii key code of the pressed key.

Tests of old keyboard. Start mbeeic.
1. Load ASTEROIDS PLUS, stay in attract mode, hold down spacebar,
   it should only fire bullets. If it sometimes starts turning,
   thrusting or using the shield, then there is a problem.

2. Load SCAVENGER and make sure it doesn't go to the next level
   until you find the Exit.

3. At the Basic prompt, type in EDASM press enter. At the memory size
   prompt press enter. Now, make sure the keyboard works properly.

See drivers\mbee.cpp for any issues.

****************************************************************************/


#include "emu.h"
#include "includes/mbee.h"

WRITE_LINE_MEMBER( mbee_state::crtc_vs )
{
	m_b7_vs = state;
	if ((m_io_config->read() & 0xc0) == 0) // VS selected in config menu
		m_pio->port_b_write(pio_port_b_r());
}


/***********************************************************

    Handlers of video, colour, and attribute RAM

************************************************************/


u8 mbee_state::video_low_r(offs_t offset)
{
	if ((m_1c & 0x9f) == 0x90)
		return m_aram[offset];
	else
	if (m_0b)
		return m_pram[offset];
	else
		return m_vram[offset];
}

void mbee_state::video_low_w(offs_t offset, u8 data)
{
	if (BIT(m_1c, 4))
	{
		// non-premium attribute writes are discarded
		if (BIT(m_1c, 7))
			m_aram[offset] = data;
	}
	else
		m_vram[offset] = data;
}

u8 mbee_state::video_high_r(offs_t offset)
{
	if (BIT(m_08, 6) && BIT(m_features, 0))
		return m_cram[offset];
	else
		return m_pram[(((m_1c & 15) + 1) << 11) | offset];
}

void mbee_state::video_high_w(offs_t offset, u8 data)
{
	if (BIT(m_08, 6) && (m_0b==0) && BIT(m_features, 0))
		m_cram[offset] = data;
	else
		m_pram[(((m_1c & 15) + 1) << 11) | offset] = data;
}

void mbee_state::port0b_w(u8 data)
{
	m_0b = BIT(data, 0);
}

u8 mbee_state::port08_r()
{
	return m_08;
}

void mbee_state::port08_w(u8 data)
{
	m_08 = data & 0x4e;
}

u8 mbee_state::port1c_r()
{
	return m_1c;
}

void mbee_state::port1c_w(u8 data)
{
/*  d7 extended graphics (1=allow attributes and pcg banks)
    d5 bankswitch basic rom
    d4 select attribute ram
    d3..d0 select videoram bank */

	if (BIT(m_features, 3) && BIT(data, 7))
		m_1c = data;
	else
		m_1c = data & 0x30;

	if (m_basic)
		m_basic->set_entry(BIT(data, 5));
}


/***********************************************************

    CRTC-driven keyboard

************************************************************/


void mbee_state::oldkb_matrix_r(u16 offs)
{
	if (!BIT(m_features, 2))
	{
		u8 port = (offs >> 7) & 7;
		u8 bit = (offs >> 4) & 7;
		u8 extra = 0;
		u8 data = m_io_oldkb[port]->read();
		bool keydown  = BIT(data, bit);

		// This adds premium-style cursor keys to the old keyboard.
		// They are used by the pc85 menu. Premium keyboards already
		// have these keys fitted.
		if (!keydown && !BIT(m_features, 3))
		{
			if ((port == 0) || (port == 2) || (port == 3))
				extra = m_io_x7->read();
			else if (port == 7)
				extra = data;

			if (extra)
			{
				if (BIT(extra, 0)) // cursor up
				{
					if( port == 7 && bit == 1 ) keydown = 1;
					if( port == 0 && bit == 5 ) keydown = 1; // control E
				}
				else if (BIT(extra, 2)) // cursor down
				{
					if( port == 7 && bit == 1 ) keydown = 1;
					if( port == 3 && bit == 0 ) keydown = 1; // control X
				}
				else if (BIT(extra, 3)) // cursor left
				{
					if( port == 7 && bit == 1 ) keydown = 1;
					if( port == 2 && bit == 3 ) keydown = 1; // control S
				}
				else if (BIT(extra, 6)) // cursor right
				{
					if( port == 7 && bit == 1 ) keydown = 1;
					if( port == 0 && bit == 4 ) keydown = 1; // control D
				}
			}
		}

		if( keydown )
			m_crtc->assert_light_pen_input(); //lpen_strobe
	}
}


void mbee_state::oldkb_scan( u16 param )
{
	if (m_0b) return; // IC5 (pins 11,12,13)
	if (param & 15) return; // only scan once per row instead of 16 times
	oldkb_matrix_r(param);
}


/***********************************************************

    CRTC registers

************************************************************/

void mbee_state::m6545_index_w(u8 data)
{
	data &= 0x1f;
	m_sy6545_ind = data;
	m_crtc->address_w(data);
}

void mbee_state::m6545_data_w(u8 data)
{
	static const u8 sy6545_mask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0,0x3f,0xff};

	switch( m_sy6545_ind )
	{
	case 12:
		data &= 0x3f; // select alternate character set
		if( m_sy6545_reg[12] != data )
			memcpy(m_pram.get(), memregion("chargen")->base() + (((data & 0x30) == 0x20) << 11), 0x800);
		break;
	}
	m_sy6545_reg[m_sy6545_ind] = data & sy6545_mask[m_sy6545_ind];  /* save data in register */
	m_crtc->register_w(data);
}




/***********************************************************

    Video

************************************************************/

u32 mbee_state::screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_framecnt++;
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}


MC6845_ON_UPDATE_ADDR_CHANGED( mbee_state::crtc_update_addr )
{
// parameters passed are address, strobe
// not used on 256TC

	if (strobe)
		oldkb_matrix_r(address);
}


MC6845_UPDATE_ROW( mbee_state::crtc_update_row )
{
	if (!de)
		return;

	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	// colours
	u8 colourm = BIT(m_08, 1, 3);
	u8 monopal = BIT(m_io_config->read(), 4, 2);
	// if colour chosen on mono bee, default to amber
	if ((monopal==0) && !BIT(m_features, 0))
		monopal = 2;

	u32 *p = &bitmap.pix(y);
	u8 inv, attr=0, gfx, fg=96+monopal, bg=96, col=0;
	u16 mem, x, chr;

	for (x = 0; x < x_count; x++)           // for each character
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = m_vram[mem];

		if (BIT(m_1c, 7)) // premium graphics enabled?
		{
			attr = m_aram[mem];

			if (BIT(chr, 7))
				chr += ((attr & 15) << 7);          // bump chr to its particular pcg definition

			if (BIT(attr, 6))
				inv ^= 0xff;                    // inverse attribute

			if (BIT(attr, 7) & BIT(m_framecnt, 4))            // flashing attribute
				chr = 0x20;
		}

		oldkb_scan(x+ma);

		/* get pattern of pixels for that character scanline */
		gfx = m_pram[(chr<<4) | ra] ^ inv;

		// get colours
		if (monopal==0)
		{
			col = m_cram[mem];                     // read a byte of colour

			if (BIT(m_features, 3)) // premium
			{
				fg = col & 15;
				bg = col >> 4;
			}
			else
			{
				fg = (col & 0x1f) | 64;
				bg = ((col & 0xe0) >> 2) | colourm;
			}
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

void mbee_state::standard_palette(palette_device &palette) const
{
	constexpr u8 bglevel[] = { 0, 0x54, 0xa0, 0xff };
	constexpr u8 fglevel[] = { 0, 0xa0, 0xff, 0xff };

	// set up background colours (00-63)
	u8 i = 0;
	for (u8 b : bglevel)
	{
		for (u8 g : bglevel)
		{
			for (u8 r : bglevel)
			{
				u8 const k = bitswap<8>(i, 7, 6, 5, 3, 1, 4, 2, 0);
				palette.set_pen_color(k, rgb_t(r, g, b));
				i++;
			}
		}
	}

	// set up foreground palette (64-95) by reading the prom
	u8 const *const color_prom = memregion("proms")->base();
	for (i = 0; i < 32; i++)
	{
		u8 const k = color_prom[i];
		u8 const r = fglevel[(BIT(k, 2))|(BIT(k, 5)<<1)];
		u8 const g = fglevel[(BIT(k, 1))|(BIT(k, 4)<<1)];
		u8 const b = fglevel[(BIT(k, 0))|(BIT(k, 3)<<1)];
		palette.set_pen_color(i|64, rgb_t(r, g, b));
	}

	// monochrome palette
	palette.set_pen_color(96, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_pen_color(97, rgb_t(0x00, 0xff, 0x00)); // green
	palette.set_pen_color(98, rgb_t(0xf7, 0xaa, 0x00)); // amber
	palette.set_pen_color(99, rgb_t(0xff, 0xff, 0xff)); // white
}


void mbee_state::premium_palette(palette_device &palette) const
{
	// set up 8 low intensity colours
	for (u8 i = 0; i < 7; i++)
	{
		u8 const r = BIT(i, 0) ? 0xc0 : 0;
		u8 const g = BIT(i, 1) ? 0xc0 : 0;
		u8 const b = BIT(i, 2) ? 0xc0 : 0;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	// colour 8 is dark grey, rather than black
	palette.set_pen_color(8, rgb_t(96, 96, 96));

	// set up 8 high intensity colours
	for (u8 i = 9; i < 16; i++)
	{
		u8 const r = BIT(i, 0) ? 0xff : 0;
		u8 const g = BIT(i, 1) ? 0xff : 0;
		u8 const b = BIT(i, 2) ? 0xff : 0;
		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	// monochrome palette
	palette.set_pen_color(96, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_pen_color(97, rgb_t(0x00, 0xff, 0x00)); // green
	palette.set_pen_color(98, rgb_t(0xf7, 0xaa, 0x00)); // amber
	palette.set_pen_color(99, rgb_t(0xff, 0xff, 0xff)); // white
}
