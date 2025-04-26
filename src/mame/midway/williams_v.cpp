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
#include "williams.h"

/*************************************
 *
 *  Williams video startup
 *
 *************************************/

void williams_state::video_start()
{
	save_item(NAME(m_cocktail));
}


void blaster_state::video_start()
{
	williams_state::video_start();

	save_item(NAME(m_color0));
	save_item(NAME(m_video_control));
}


void williams2_state::video_start()
{
	williams_state::video_start();

	// create the tilemap
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(williams2_state::get_tile_info)), TILEMAP_SCAN_COLS, 24,16, 128,16);
	m_bg_tilemap->set_scrolldx(2, 0);

	save_item(NAME(m_tilemap_xscroll));
	save_item(NAME(m_fg_color));
	save_item(NAME(m_gain));
	save_item(NAME(m_offset));
}



/*************************************
 *
 *  Williams video update
 *
 *************************************/

uint32_t williams_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// precompute the palette
	rgb_t pens[16];
	for (int x = 0; x < 16; x++)
		pens[x] = m_palette->pen_color(m_paletteram[x]);

	// loop over rows
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t const *const source = &m_videoram[y];
		uint32_t *const dest = &bitmap.pix(y);

		// loop over columns
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pix = source[(x / 2) * 256];
			dest[x + 0] = pens[pix >> 4];
			dest[x + 1] = pens[pix & 0x0f];
		}
	}
	return 0;
}


uint32_t blaster_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const palette_0 = &m_videoram[0xbb00];
	uint8_t const *const scanline_control = &m_videoram[0xbc00];
	rgb_t pens[16];

	// precompute the palette
	for (int x = 0; x < 16; x++)
		pens[x] = m_palette->pen_color(m_paletteram[x]);

	// if we're blitting from the top, start with a 0 for color 0
	if (cliprect.min_y == screen.visible_area().min_y || !(m_video_control & 1))
		m_color0 = m_palette->pen_color(palette_0[0] ^ 0xff);

	// loop over rows
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int const erase_behind = m_video_control & scanline_control[y] & 2;
		uint8_t *const source = &m_videoram[y];
		uint32_t *const dest = &bitmap.pix(y);

		// latch a new color0 pen?
		if (m_video_control & scanline_control[y] & 1)
			m_color0 = m_palette->pen_color(palette_0[y] ^ 0xff);

		// loop over columns
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pix = source[(x/2) * 256];

			// clear behind us if requested
			if (erase_behind)
				source[(x/2) * 256] = 0;

			// now draw
			dest[x+0] = (pix & 0xf0) ? pens[pix >> 4] : rgb_t(m_color0 | pens[0]);
			dest[x+1] = (pix & 0x0f) ? pens[pix & 0x0f] : rgb_t(m_color0 | pens[0]);
		}
	}
	return 0;
}


uint32_t williams2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];

	// draw the background
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// fetch the relevant pens
	for (int x = 1; x < 16; x++)
		pens[x] = m_palette->pen_color(m_fg_color * 16 + x);

	// loop over rows
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint8_t const *const source = &m_videoram[y];
		uint32_t *const dest = &bitmap.pix(y);

		// loop over columns
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pix = source[(x/2) * 256];

			if (pix & 0xf0)
				dest[x+0] = pens[pix >> 4];
			if (pix & 0x0f)
				dest[x+1] = pens[pix & 0x0f];
		}
	}
	return 0;
}


uint32_t mysticm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];

	// draw the background
	m_bg_tilemap->mark_all_dirty();
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	// loop over rows
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// fetch the relevant pens
		for (int x = 1; x < 16; x++)
			pens[x] = m_palette->pen_color(color_decode(m_fg_color, 1, y) * 16 + x);

		uint8_t const *const source = &m_videoram[y];
		uint32_t *const dest = &bitmap.pix(y);

		// loop over columns
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pix = source[(x/2) * 256];

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


