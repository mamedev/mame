// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


    This hardware only generates sprites. But they're of various types,
    including some large "floating tilemap" ones.

    Sprites RAM is 0x40000 bytes long. All games write the sprites list
    at offset 0x3000. Each entry in the list holds data for a multi-sprite
    of up to 256 single-sprites. The list looks like this:

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Last sprite
                -e-- ---- ---- ----     ? (ignore global offsets, zooming etc. used on frame of map in grdians)
                --d- ---- ---- ----     Opaque
                ---c ---- ---- ----     0 = Each sprite specifies its size, 1 = Use the global size (following words)
                ---- b--- ---- ----     Shadow
                ---- -a98 ---- ----     Tile color depth
                ---- ---- 7654 3210     Number of sprites - 1

        2.w     fedc ba-- ---- ----     X global size
                ---- --98 7654 3210     X displacement

        4.w     fedc ba-- ---- ----     Y global size
                ---- --98 7654 3210     Y displacement

        6.w     f--- ---- ---- ----     Single-sprite(s) type: tile (0) or row of tiles (1)
                -edc ba98 7654 3210     Offset of the single-sprite(s) data (8 bytes each)


    A single-sprite can be a tile or some horizontal rows of tiles.

    Tile case:

        0.w     fedc ---- ---- ----
                ---- ba-- ---- ----     Number of tiles along X (1 << n)
                ---- --98 7654 3210     X

        2.w     fedc ---- ---- ----
                ---- ba-- ---- ----     Number of tiles along Y (1 << n)
                ---- --98 7654 3210     Y

        4.w     fedc ba98 765- ----     Color code (16 color steps)
                ---- ---- ---4 ----     Flip X
                ---- ---- ---- 3---     Flip Y
                ---- ---- ---- -210     Code (high bits)

        6.w                             Code (low bits)


    Row case:

        0.w     fedc ba-- ---- ----     Number of columns (local)
                ---- --98 7654 3210     X

        2.w     fedc ba-- ---- ----     Number of rows - 1 (local)
                ---- --98 7654 3210     Y

        4.w     f--- ---- ---- ----     Tile size: 8x8 (0) or 16x16 (1)
                -edc ba-- ---- ----     "Tilemap" page (0x2000 bytes each)
                ---- --98 7654 3210     "Tilemap" scroll X

        6.w     fedc ba9- ---- ----
                ---- ---8 7654 3210     "Tilemap" scroll Y


    Shadows (same principle as ssv.cpp):

    The low bits of the pens from a "shadowing" tile (regardless of color code)
    substitute the top bits of the color index (0-7fff) in the frame buffer.

    Note:

    Based on raster effect usage in Guardians it would appear this list is
    transformed into an alt format by the hardware.  It is unknown if this
    alt format is visible to the CPU for reading, or only writing the updated
    scroll positions.  We could do with attempting a RAM capture to see what
    this format really looks like if it is CPU visible as the current
    implementation of this copy / reformat operation is pure guesswork

***************************************************************************/

#include "emu.h"
#include "seta2.h"

/***************************************************************************

                                Video Registers

    Offset:     Bits:                   Value:

    0/2/4/6                             Horizontal: Sync, Blank, DSPdot, Cycle (same as ssv.cpp?)
    8/a/c/e                             Vertical  : Sync, Blank, DSPdot, Cycle (same as ssv.cpp?)

    10                                  Offxet X low bits (sub pixels)
    12                                  Offset X high bits (pixels)
    14                                  Zoom X? low bits
    16                                  Zoom X? high bits *

    18                                  Offset Y low bits (sub pixels)
    1a                                  Offset Y high bits (pixels)
    1c                                  Zoom Y? low bits
    1e                                  Zoom Y? high bits *

    24                                  1->0 in funcube3 and staraudi
    26                                  1->0 during INT0, before writing sprites
                                        (probably creates a custom format sprite list at 0x0000 by processing the list at 0x3000)

    30          fedc ba98 7654 321-
                ---- ---- ---- ---0     Disable video

    32..3f                              ?

    Global X offset values based on penbros

    0x1c0 - when zoom is smallest
    counts up to 0x7ff
    then 0x00 when finished

    counts up to 0x089 when zooming in

    Zoom values (both x and y) based on penbros and others (x flip/unflip logic is reverse of y logic)

    (unflipped gfx)
    0x7f5 00 = smallest
    0x7ff 00 = normal
    0x7ff xx = larger

    (flipped gfx, negative zoom factor!, used instead of flipscreen bits in some cases)
    0x00b 00 = smallest
    0x001 00 = normal
    0x001 xx = larger

***************************************************************************/

