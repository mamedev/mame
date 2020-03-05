// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams 6809 system

****************************************************************************

    The basic video system involves a 4-bit-per-pixel bitmap, oriented
    in inverted X/Y order. That is, pixels (0,0) and (1,0) come from the
    byte at offset 0. Pixels (2,0) and (3,0) come from the byte at offset
    256. Pixels (4,0) and (5,0) come from the byte at offset 512. Etc.

    Defender and Stargate simply draw graphics to the framebuffer directly
    with no extra intervention.

    Later games added a pair of "special chips" (Special Chip 1, and Special
    Chip 2, abbreviated for clarity as SC1 and SC2) to the board which
    are special purpose blitters. During their operation they HALT the
    main CPU so that they can control the busses. The operation of the
    chips is described in detail below.

    The original SC1 had a bug that forced an XOR of the width and height
    values with 4. This was fixed in the SC2, which was used on several
    later games.

    Beginning with Sinistar, additional video tweaks were added.

    In Sinistar, a clipping window can be specified and enabled in order
    to prevent the blitter chip from drawing beyond a certain address.
    This clipping window can be switched on and off at will.

    In Blaster, a number of features were added. First, a fixed window can
    be enabled which cuts off blitter drawing at 0x9700. Second, on a
    per-scanline basis, an "erase behind" feature can be turned on which
    clears the video RAM to 0 after it is refreshed to the screen. Third,
    on a per-scanline basis, an alternate color can be latched as the new
    background color.

    For Mystic Marathon and the 3 other "2nd generation" Williams games,
    a tilemap background layer was added. This layer consisted of 24x16
    tiles and only scrolled in the X direction. In addition, the palette
    was expanded to 1024 entries, some of which were used for the tilemap.
    The traditional foreground bitmap could be configured to use any bank
    of 16 colors from the full palette.

****************************************************************************

    Blitter description from Sean Riddle's page:

    This page contains information about the Williams Special Chips, which
    were 'bit blitters'- block transfer chips that could move data around on
    the screen and in memory faster than the CPU. In fact, I've timed the
    special chips at 16 megs in 18.1 seconds. That's 910K/sec, not bad for
    the early 80s.

    The blitters were not used in Defender and Stargate, but
    were added to the ROM boards of the later games. Splat!, Blaster, Mystic
    Marathon and Joust 2 used Special Chip 2s. The only difference that I've
    seen is that SC1s have a small bug. When you tell the SC1 the size of
    the data to move, you have to exclusive-or the width and height with 4.
    The SC2s eliminate this bug.

    The blitters were accessed at memory location $CA00-CA06.

    CA01 is the mask, usually $FF to move all bits.
    CA02-3 is the source data location.
    CA04-5 is the destination data location.

    Writing to CA00 starts the blit, and the byte written determines how the
    data is blitted.

    Bit 0 indicates that the source data is either laid out linear, one
    pixel after the last, or in screen format, where there are 256 bytes from
    one pair of pixels to the next.

    Bit 1 indicates the same, but for the destination data.

    I'm not sure what bit 2 does. Looking at the image, I can't tell, but
    perhaps it has to do with the mask. My test files only used a mask of $FF.

    Bit 3 tells the blitter only to blit the foreground- that is, everything
    that is not color 0. Also known as transparency mode.

    Bit 4 is 'solid' mode. Only the color indicated by the mask is blitted.
    Note that this just creates a rectangle unless bit 3 is also set, in which
    case it blits the image, but in a solid color.

    Bit 5 shifts the image one pixel to the right. Any data on the far right
    jumps to the far left.

    Bits 6 and 7 only blit every other pixel of the image. Bit 6 says even only,
    while bit 7 says odd only.