rgb_t williams2_state::calc_col(uint16_t lo, uint16_t hi)
{
	/*
	 *  frgb contains channel output voltages created with this netlist file:
	 *      src/lib/netlist/examples/turkey_shoot.cpp
	 *  Instructions to create the table are found in turkey_shoot.cpp
	 *
	 *  Reference videos: https://www.youtube.com/watch?v=R5OeC6Wc_yI
	 *                    https://www.youtube.com/watch?v=3J_EZ1OXlww
	 *                    https://www.youtube.com/watch?v=zxZ48iJShSU
	 *
	 *
	 *  FIXME: The long term plan is to include the functionality of
	 *  nltool/nlwav into the netlist core, launch a netlist run and
	 *  create the table on the fly. This however needs some significant
	 *  investment. This table is a float table since integer would loose
	 *  dynamic range during offset and gain adjustments.
	 *
	 */

	static const float frgb[256] =
	{
		0.001889f, 0.002456f, 0.003196f, 0.004384f, 0.005272f, 0.007672f, 0.011345f, 0.017759f,
		0.018643f, 0.03103f,  0.051912f, 0.087251f, 0.1256f,   0.204967f, 0.331644f, 0.529628f,
		0.002524f, 0.003306f, 0.004371f, 0.006072f, 0.007386f, 0.010812f, 0.016096f, 0.02509f,
		0.02687f,  0.043747f, 0.070899f, 0.114042f, 0.159992f, 0.250108f, 0.391104f, 0.607433f,
		0.003522f, 0.004666f, 0.006254f, 0.008766f, 0.010795f, 0.015856f, 0.023585f, 0.036228f,
		0.039487f, 0.062206f, 0.096916f, 0.148811f, 0.20338f,  0.305447f, 0.462741f, 0.700199f,
		0.004941f, 0.006607f, 0.00895f,  0.012619f, 0.015683f, 0.022944f, 0.033832f, 0.050765f,
		0.056018f, 0.085033f, 0.127452f, 0.187886f, 0.25112f,  0.364998f, 0.53879f,  0.797876f,

		0.006857f, 0.009233f, 0.012592f, 0.017771f, 0.022172f, 0.03212f,  0.046615f, 0.06811f,
		0.075696f, 0.110906f, 0.160678f, 0.229149f, 0.300735f, 0.42593f,  0.61584f,  0.896241f,
		0.010113f, 0.01368f,  0.018726f, 0.02625f,  0.032735f, 0.046462f, 0.065809f, 0.092996f,
		0.103803f, 0.146292f, 0.204648f, 0.282479f, 0.364084f, 0.502781f, 0.712244f, 1.018759f,
		0.015744f, 0.021251f, 0.028944f, 0.039945f, 0.049446f, 0.068127f, 0.093445f, 0.127285f,
		0.14231f,  0.193004f, 0.261153f, 0.349656f, 0.443068f, 0.597642f, 0.830444f, 1.168267f,
		0.024223f, 0.032378f, 0.043467f, 0.058612f, 0.071646f, 0.09563f,  0.127147f, 0.167588f,
		0.187237f, 0.246001f, 0.32398f,  0.423282f, 0.528958f, 0.699986f, 0.957283f, 1.328146f,

		0.03429f,  0.045182f, 0.059626f, 0.078588f, 0.094882f, 0.123449f, 0.160233f, 0.206173f,
		0.230031f, 0.295596f, 0.382005f, 0.490617f, 0.607092f, 0.792615f, 1.071663f, 1.471949f,
		0.053106f, 0.068185f, 0.087712f, 0.112105f, 0.132967f, 0.167685f, 0.211656f, 0.265082f,
		0.295044f, 0.369837f, 0.467993f, 0.58972f,  0.721623f, 0.927771f, 1.23799f,  1.680648f,
		0.083576f, 0.103924f, 0.129726f, 0.160516f, 0.186838f, 0.228722f, 0.281144f, 0.343382f,
		0.381103f, 0.46708f,  0.5795f,   0.717377f, 0.86859f,  1.100548f, 1.449999f, 1.946101f,
		0.124032f, 0.149904f, 0.182177f, 0.219335f, 0.251408f, 0.300484f, 0.361692f, 0.433113f,
		0.479348f, 0.577076f, 0.704911f, 0.860274f, 1.032601f, 1.292736f, 1.685276f, 2.240223f,

		0.175706f, 0.207011f, 0.245689f, 0.289706f, 0.327912f, 0.384501f, 0.455038f, 0.536297f,
		0.592068f, 0.702442f, 0.847309f, 1.021943f, 1.217772f, 1.509243f, 1.949852f, 2.570528f,
		0.24927f,  0.287025f, 0.333877f, 0.385984f, 0.431717f, 0.497443f, 0.579727f, 0.673464f,
		0.741572f, 0.868017f, 1.034559f, 1.234123f, 1.460333f, 1.79207f,  2.295118f, 3.001228f,
		0.357357f, 0.403448f, 0.460931f, 0.52375f,  0.579557f, 0.656404f, 0.754715f, 0.865436f,
		0.950203f, 1.097887f, 1.293872f, 1.527752f, 1.795589f, 2.182008f, 2.770169f, 3.593798f,
		0.494255f, 0.549094f, 0.61884f,  0.693823f, 0.760527f, 0.851544f, 0.968276f, 1.098965f,
		1.203912f, 1.377026f, 1.608183f, 1.882244f, 2.200036f, 2.652345f, 3.342626f, 4.215708f
	};

	// update the palette entry
	const uint16_t i =  (hi >> 4) & 15;
	const uint16_t ub = (hi >> 0) & 15;
	const uint16_t ug = (lo >> 4) & 15;
	const uint16_t ur = (lo >> 0) & 15;

	// normalize
	float r = frgb[i * 16 + ur] / 4.22f;
	float g = frgb[i * 16 + ug] / 4.22f;
	float b = frgb[i * 16 + ub] / 4.22f;

	// cut off
	r = std::max(r + m_offset[0], 0.0f);
	g = std::max(g + m_offset[1], 0.0f);
	b = std::max(b + m_offset[2], 0.0f);

	// drive
	r = std::min(r * m_gain[0] / 0.25f, 1.0f);
	g = std::min(g * m_gain[1] / 0.25f, 1.0f);
	b = std::min(b * m_gain[2] / 0.25f, 1.0f);

	return rgb_t(int(r * 255), int(g * 255), int(b * 255));
}