/***************************************************************************

  NON-BUGS

  grdians : After the fire rowscroll effect in the intro there is a small artifact
            left scrolling at the top of the screen when the next image is displayed
            See 4:24 in https://www.youtube.com/watch?v=cvHGFEsB_cM

***************************************************************************/

uint16_t seta2_state::vregs_r(offs_t offset)
{
	return m_vregs[offset];
}

void seta2_state::vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* 02/04 = horizontal display start/end
	           mj4simai = 0065/01E5 (0180 visible area)
	           myangel =  005D/01D5 (0178 visible area)
	           pzlbowl =  0058/01D8 (0180 visible area)
	           penbros =  0065/01A5 (0140 visible area)
	           grdians =  0059/0188 (012f visible area)
	   06    = horizontal total?
	           mj4simai = 0204
	           myangel =  0200
	           pzlbowl =  0204
	           penbros =  01c0
	           grdians =  019a
	*/

	uint16_t olddata = m_vregs[offset];

	COMBINE_DATA(&m_vregs[offset]);


	if (m_vregs[offset] != olddata)
		logerror("CPU #0 PC %06X: Video Reg %02X <- %04X\n", m_maincpu->pc(), offset * 2, data);

	switch (offset * 2)
	{
	case 0x1a:
		logerror("%s: Register 1a write (vertical offset?) %04X (%04x)\n", machine().describe_context(), data, mem_mask);
		break;

	case 0x1c:  // FLIP SCREEN (myangel)    <- this is actually zoom
		flip_screen_set(data & 1);
		if (data & ~1)  logerror("CPU #0 PC %06X: flip screen unknown bits %04X\n", m_maincpu->pc(), data);
		break;
	case 0x2a:  // FLIP X (pzlbowl)
		flip_screen_x_set(data & 1);
		if (data & ~1)  logerror("CPU #0 PC %06X: flipx unknown bits %04X\n", m_maincpu->pc(), data);
		break;
	case 0x2c:  // FLIP Y (pzlbowl)
		flip_screen_y_set(data & 1);
		if (data & ~1)  logerror("CPU #0 PC %06X: flipy unknown bits %04X\n", m_maincpu->pc(), data);
		break;

	case 0x30:  // BLANK SCREEN (pzlbowl, myangel)
		if (data & ~1)  logerror("CPU #0 PC %06X: blank unknown bits %04X\n", m_maincpu->pc(), data);
		break;

	case 0x24: // funcube3 and staraudi write here instead, why? mirror or different meaning?
	case 0x26: // something display list related? buffering control?
		if (data)
		{
			/* copy the base spritelist to a private (non-CPU visible buffer)
			   copy the indexed sprites to 0 in spriteram, adjusting pointers in base sprite list as appropriate
			   this at least gets the sprite data in the right place for the grdians raster effect to write the
			   changed scroll values to the correct sprites, but is still nothing more than a guess
			*/
			int current_sprite_entry = 0;

			for (int i = 0; i < 0x1000 / 2; i += 4)
			{
				uint16_t num = m_private_spriteram[i + 0] = m_spriteram[(0x3000 / 2) + i + 0];
				m_private_spriteram[i + 1] = m_spriteram[(0x3000 / 2) + i + 1];
				m_private_spriteram[i + 2] = m_spriteram[(0x3000 / 2) + i + 2];

				int sprite = m_spriteram[(0x3000 / 2) + i + 3];
				m_private_spriteram[i + 3] = ((current_sprite_entry / 4) & 0x7fff) | (sprite & 0x8000);

				int list2addr = (sprite & 0x7fff) * 4;

				num &=0xff;

				for (int j = 0; j <= num; j++)
				{
					if (current_sprite_entry < 0x3000 / 2)
					{
						m_spriteram[current_sprite_entry + 0] = m_spriteram[(list2addr + (j * 4) + 0) & 0x1ffff];
						m_spriteram[current_sprite_entry + 1] = m_spriteram[(list2addr + (j * 4) + 1) & 0x1ffff];
						m_spriteram[current_sprite_entry + 2] = m_spriteram[(list2addr + (j * 4) + 2) & 0x1ffff];
						m_spriteram[current_sprite_entry + 3] = m_spriteram[(list2addr + (j * 4) + 3) & 0x1ffff];
						current_sprite_entry += 4;
					}
				}

				if (m_private_spriteram[i + 0] & 0x8000) // end of list marker, mj4simai must draw the sprite this covers for the company logo, title screen etc.
				{
					// HACK: however penbros has a dummy sprite entry there which points to 0x0000 as the tile source, and causes garbage with the rearranged format,
					// so change it to something that's invalid where we can filter it later.  This strongly indicates that the current approach is incorrect however.
					if (sprite == 0x00)
					{
						m_private_spriteram[i + 3] |= 0x4000;
					}

					break;
				}
			}

		}
		break;

	case 0x3c: // Raster IRQ related
		//logerror("%s: Register 3c write (raster enable?) current vpos is %d  :   %04X (%04x)\n",machine().describe_context(),m_screen->vpos(), data, mem_mask);
		COMBINE_DATA(&m_rasterenabled);

		//if (m_rasterenabled & 1)
		{
			int hpos = 0;
			int vpos = m_rasterposition;

			// in the vblank it specifies line 0, the first raster interrupt then specifies line 0 again before the subsequent ones use the real line numbers?
			// It seems more likely that the raster IRQ stays asserted for the entire line, thus triggering a second interrupt unless the line number is changed?
			if (m_rasterposition == m_screen->vpos()) hpos = m_screen->hpos() + 0x100;
			//logerror("setting raster to %d %d\n", vpos, hpos);
			m_raster_timer->adjust(m_screen->time_until_pos(vpos, hpos), 0);
		}
		break;

	case 0x3e: // Raster IRQ related
		//logerror("%s: Register 3e write (raster position?) %04X (%04x)\n",machine().describe_context(),data, mem_mask);
		COMBINE_DATA(&m_rasterposition);
		break;
	}
}