******************************************************************************
    Special Chip 1 and 2 aka VLSI VL2001/2001A Pinout:
                               _______    _______
                             _|*      \__/       |_
                     /E <-  |_|1               40|_| ?-> /WINH
                             _|                  |_
                    TCF <-  |_|2               39|_|  -> A15
                             _|                  |_
                  /HTCF <-? |_|3               38|_|  -> A14
                             _|                  |_
                     D7 <>  |_|4               37|_|  -> A13
                             _|                  |_
                  /HALT ->  |_|5               36|_|  <- /RESET
                             _|                  |_
                  /BABS ->  |_|6               35|_|  == /4MHZ
                             _|                  |_
                     D6 <>  |_|7               34|_|  -> A12
                             _|                  |_
                     D5 <>  |_|8               33|_|  -> A11
                             _|                  |_
                     D4 <>  |_|9     5     V   32|_|  -> A10
                             _|      4     T     |_
 (not bonded, pcb GND) N/C  |_|10    1  V  I   31|_|  -> A9
                             _|      0  L        |_
                     D3 <>  |_|11    -  2      30|_|  -> A8
                             _|      0  0  8     |_
                     D2 <>  |_|12    9  0  2   29|_|  -- VCC(+5v)
                             _|      8  1  2     |_
                     D1 <>  |_|13    6     0   28|_|  <> A0
                             _|      6           |_
                    U/L ->  |_|14              27|_|  <> A1
                             _|                  |_
(not bonded, pcb +12v) N/C  |_|15              26|_|  <> A2
                             _|                  |_
                    GND --  |_|16              25|_|  <- /CS
                             _|                  |_
                     D0 <>  |_|17              24|_|  N/C (not bonded, pcb GND)
                             _|                  |_
                    R/W <>? |_|18              23|_|  -> A7
                             _|                  |_
                     A3 <-  |_|19              22|_|  -> A6
                             _|                  |_
                     A4 <-  |_|20              21|_|  -> A5
                              |__________________|

The full silkscreen markings of SC1 (under the "Special Chip 1" sticker, if it is present) are:
          VTI  8220
            VL2001
          5410-09866

The full silkscreen markings of SC2 (under the "Special Chip 2" sticker, if it is present) are:
<VTi Logo> 242
          VL2001A
          5410-09958
******************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/williams.h"

/*************************************
 *
 *  Williams video startup
 *
 *************************************/

void williams_state::state_save_register()
{
	save_item(NAME(m_blitter_window_enable));
	save_item(NAME(m_cocktail));
	save_item(NAME(m_blitterram));
	save_item(NAME(m_blitter_remap_index));
}


void williams_state::video_start()
{
	blitter_init(m_blitter_config, nullptr);
	state_save_register();
}


void blaster_state::video_start()
{
	blitter_init(m_blitter_config, memregion("proms")->base());
	state_save_register();
	save_item(NAME(m_color0));
	save_item(NAME(m_video_control));
}


void williams2_state::video_start()
{
	blitter_init(m_blitter_config, nullptr);

	/* create the tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(williams2_state::get_tile_info)), TILEMAP_SCAN_COLS,  24,16, 128,16);
	m_bg_tilemap->set_scrolldx(2, 0);

	state_save_register();
	save_item(NAME(m_tilemap_xscroll));
	save_item(NAME(m_fg_color));
}



/*************************************
 *
 *  Williams video update
 *
 *************************************/

uint32_t williams_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];
	int x, y;

	/* precompute the palette */
	for (x = 0; x < 16; x++)
		pens[x] = m_palette->pen_color(m_paletteram[x]);

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t *source = &m_videoram[y];
		uint32_t *dest = &bitmap.pix32(y);

		/* loop over columns */
		for (x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			int pix = source[(x/2) * 256];
			dest[x+0] = pens[pix >> 4];
			dest[x+1] = pens[pix & 0x0f];
		}
	}
	return 0;
}


uint32_t blaster_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];

	/* precompute the palette */
	for (int x = 0; x < 16; x++)
		pens[x] = m_palette->pen_color(m_paletteram[x]);

	/* if we're blitting from the top, start with a 0 for color 0 */
	if (cliprect.min_y == screen.visible_area().min_y || !(m_video_control & 1))
		m_color0 = m_palette->pen_color(m_palette_0[0] ^ 0xff);

	/* loop over rows */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int erase_behind = m_video_control & m_scanline_control[y] & 2;
		uint8_t *source = &m_videoram[y];
		uint32_t *dest = &bitmap.pix32(y);

		/* latch a new color0 pen? */
		if (m_video_control & m_scanline_control[y] & 1)
			m_color0 = m_palette->pen_color(m_palette_0[y] ^ 0xff);

		/* loop over columns */
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			int pix = source[(x/2) * 256];

			/* clear behind us if requested */
			if (erase_behind)
				source[(x/2) * 256] = 0;

			/* now draw */
			dest[x+0] = (pix & 0xf0) ? pens[pix >> 4] : rgb_t(m_color0 | pens[0]);
			dest[x+1] = (pix & 0x0f) ? pens[pix & 0x0f] : rgb_t(m_color0 | pens[0]);
		}
	}
	return 0;
}