void williams2_state::paletteram_w(offs_t offset, u8 data)
{
	// set the new value
	m_paletteram[offset] = data;

	// pull the associated low/high bytes
	uint16_t entry_lo = m_paletteram[offset & ~1];
	uint16_t entry_hi = m_paletteram[offset |  1];

	m_bg_tilemap->mark_all_dirty();

	m_palette->set_pen_color(offset / 2, calc_col(entry_lo, entry_hi));
}


void williams2_state::rebuild_palette()
{
	for (offs_t i = 0; i < 2048; i++)
		paletteram_w(i, m_paletteram[i]);
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


u8 williams2_state::video_counter_r()
{
	return m_screen->vpos() & 0xff;
}



/*************************************
 *
 *  Tilemap handling
 *
 *************************************/

TILE_GET_INFO_MEMBER(williams2_state::get_tile_info)
{
	int const mask = m_gfxdecode->gfx(0)->elements() - 1;
	int const data = m_tileram[tile_index];
	int const y = (tile_index >> 1) & 7;

	// On tshoot and inferno, IC79 is a 74LS157 selector jumpered to be enabled
	int const color = y;

	tileinfo.set(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}


int mysticm_state::color_decode(uint8_t base_col, int sig_J1, int y)
{
	int const v = y << 6;
	int const sig_W11 = (v >> 11) & 1;
	int const sig_W12 = (v >> 12) & 1;
	int const sig_W13 = (v >> 13) & 1;

	// There are four "jumpers" in the schematics.
	// J3 and J4 allow to turn off background tilemaps completely.
	// BACKSEL (active low) in this case is forced to high.
	// J1 and J2 allow to turn on/off "sky" processing. In this case,
	// for sky (up to ~1/3 of vertical resolution an alternative palette
	// is used. For mysticm it is connected to BACKSEL.

	// Cascading inputs ">" and "=" are set to "H", thus
	// cascading input "<" (connected to W11) has no effect
	// according to truthtable for 7485. Thus there are two possibilities:
	// a. A real 7485 works different to the datasheet
	// b. input "=" on the real board is connected to GND.

	// FIXME: Investigate further.

	// IC79 is a 74LS85 comparator that controls the low bit
	int const a = 1 | ((base_col & 1) << 2) | ((base_col & 1) << 3);
	int const b = (sig_W12 << 0) | (sig_W13 << 1) | (0 << 2) | (sig_J1 << 3);
	int const color = (a > b) || ((a == b) && !sig_W11);

	// mysticm schematics show Page1 and Page2 crossed, i.e.
	// Page1 -> B2 (IC80) and Page2 -> B1 (IC80)
	// This does not produce colors observed on real hardware.
	// FIXME: Verify Page1 and Page2 connections.
	//return ((base_col & 0x04) >> 1) | ((base_col & 0x02) << 1) | (base_col & 0x38) | color;
	return (base_col & 0x3e) | color;
}


TILE_GET_INFO_MEMBER(mysticm_state::get_tile_info)
{
	int const color = color_decode(m_bg_color, 0, (tile_index << 4) & 0xff);

	int const mask = m_gfxdecode->gfx(0)->elements() - 1;
	int const data = m_tileram[tile_index];

	//m_bg_tilemap->set_palette_offset((color & 0x3e) << 4);
	//tileinfo.set(0, data & mask, color & 1, (data & ~mask) ? TILE_FLIPX : 0);
	m_bg_tilemap->set_palette_offset(0);
	tileinfo.set(0, data & mask, (color & 0x3f), (data & ~mask) ? TILE_FLIPX : 0);

	//gfx_element *gfx = tileinfo.decoder->gfx(0);
	//printf("%d %d %d %d\n", gfx->elements(), gfx->colorbase(), gfx->granularity(), gfx->colors());

}

TILE_GET_INFO_MEMBER(joust2_state::get_tile_info)
{
	int const mask = m_gfxdecode->gfx(0)->elements() - 1;
	int const data = m_tileram[tile_index];

	// IC79 is a 74LS157 selector jumpered to be disabled
	int const color = 0;

	tileinfo.set(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}

// based on the board type, only certain bits are used
// the rest are determined by other factors

void williams2_state::bg_select_w(u8 data)
{
	// IC79 is a 74LS157 selector jumpered to be enabled
	m_bg_tilemap->set_palette_offset((data & 0x38) << 4);
}

void mysticm_state::bg_select_w(u8 data)
{
	// IC79 is a 74LS85 comparator that controls the low bit
	m_bg_color = data;
	m_bg_tilemap->mark_all_dirty();
}

void joust2_state::bg_select_w(u8 data)
{
	// IC79 is a 74LS157 selector jumpered to be disabled
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