uint16_t seta2_state::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void seta2_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

/***************************************************************************


                                Sprites Drawing


***************************************************************************/

inline void seta2_state::drawgfx_line(bitmap_ind16& bitmap, const rectangle& cliprect, int which_gfx, const uint8_t* const addr, const uint32_t realcolor, int flipx, int flipy, int base_sx, uint32_t xzoom, int use_shadow, int screenline, int line, int opaque)
{
	struct drawmodes
	{
		int gfx_mask;
		int gfx_shift;
		int shadow;
	};

	// this is the same logic as ssv.cpp, although this has more known cases, but also some bugs with the handling
	static constexpr drawmodes BPP_MASK_TABLE[8] = {
		{ 0xff, 0, 4 }, // 0: ultrax, twineag2 text - is there a local / global mixup somewhere, or is this an 'invalid' setting that just enables all planes?
		{ 0x30, 4, 2 }, // 1: unverified case, mimic old driver behavior of only using lowest bit  (myangel2 question bubble, myangel endgame)
		{ 0x07, 0, 3 }, // 2: unverified case, mimic old driver behavior of only using lowest bit  (myangel "Graduate Tests")
		{ 0xff, 0, 0 }, // 3: unverified case, mimic old driver behavior of only using lowest bit  (staraudi question bubble: pen %00011000 with shadow on!)
		{ 0x0f, 0, 3 }, // 4: eagle shot 4bpp birdie text
		{ 0xf0, 4, 4 }, // 5: eagle shot 4bpp japanese text
		{ 0x3f, 0, 5 }, // 6: common 6bpp case + keithlcy (logo), drifto94 (wheels) masking  ) (myangel sliding blocks test)
		{ 0xff, 0, 8 }, // 7: common 8bpp case
	};

	int shadow = BPP_MASK_TABLE[(which_gfx & 0x0700) >> 8].shadow;
	int gfx_mask = BPP_MASK_TABLE[(which_gfx & 0x0700) >> 8].gfx_mask;
	int gfx_shift = BPP_MASK_TABLE[(which_gfx & 0x0700) >> 8].gfx_shift;

	if (!use_shadow)
		shadow = 0;

	uint16_t *const dest = &bitmap.pix(screenline);

	int minx = cliprect.min_x << 16;
	int maxx = (cliprect.max_x + 1) << 16;

	if (xzoom < 0x10000) // shrink
	{
		int x0 = flipx ? (base_sx + (8 * xzoom) - xzoom) : (base_sx);
		int x1 = flipx ? (base_sx - xzoom) : (x0 + (8 * xzoom));
		const int dx = flipx ? (-xzoom) : (xzoom);

		const uint8_t* const source = flipy ? addr + (7 - line) * 8 : addr + line * 8;
		int column = 0;

		for (int sx = x0; sx != x1; sx += dx)
		{
			uint8_t pen = (source[column++] & gfx_mask) >> gfx_shift;


			if (sx >= minx && sx < maxx)
			{
				int realsx = sx >> 16;

				if (pen || opaque)
				{
					if (!shadow)
					{
						dest[realsx] = (realcolor + pen) & 0x7fff;
					}
					else
					{
						int pen_shift = 15 - shadow;
						int pen_mask = (1 << pen_shift) - 1;
						dest[realsx] = ((dest[realsx] & pen_mask) | (pen << pen_shift)) & 0x7fff;
					}
				}
			}
		}
	}
	else // enlarge or no zoom
	{
		const uint8_t* const source = flipy ? addr + (7 - line) * 8 : addr + line * 8;

		int x0 = (base_sx);
		int x1 = (x0 + (8 * xzoom));

		int column;
		if (!flipx)
		{
			column = 0;
		}
		else
		{
			column = 7;
		}

		uint32_t countx = 0;
		for (int sx = x0; sx < x1; sx += 0x10000)
		{
			uint8_t pen = (source[column] & gfx_mask) >> gfx_shift;

			if (sx >= minx && sx < maxx)
			{
				int realsx = sx >> 16;

				if (pen || opaque)
				{
					if (!shadow)
					{
						dest[realsx] = (realcolor + pen) & 0x7fff;
					}
					else
					{
						int pen_shift = 15 - shadow;
						int pen_mask = (1 << pen_shift) - 1;
						dest[realsx] = ((dest[realsx] & pen_mask) | (pen << pen_shift)) & 0x7fff;
					}
				}
			}

			countx += 0x10000;
			if (countx >= xzoom)
			{
				if (!flipx)
				{
					column++;
				}
				else
				{
					column--;
				}

				countx -= xzoom;
			}
		}
	}
}