uint32_t williams2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];

	/* draw the background */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* fetch the relevant pens */
	for (int x = 1; x < 16; x++)
		pens[x] = m_palette->pen_color(m_fg_color * 16 + x);

	/* loop over rows */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t *source = &m_videoram[y];
		uint32_t *dest = &bitmap.pix32(y);

		/* loop over columns */
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			int pix = source[(x/2) * 256];

			if (pix & 0xf0)
				dest[x+0] = pens[pix >> 4];
			if (pix & 0x0f)
				dest[x+1] = pens[pix & 0x0f];
		}
	}
	return 0;
}



/*************************************
 *
 *  Williams palette I/O
 *
 *************************************/

void williams_state::palette_init(palette_device &palette) const
{
	static constexpr int resistances_rg[3] = { 1200, 560, 330 };
	static constexpr int resistances_b[2]  = { 560, 330 };

	// compute palette information
	// note that there really are pullup/pulldown resistors, but this situation is complicated
	// by the use of transistors, so we ignore that and just use the relative resistor weights
	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_r, 0, 0,
			3, resistances_rg, weights_g, 0, 0,
			2, resistances_b,  weights_b, 0, 0);

	// build a palette lookup
	for (int i = 0; i < 256; i++)
	{
		int const r = combine_weights(weights_r, BIT(i, 0), BIT(i, 1), BIT(i, 2));
		int const g = combine_weights(weights_g, BIT(i, 3), BIT(i, 4), BIT(i, 5));
		int const b = combine_weights(weights_b, BIT(i, 6), BIT(i, 7));

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



void williams2_state::paletteram_w(offs_t offset, u8 data)
{
	uint8_t entry_lo, entry_hi, r, g, b, i;

	/* Lookup table of 4-bit intensity (rows) vs. 4-bit color (columns) */
	/* This was generated by breadboarding a circuit to duplicate the DAC in the lower */
	/* right corner of the video schematic for Inferno, Mystic Marathon, and Turkey Shoot. */
	/* Both the intensity circuit and one of the 3 identical color circuits was implemented. */
	/* For each of the 256 combinations, a voltage was measured and was found to fall */
	/* within the range of .37v to 4.85v, very close to the expected 0-5v range */
	/* Each of these values was then scaled to an 8-bit 0-255 number that can then */
	/* be directly used to feed the three color values for the rgb_t function */
	static const uint8_t ivsc[16][16] = {
		{ 19, 21, 23, 25, 26, 29, 32, 35, 38, 43, 49, 56, 65, 76, 96, 108 },
		{ 21, 22, 24, 26, 28, 30, 34, 37, 40, 45, 52, 59, 68, 80, 101, 114 },
		{ 22, 24, 26, 28, 30, 33, 36, 39, 43, 48, 55, 63, 73, 86, 107, 121 },
		{ 24, 25, 27, 29, 32, 35, 38, 42, 46, 52, 59, 67, 77, 91, 114, 129 },
		{ 25, 27, 29, 31, 34, 37, 40, 45, 48, 54, 62, 71, 81, 96, 121, 137 },
		{ 27, 28, 31, 34, 36, 39, 44, 48, 52, 58, 66, 76, 87, 103, 129, 146 },
		{ 29, 31, 34, 36, 39, 43, 47, 52, 56, 63, 72, 82, 94, 111, 140, 158 },
		{ 32, 34, 37, 39, 43, 46, 51, 56, 61, 68, 78, 89, 102, 120, 151, 171 },
		{ 32, 35, 38, 41, 44, 48, 53, 59, 64, 72, 83, 94, 109, 129, 161, 182 },
		{ 36, 38, 42, 45, 48, 53, 59, 65, 70, 79, 90, 104, 119, 141, 177, 201 },
		{ 40, 43, 46, 50, 54, 59, 65, 72, 79, 88, 101, 115, 133, 157, 198, 224 },
		{ 45, 48, 52, 57, 61, 66, 74, 81, 88, 98, 113, 129, 149, 176, 221, 249 },
		{ 50, 54, 58, 64, 68, 75, 83, 91, 99, 111, 128, 146, 169, 200, 249, 253 },
		{ 58, 63, 68, 74, 79, 87, 96, 106, 116, 129, 148, 169, 195, 231, 253, 254 },
		{ 71, 76, 83, 89, 96, 105, 116, 128, 139, 156, 179, 205, 236, 252, 254, 254 },
		{ 91, 97, 105, 114, 123, 133, 147, 161, 176, 196, 223, 249, 252, 254, 254, 255 }
	};

	/* set the new value */
	m_paletteram[offset] = data;

	/* pull the associated low/high bytes */
	entry_lo = m_paletteram[offset & ~1];
	entry_hi = m_paletteram[offset |  1];

	/* extract the 4-bit intensity, blue, red, and green values from the low/high bytes */
	i = ((entry_hi >> 4) & 15);
	b = ((entry_hi >> 0) & 15);
	g = ((entry_lo >> 4) & 15);
	r = ((entry_lo >> 0) & 15);

	/* Mystic Marathon red and blue each appear to have two bits swapped.  Rather than 1248 we need 1842. */
	/* Since this may not be needed for Inferno or Turkey Shoot, which appear to have the same video DAC circuit, */
	/* either some unique part of the circuit swaps them, or possibly the ROM rip for Mystic Marathon is wrong. */
	if (m_williams2_tilemap_config == WILLIAMS_TILEMAP_MYSTICM) {
		r = (r & 1) | ((r & 2) << 2) | (r & 4) | ((r & 8) >> 2);
		b = (b & 1) | ((b & 2) << 2) | (b & 4) | ((b & 8) >> 2);
	}

	/* cross reference 4-bit intensity vs. each 4-bit color to get the 3 final 8-bit color values */
	b = ivsc[i][b];
	g = ivsc[i][g];
	r = ivsc[i][r];

	/* update the palette entry */
	m_palette->set_pen_color(offset / 2, rgb_t(r, g, b));
}


void williams2_state::fg_select_w(u8 data)
{
	m_fg_color = data & 0x3f;
}



/*************************************
 *
 *  Video position readout
 *
 *************************************/

u8 williams_state::video_counter_r()
{
	if (m_screen->vpos() < 0x100)
		return m_screen->vpos() & 0xfc;
	else
		return 0xfc;
}



/*************************************
 *
 *  Tilemap handling
 *
 *************************************/

TILE_GET_INFO_MEMBER(williams2_state::get_tile_info)
{
	int mask = m_gfxdecode->gfx(0)->elements() - 1;
	int data = m_tileram[tile_index];
	int y = (tile_index >> 1) & 7;

	/* On tshoot and inferno, IC79 is a 74LS157 selector jumpered to be enabled */
	int color = y;

	SET_TILE_INFO_MEMBER(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(mysticm_state::get_tile_info)
{
	int mask = m_gfxdecode->gfx(0)->elements() - 1;
	int data = m_tileram[tile_index];
	int y = (tile_index >> 1) & 7;
	int color = 0; // TODO: This seems highly suspect. Could it be why the palette doesn't work in mysticm?

	/* IC79 is a 74LS85 comparator that controls the low bit */
	int a = 1 | ((color & 1) << 2) | ((color & 1) << 3);
	int b = ((y & 6) >> 1);
	int casc = (y & 1);
	color = (a > b) || ((a == b) && !casc);

	SET_TILE_INFO_MEMBER(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(joust2_state::get_tile_info)
{
	int mask = m_gfxdecode->gfx(0)->elements() - 1;
	int data = m_tileram[tile_index];

	/* IC79 is a 74LS157 selector jumpered to be disabled */
	int color = 0;

	SET_TILE_INFO_MEMBER(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}

/* based on the board type, only certain bits are used */
/* the rest are determined by other factors */

void williams2_state::bg_select_w(u8 data)
{
	/* IC79 is a 74LS157 selector jumpered to be enabled */
	m_bg_tilemap->set_palette_offset((data & 0x38) << 4);
}

void mysticm_state::bg_select_w(u8 data)
{
	/* IC79 is a 74LS85 comparator that controls the low bit */
	m_bg_tilemap->set_palette_offset((data & 0x3e) << 4);
}

void joust2_state::bg_select_w(u8 data)
{
	/* IC79 is a 74LS157 selector jumpered to be disabled */
	m_bg_tilemap->set_palette_offset((data & 0x3f) << 4);
}

void williams2_state::tileram_w(offs_t offset, u8 data)
{
	m_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void williams2_state::xscroll_low_w(u8 data)
{
	m_tilemap_xscroll = (m_tilemap_xscroll & ~0x00f) | ((data & 0x80) >> 4) | (data & 0x07);
	m_bg_tilemap->set_scrollx(0, (m_tilemap_xscroll & 7) + ((m_tilemap_xscroll >> 3) * 6));
}


void williams2_state::xscroll_high_w(u8 data)
{
	m_tilemap_xscroll = (m_tilemap_xscroll & 0x00f) | (data << 4);
	m_bg_tilemap->set_scrollx(0, (m_tilemap_xscroll & 7) + ((m_tilemap_xscroll >> 3) * 6));
}



/*************************************
 *
 *  Blaster-specific enhancements
 *
 *************************************/

void blaster_state::remap_select_w(u8 data)
{
	m_blitter_remap_index = data;
	m_blitter_remap = m_blitter_remap_lookup.get() + data * 256;
}


void blaster_state::video_control_w(u8 data)
{
	m_video_control = data;
}



/*************************************
 *
 *  Blitter setup and control
 *
 *************************************/

void williams_state::blitter_init(int blitter_config, const uint8_t *remap_prom)
{
	static const uint8_t dummy_table[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

	/* by default, there is no clipping window - this will be touched only by games that have one */
	m_blitter_window_enable = 0;

	/* switch off the video config */
	m_blitter_xor = (blitter_config == WILLIAMS_BLITTER_SC1) ? 4 : 0;

	/* create the remap table; if no PROM, make an identity remap table */
	m_blitter_remap_lookup = std::make_unique<uint8_t[]>(256 * 256);
	m_blitter_remap_index = 0;
	m_blitter_remap = m_blitter_remap_lookup.get();
	for (int i = 0; i < 256; i++)
	{
		const uint8_t *table = remap_prom ? (remap_prom + (i & 0x7f) * 16) : dummy_table;
		for (int j = 0; j < 256; j++)
			m_blitter_remap_lookup[i * 256 + j] = (table[j >> 4] << 4) | table[j & 0x0f];
	}
}


WRITE8_MEMBER(williams_state::blitter_w)
{
	/* store the data */
	m_blitterram[offset] = data;

	/* only writes to location 0 trigger the blit */
	if (offset != 0)
		return;

	/* compute the starting locations */
	int sstart = (m_blitterram[2] << 8) + m_blitterram[3];
	int dstart = (m_blitterram[4] << 8) + m_blitterram[5];

	/* compute the width and height */
	int w = m_blitterram[6] ^ m_blitter_xor;
	int h = m_blitterram[7] ^ m_blitter_xor;

	/* adjust the width and height */
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	/* do the actual blit */
	int accesses = blitter_core(space, sstart, dstart, w, h, data);

	/* based on the number of memory accesses needed to do the blit, compute how long the blit will take */
	int estimated_clocks_at_4MHz = 4;
	if(data & WMS_BLITTER_CONTROLBYTE_SLOW)
	{
		estimated_clocks_at_4MHz += 4 * (accesses + 2);
	}
	else
	{
		estimated_clocks_at_4MHz += 2 * (accesses + 3);
	}

	m_maincpu->adjust_icount(-((estimated_clocks_at_4MHz + 3) / 4));

	/* Log blits */
	logerror("%04X:Blit @ %3d : %02X%02X -> %02X%02X, %3dx%3d, mask=%02X, flags=%02X, icount=%d, win=%d\n",
			m_maincpu->pc(), m_screen->vpos(),
			m_blitterram[2], m_blitterram[3],
			m_blitterram[4], m_blitterram[5],
			m_blitterram[6], m_blitterram[7],
			m_blitterram[1], m_blitterram[0],
			((estimated_clocks_at_4MHz + 3) / 4), m_blitter_window_enable);
}


void williams2_state::blit_window_enable_w(u8 data)
{
	m_blitter_window_enable = data & 0x01;
}



/*************************************
 *
 *  Blitter core
 *
 *************************************/

inline void williams_state::blit_pixel(address_space &space, int dstaddr, int srcdata, int controlbyte)
{
	/* always read from video RAM regardless of the bank setting */
	int curpix = (dstaddr < 0xc000) ? m_videoram[dstaddr] : space.read_byte(dstaddr);   //current pixel values at dest

	int solid = m_blitterram[1];
	unsigned char keepmask = 0xff;          //what part of original dst byte should be kept, based on NO_EVEN and NO_ODD flags

	//even pixel (D7-D4)
	if((controlbyte & WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY) && !(srcdata & 0xf0))    //FG only and src even pixel=0
	{
		if(controlbyte & WMS_BLITTER_CONTROLBYTE_NO_EVEN)
			keepmask &= 0x0f;
	}
	else
	{
		if(!(controlbyte & WMS_BLITTER_CONTROLBYTE_NO_EVEN))
			keepmask &= 0x0f;
	}

	//odd pixel (D3-D0)
	if((controlbyte & WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY) && !(srcdata & 0x0f))    //FG only and src odd pixel=0
	{
		if(controlbyte & WMS_BLITTER_CONTROLBYTE_NO_ODD)
			keepmask &= 0xf0;
	}
	else
	{
		if(!(controlbyte & WMS_BLITTER_CONTROLBYTE_NO_ODD))
			keepmask &= 0xf0;
	}

	curpix &= keepmask;
	if(controlbyte & WMS_BLITTER_CONTROLBYTE_SOLID)
		curpix |= (solid & ~keepmask);
	else
		curpix |= (srcdata & ~keepmask);

/* if the window is enabled, only blit to videoram below the clipping address */
/* note that we have to allow blits to non-video RAM (e.g. tileram, Sinistar $DXXX SRAM) because those */
/* are not blocked by the window enable */
	if (!m_blitter_window_enable || dstaddr < m_blitter_clip_address || dstaddr >= 0xc000)
		space.write_byte(dstaddr, curpix);
}


int williams_state::blitter_core(address_space &space, int sstart, int dstart, int w, int h, int controlbyte)
{
	int accesses = 0;

	/* compute how much to advance in the x and y loops */
	int sxadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256) ? 0x100 : 1;
	int syadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256) ? 1 : w;
	int dxadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256) ? 0x100 : 1;
	int dyadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256) ? 1 : w;

	int pixdata = 0;

	/* loop over the height */
	for (int y = 0; y < h; y++)
	{
		int source = sstart & 0xffff;
		int dest = dstart & 0xffff;

		/* loop over the width */
		for (int x = 0; x < w; x++)
		{
			if (!(controlbyte & WMS_BLITTER_CONTROLBYTE_SHIFT)) //no shift
			{
				blit_pixel(space, dest, m_blitter_remap[space.read_byte(source)], controlbyte);
			}
			else
			{   //shift one pixel right
				pixdata = (pixdata << 8) | m_blitter_remap[space.read_byte(source)];
				blit_pixel(space, dest, (pixdata >> 4) & 0xff, controlbyte);
			}
			accesses += 2;

			/* advance src and dst pointers */
			source = (source + sxadv) & 0xffff;
			dest   = (dest + dxadv) & 0xffff;
		}

		/* note that PlayBall! indicates the X coordinate doesn't wrap */
		if (controlbyte & WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256)
			dstart = (dstart & 0xff00) | ((dstart + dyadv) & 0xff);
		else
			dstart += dyadv;

		if (controlbyte & WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256)
			sstart = (sstart & 0xff00) | ((sstart + syadv) & 0xff);
		else
			sstart += syadv;
	}
	return accesses;
}