// takes an x/y pixel position in the virtual tilemap and returns the code + attributes etc. for it
inline void seta2_state::get_tile(uint16_t* spriteram, int is_16x16, int x, int y, int page, int& code, int& attr, int& flipx, int& flipy, int& color)
{
	int xtile = x >> (is_16x16 ? 4 : 3);
	int ytile = y >> (is_16x16 ? 4 : 3);

	// yes the tilemap in RAM is flipped?!
	ytile ^= 0x1f;

	uint16_t *s3 = &spriteram[2 * ((page * 0x2000 / 4) + ((ytile & 0x1f) << 6) + ((xtile) & 0x03f))];
	attr = s3[0];
	code = s3[1] + ((attr & 0x0007) << 16);
	flipx = (attr & 0x0010);
	flipy = (attr & 0x0008);
	color = (attr & 0xffe0) >> 5;
	if (is_16x16)
	{
		code &= ~3;

		if (!flipx)
		{
			if (x & 8)
			{
				code += 1;
			}
		}
		else
		{
			if (!(x & 8))
			{
				code += 1;
			}
		}

		if (!flipy)
		{
			if (y & 8)
			{
				code += 2;
			}
		}
		else
		{
			if (!(y & 8))
			{
				code += 2;
			}
		}
	}
}

int seta2_state::calculate_global_xoffset(int nozoom_fixedpalette_fixedposition)
{
	/*
	int global_xoffset = (m_vregs[0x12/2] & 0x7ff); // and 0x10/2 for low bits
	if (global_xoffset & 0x400)
	    global_xoffset -= 0x800;

	// funcube3 sets a global xoffset of -1 causing a single pixel shift, does something else compensate for it?
	// note, it also writes a different address for the sprite buffering (related?) but doesn't also have the global zoom set to negative like Star Audition which also writes there.

	int global_xzoom = (m_vregs[0x16/2] & 0x7ff); // and 0x14/2 for low bits

	// HACK: this inverts the zoom on all sprites, thus flipping the screen and altering positions as the origin becomes the right hand side, not left, see star audition (by default) or deer hunting when you turn on horizontal flip
	// TODO: properly render negative zoom sprites
	if (global_xzoom & 0x400)
	{
	    global_xoffset -= 0x14f;
	}
	*/

	int global_xoffset = 0;

	if (nozoom_fixedpalette_fixedposition)
		global_xoffset = 0x80;

	return global_xoffset;
}

int seta2_state::calculate_global_yoffset(int nozoom_fixedpalette_fixedposition)
{
	// Sprites list
	//int global_yoffset = (m_vregs[0x1a / 2] & 0x7ff); // and 0x18/2 for low bits
	//if (global_yoffset & 0x400)
	//  global_yoffset -= 0x800;

	//global_yoffset += 1; // +2 for myangel / myangel2?
	int global_yoffset = 0;

	if (nozoom_fixedpalette_fixedposition)
		global_yoffset = -0x90;

	return global_yoffset;
}


void seta2_state::draw_sprites_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int scanline, int realscanline, int xoffset, uint32_t xzoom, bool xzoominverted)
{
	uint16_t* spriteram = m_spriteram;

	uint16_t *s1 = m_private_spriteram.get();

	int sprite_debug_count = 0;

	for (; s1 < &m_private_spriteram[0x1000 / 2]; s1 += 4,sprite_debug_count++)
	{
		int num = s1[0];

		int xoffs = s1[1];
		int yoffs = s1[2];
		int sprite = s1[3];

		// Single-sprite address
		uint16_t *s2 = &spriteram[(sprite & 0x7fff) * 4];
		uint16_t *end = &spriteram[m_spriteram.bytes() / 2];

		// Single-sprite size
		int global_sizex = xoffs & 0xfc00;
		int global_sizey = yoffs & 0xfc00;


		int nozoom_fixedpalette_fixedposition = num & 0x4000; // ignore various things including global offsets, zoom.  different palette selection too?
		bool opaque = num & 0x2000;
		int use_global_size = num & 0x1000;
		int use_shadow = num & 0x0800;
		int which_gfx = num & 0x0700;
		xoffs &= 0x3ff;
		yoffs &= 0x3ff;



		if (yoffs & 0x200)
			yoffs -= 0x400;

		int global_xoffset = calculate_global_xoffset(nozoom_fixedpalette_fixedposition);
		int global_yoffset = calculate_global_yoffset(nozoom_fixedpalette_fixedposition);
		int usedscanline;
		int usedxoffset;
		uint32_t usedxzoom;

		if (nozoom_fixedpalette_fixedposition)
		{
			use_shadow = 0;
		//  which_gfx = 4 << 8;
			usedscanline = realscanline; // no zooming?
			usedxzoom = 0x10000;
			usedxoffset = 0;
		}
		else
		{
			usedscanline = scanline;
			usedxzoom = xzoom;
			usedxoffset = xoffset;
		}

		// Number of single-sprites
		num = (num & 0x00ff) + 1;

		// all sprites, except invalid ones should have a pointer <0x3000 in the reformatted list
		if ((sprite&0x7fff) < 0x3000 / 2 / 4)
		{
			for (; num > 0; num--, s2 += 4)
			{
				if (s2 >= end)  break;

				if (sprite & 0x8000)
				{

					// "floating tilemap" sprite
					// the 'floating tilemap sprites' are just a window into the tilemap, the position of the sprite does not change the scroll values

					// get everything we need to calculate if sprite covers this scanline
					int sy = s2[1] & 0x3ff;
					sy += global_yoffset;
					sy &= 0x3ff;
					if (sy & 0x200)
						sy -= 0x400;

					int local_sizey = s2[1] & 0xfc00;

					int height = use_global_size ? global_sizey : local_sizey;
					height = ((height & 0xfc00) >> 10) + 1;

					int firstline = (sy + yoffs) & 0x3ff;

					if (firstline & 0x200)
						firstline -= 0x400;

					int endline = firstline + height * 0x10 - 1;

					// if the sprite doesn't cover this scanline, bail now

					if (endline & 0x200)
						endline -= 0x400;

					if (endline >= firstline)
					{
						if (firstline > usedscanline)    continue;
						if (endline < usedscanline)    continue;
					}
					else
					{
						// cases where the sprite crosses 0
						if ((usedscanline > endline) && (usedscanline < firstline))
							continue;
					}

					// get everything we need to calculate if sprite is actually within the x co-ordinates of the screen
					int sx = s2[0];
					int local_sizex = sx & 0xfc00;
					sx &= 0x3ff;
					sx -= global_xoffset;

					int width = use_global_size ? global_sizex : local_sizex;
					width = ((width & 0xfc00) >> 10)/* + 1*/; // reelquak reels
					if (!width)
						continue;

					int firstcolumn = (sx + xoffs);
					firstcolumn = (firstcolumn & 0x1ff) - (firstcolumn & 0x200);
					int lastcolumn = firstcolumn + width * 0x10 - 1;

					// if the sprite isn't within the x-coordinates of the screen, bail
					if (firstcolumn > cliprect.max_x)    continue;
					if (lastcolumn < cliprect.min_x)    continue;


					// otherwise get the rest of the things we need to draw
					int scrolly = s2[3];
					scrolly &= 0x1ff;
					scrolly += global_yoffset;
					int sourceline = (usedscanline - scrolly) & 0x1ff;

					int scrollx = s2[2];
					int is_16x16 = (scrollx & 0x8000) >> 15;
					int page = (scrollx & 0x7c00) >> 10;
					scrollx &= 0x3ff;

					// we treat 16x16 tiles as 4 8x8 tiles, so while the tilemap is 0x40 tiles wide in memory, that becomes 0x80 tiles in 16x16 mode, with the data wrapping in 8x8 mode
					for (int x = 0; x < 0x80; x++)
					{
						int code, attr, flipx, flipy, color;
						// tilemap data is NOT buffered?
						get_tile(spriteram, is_16x16, x * 8, sourceline, page, code, attr, flipx, flipy, color);

						int tileline = sourceline & 0x07;
						int dx = sx + (scrollx & 0x3ff) + xoffs + 0x10;
						int px = (((dx + x * 8) + 0x10) & 0x3ff) - 0x10;
						int dst_x = px & 0x3ff;
						dst_x = (dst_x & 0x1ff) - (dst_x & 0x200);

						if ((dst_x >= firstcolumn - 8) && (dst_x <= lastcolumn)) // reelnquak reels are heavily glitched without this check
						{
							uint32_t realsx = dst_x;
							realsx -= usedxoffset>>16; // need to refactor, this causes loss of lower 16 bits of offset which are important in zoomed cases for precision
							realsx = realsx * usedxzoom;
							drawgfx_line(bitmap, cliprect, which_gfx, m_spritegfx->get_data(m_realtilenumber[code]), color << 4, flipx, flipy, realsx, usedxzoom, use_shadow, realscanline, tileline, opaque);
						}
					}
				}
				else
				{
					// "normal" sprite
					int sy = s2[1] & 0x1ff;

					if (sy & 0x100)
						sy -= 0x200;

					sy += global_yoffset;
					sy &= 0x3ff;


					if (realscanline == 128)
					{
					//  printf("%04x %02x %d %d\n", sprite_debug_count, num, yoffs, sy);
					}

					int sizey = use_global_size ? global_sizey : s2[1] & 0xfc00;

					sizey = (1 << ((sizey & 0x0c00) >> 10)) - 1;

					int firstline = (sy + yoffs) & 0x3ff;
					int endline = (firstline + (sizey + 1) * 8) - 1;

					//firstline &= 0x3ff;
					endline &= 0x3ff;

					if (firstline & 0x200)
						firstline -= 0x400;

					if (endline & 0x200)
						endline -= 0x400;


					// if the sprite doesn't cover this scanline, bail now
					if (endline >= firstline)
					{
						if (firstline > usedscanline)    continue;
						if (endline < usedscanline)    continue;
					}
					else
					{
						// cases where the sprite crosses 0
						if ((usedscanline > endline) && (usedscanline < firstline))
							continue;
					}

					// otherwise get the rest of the things we need to draw
					int attr = s2[2];
					int code = s2[3] + ((attr & 0x0007) << 16);
					int flipx = (attr & 0x0010);
					int flipy = (attr & 0x0008);
					int color = (attr & 0xffe0) >> 5;

					int sx = s2[0];
					int sizex = use_global_size ? global_sizex : sx;
					sizex = (1 << ((sizex & 0x0c00) >> 10)) - 1;

					sx += xoffs;
					sx = (sx & 0x1ff) - (sx & 0x200);
					sx -= global_xoffset;

					int basecode = code &= ~((sizex + 1) * (sizey + 1) - 1);   // see myangel, myangel2 and grdians


					int line = usedscanline - firstline;
					int y = (line >> 3);
					line &= 0x7;

					if (nozoom_fixedpalette_fixedposition)
					{
						// grdians map...
						color = 0x7ff;
					}

					for (int x = 0; x <= sizex; x++)
					{
						int realcode = (basecode + (flipy ? sizey - y : y)*(sizex + 1)) + (flipx ? sizex - x : x);
						uint32_t realsx = (sx + x * 8);
						realsx -= usedxoffset>>16; // need to refactor, this causes loss of lower 16 bits of offset which are important in zoomed cases for precision
						realsx = realsx * usedxzoom;
						drawgfx_line(bitmap, cliprect, which_gfx, m_spritegfx->get_data(m_realtilenumber[realcode]), color << 4, flipx, flipy, realsx, usedxzoom, use_shadow, realscanline, line, opaque);
					}

				}
			}
		}
		if (s1[0] & 0x8000) break;  // end of list marker
	}   // sprite list
}


TIMER_CALLBACK_MEMBER(seta2_state::raster_timer_done)
{
	auto *tmp68301 = dynamic_cast<tmp68301_device *>(m_maincpu.target());
	if (tmp68301)
	{
		if (m_rasterenabled & 1)
		{
			tmp68301->external_interrupt_1();
			logerror("external int (vpos is %d)\n", m_screen->vpos());
			m_screen->update_partial(m_screen->vpos() - 1);
		}
	}
}


void seta2_state::draw_sprites(bitmap_ind16& bitmap, const rectangle& cliprect)
{
	//printf("yoffset: %04x%04x yzoom: %04x%04x | xoffset: %04x%04x xzoom: %04x%04x  \n", m_vregs[0x1a/2],  m_vregs[0x18/2],  m_vregs[0x1e/2],  m_vregs[0x1c/2]   ,   m_vregs[0x12/2],  m_vregs[0x10/2],  m_vregs[0x16/2],  m_vregs[0x14/2]);

	uint32_t yoffset = (m_vregs[0x1a / 2] << 16) | m_vregs[0x18 / 2];
	yoffset &= 0x07ffffff;
	yoffset = 0x07ffffff - yoffset;

	uint32_t yzoom = (m_vregs[0x1e / 2] << 16) | m_vregs[0x1c / 2];
	yzoom &= 0x07ffffff;
	bool yzoominverted = false;
	bool xzoominverted = false;

	if (yzoom & 0x04000000)
	{
		yzoom = 0x8000000 - yzoom;
		yzoominverted = true;
	}


	int xoffset = (m_vregs[0x12 / 2] << 16) | m_vregs[0x10 / 2];
	xoffset &= 0x07ffffff;

	if (xoffset & 0x04000000)
		xoffset -= 0x08000000;

	//xoffset = 0x07ffffff - xoffset;

	uint32_t xzoom = (m_vregs[0x16 / 2] << 16) | m_vregs[0x14 / 2];

	if (xzoom & 0x04000000)
	{
		xzoom = 0x8000000 - xzoom;
		xzoominverted = true;
	}


	if (!xzoom)
		return;

	uint64_t inc = 0x100000000ULL;

	uint32_t inc2 = inc / xzoom;

	//printf("xinc is %04x xoom %04x xoffset is %4x\n", inc2, xzoom, xoffset);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		rectangle tempcliprect(cliprect);

		tempcliprect.sety(y, y);

		int yy;

		if (!yzoominverted)
		{
			yy = y; // not handled yet (this is using negative yzoom to do flipscreen...)
		}
		else
		{
			yy = y * yzoom;
			yy += yoffset;
			yy &= 0x07ffffff;
			yy >>= 16;

		//  printf("line %04x yline requested %04x\n", y, yy);

			if (yy & 0x400)
				yy -= 0x800;

		}

		draw_sprites_line(bitmap, tempcliprect, yy, y, xoffset, inc2, xzoominverted);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

void seta2_state::video_start()
{
	for (int i = 0; m_gfxdecode->gfx(i); ++i)
		m_gfxdecode->gfx(i)->set_granularity(16);

	m_private_spriteram = make_unique_clear<uint16_t[]>(0x1000 / 2);

	m_realtilenumber = std::make_unique<uint32_t[]>(0x80000);

	m_spritegfx = m_gfxdecode->gfx(0);
	for (int i = 0; i < 0x80000; i++)
	{
		m_realtilenumber[i] = i % m_spritegfx->elements();
	}

	m_raster_timer = timer_alloc(FUNC(seta2_state::raster_timer_done), this);

	save_pointer(NAME(m_private_spriteram), 0x1000 / 2);
}

uint32_t seta2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Black or pen 0?
	bitmap.fill(m_palette->pen(0), cliprect);

	if ( (m_vregs[0x30/2] & 1) == 0 )   // 1 = BLANK SCREEN
		draw_sprites(bitmap, cliprect);

	return 0;
}

WRITE_LINE_MEMBER(seta2_state::screen_vblank)
{
	//popmessage("yoffset: %04x%04x yzoom: %04x%04x | xoffset: %04x%04x xzoom: %04x%04x  \n", m_vregs[0x1a/2],  m_vregs[0x18/2],  m_vregs[0x1e/2],  m_vregs[0x1c/2]   ,   m_vregs[0x12/2],  m_vregs[0x10/2],  m_vregs[0x16/2],  m_vregs[0x14/2]);
}

// staraudi
void staraudi_state::draw_rgbram(bitmap_ind16 &bitmap)
{
	if (!(m_cam & 0x0008))
		return;

	for (int y = 0x100; y < 0x200; ++y)
	{
		for (int x = 0; x < 0x200; ++x)
		{
			int offs = x * 2/2 + y * 0x400/2;
			uint32_t data = ((m_rgbram[offs + 0x40000/2] & 0xff) << 16) | m_rgbram[offs];
			bitmap.pix(y, x) = (data & 0x7fff);
		}
	}
}
uint32_t staraudi_state::staraudi_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update(screen, bitmap, cliprect);
	if (false)
		draw_rgbram(bitmap);

	return 0;
}
