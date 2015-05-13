// license:???
// copyright-holders:Michael Soderstrom, Marc LaFontaine, Aaron Giles
/***************************************************************************

    Williams 6809 system

****************************************************************************

    The basic video system involves a 4-bit-per-pixel bitmap, oriented
    in inverted X/Y order. That is, pixels (0,0) and (1,0) come from the
    byte at offset 0. Pixels (2,0) and (3,0) come from the byte at offset
    256. Pixels (4,0) and (5,0) come from the byte at offset 512. Etc.

    Defender and Stargate simply draw graphics to the framebuffer directly
    with no extra intervention.

    Later games added a pair of "special chips" (SC-01) to the board which
    are special purpose blitters. During their operation they HALT the
    main CPU so that they can control the busses. The operation of the
    chips is described in detail below.

    The original SC-01 had a bug that forced an XOR of the width and height
    values with 4. This was fixed in the SC-02, which was used on  several
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
    the data to move, you have to exclusive-or the width and height with 2.
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

***************************************************************************/

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


VIDEO_START_MEMBER(williams_state,williams)
{
	blitter_init(m_blitter_config, NULL);
	create_palette_lookup();
	state_save_register();
}


VIDEO_START_MEMBER(blaster_state,blaster)
{
	blitter_init(m_blitter_config, memregion("proms")->base());
	create_palette_lookup();
	state_save_register();
	save_item(NAME(m_blaster_color0));
	save_item(NAME(m_blaster_video_control));
}


VIDEO_START_MEMBER(williams2_state,williams2)
{
	blitter_init(m_blitter_config, NULL);

	/* create the tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(williams2_state::get_tile_info),this), TILEMAP_SCAN_COLS,  24,16, 128,16);
	m_bg_tilemap->set_scrolldx(2, 0);

	state_save_register();
	save_item(NAME(m_tilemap_xscroll));
	save_item(NAME(m_williams2_fg_color));
}



/*************************************
 *
 *  Williams video update
 *
 *************************************/

UINT32 williams_state::screen_update_williams(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];
	int x, y;

	/* precompute the palette */
	for (x = 0; x < 16; x++)
		pens[x] = m_palette_lookup[m_generic_paletteram_8[x]];

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT8 *source = &m_videoram[y];
		UINT32 *dest = &bitmap.pix32(y);

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


UINT32 blaster_state::screen_update_blaster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];
	int x, y;

	/* precompute the palette */
	for (x = 0; x < 16; x++)
		pens[x] = m_palette_lookup[m_generic_paletteram_8[x]];

	/* if we're blitting from the top, start with a 0 for color 0 */
	if (cliprect.min_y == screen.visible_area().min_y || !(m_blaster_video_control & 1))
		m_blaster_color0 = m_palette_lookup[m_blaster_palette_0[0] ^ 0xff];

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int erase_behind = m_blaster_video_control & m_blaster_scanline_control[y] & 2;
		UINT8 *source = &m_videoram[y];
		UINT32 *dest = &bitmap.pix32(y);

		/* latch a new color0 pen? */
		if (m_blaster_video_control & m_blaster_scanline_control[y] & 1)
			m_blaster_color0 = m_palette_lookup[m_blaster_palette_0[y] ^ 0xff];

		/* loop over columns */
		for (x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			int pix = source[(x/2) * 256];

			/* clear behind us if requested */
			if (erase_behind)
				source[(x/2) * 256] = 0;

			/* now draw */
			dest[x+0] = (pix & 0xf0) ? pens[pix >> 4] : rgb_t(m_blaster_color0 | pens[0]);
			dest[x+1] = (pix & 0x0f) ? pens[pix & 0x0f] : rgb_t(m_blaster_color0 | pens[0]);
		}
	}
	return 0;
}


UINT32 williams2_state::screen_update_williams2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t pens[16];
	int x, y;

	/* draw the background */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* fetch the relevant pens */
	for (x = 1; x < 16; x++)
		pens[x] = m_palette->pen_color(m_williams2_fg_color * 16 + x);

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT8 *source = &m_videoram[y];
		UINT32 *dest = &bitmap.pix32(y);

		/* loop over columns */
		for (x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
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

void williams_state::create_palette_lookup()
{
	static const int resistances_rg[3] = { 1200, 560, 330 };
	static const int resistances_b[2]  = { 560, 330 };
	double weights_r[3], weights_g[3], weights_b[2];
	int i;

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the relative resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_r, 0, 0,
			3, resistances_rg, weights_g, 0, 0,
			2, resistances_b,  weights_b, 0, 0);

	/* build a palette lookup */
	m_palette_lookup = auto_alloc_array(machine(), rgb_t, 256);
	for (i = 0; i < 256; i++)
	{
		int r = combine_3_weights(weights_r, BIT(i,0), BIT(i,1), BIT(i,2));
		int g = combine_3_weights(weights_g, BIT(i,3), BIT(i,4), BIT(i,5));
		int b = combine_2_weights(weights_b, BIT(i,6), BIT(i,7));

		m_palette_lookup[i] = rgb_t(r, g, b);
	}
}


WRITE8_MEMBER(williams2_state::williams2_paletteram_w)
{
	static const UINT8 ztable[16] =
	{
		0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,  0x9,
		0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11
	};
	UINT8 entry_lo, entry_hi, i, r, g, b;

	/* set the new value */
	m_generic_paletteram_8[offset] = data;

	/* pull the associated low/high bytes */
	entry_lo = m_generic_paletteram_8[offset & ~1];
	entry_hi = m_generic_paletteram_8[offset |  1];

	/* update the palette entry */
	i = ztable[(entry_hi >> 4) & 15];
	b = ((entry_hi >> 0) & 15) * i;
	g = ((entry_lo >> 4) & 15) * i;
	r = ((entry_lo >> 0) & 15) * i;
	m_palette->set_pen_color(offset / 2, rgb_t(r, g, b));
}


WRITE8_MEMBER(williams2_state::williams2_fg_select_w)
{
	m_williams2_fg_color = data & 0x3f;
}



/*************************************
 *
 *  Video position readout
 *
 *************************************/

READ8_MEMBER(williams_state::williams_video_counter_r)
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
	int data = m_williams2_tileram[tile_index];
	int y = (tile_index >> 1) & 7;
	int color = 0;

	switch (m_williams2_tilemap_config)
	{
		case WILLIAMS_TILEMAP_MYSTICM:
		{
			/* IC79 is a 74LS85 comparator that controls the low bit */
			int a = 1 | ((color & 1) << 2) | ((color & 1) << 3);
			int b = ((y & 6) >> 1);
			int casc = (y & 1);
			color = (a > b) || ((a == b) && !casc);
			break;
		}

		case WILLIAMS_TILEMAP_TSHOOT:
			/* IC79 is a 74LS157 selector jumpered to be enabled */
			color = y;
			break;

		case WILLIAMS_TILEMAP_JOUST2:
			/* IC79 is a 74LS157 selector jumpered to be disabled */
			color = 0;
			break;
	}

	SET_TILE_INFO_MEMBER(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}


WRITE8_MEMBER(williams2_state::williams2_bg_select_w)
{
	/* based on the tilemap config, only certain bits are used */
	/* the rest are determined by other factors */
	switch (m_williams2_tilemap_config)
	{
		case WILLIAMS_TILEMAP_MYSTICM:
			/* IC79 is a 74LS85 comparator that controls the low bit */
			data &= 0x3e;
			break;

		case WILLIAMS_TILEMAP_TSHOOT:
			/* IC79 is a 74LS157 selector jumpered to be enabled */
			data &= 0x38;
			break;

		case WILLIAMS_TILEMAP_JOUST2:
			/* IC79 is a 74LS157 selector jumpered to be disabled */
			data &= 0x3f;
			break;
	}
	m_bg_tilemap->set_palette_offset(data * 16);
}


WRITE8_MEMBER(williams2_state::williams2_tileram_w)
{
	m_williams2_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(williams2_state::williams2_xscroll_low_w)
{
	m_tilemap_xscroll = (m_tilemap_xscroll & ~0x00f) | ((data & 0x80) >> 4) | (data & 0x07);
	m_bg_tilemap->set_scrollx(0, (m_tilemap_xscroll & 7) + ((m_tilemap_xscroll >> 3) * 6));
}


WRITE8_MEMBER(williams2_state::williams2_xscroll_high_w)
{
	m_tilemap_xscroll = (m_tilemap_xscroll & 0x00f) | (data << 4);
	m_bg_tilemap->set_scrollx(0, (m_tilemap_xscroll & 7) + ((m_tilemap_xscroll >> 3) * 6));
}



/*************************************
 *
 *  Blaster-specific enhancements
 *
 *************************************/

WRITE8_MEMBER(blaster_state::blaster_remap_select_w)
{
	m_blitter_remap_index = data;
	m_blitter_remap = m_blitter_remap_lookup + data * 256;
}


WRITE8_MEMBER(blaster_state::blaster_video_control_w)
{
	m_blaster_video_control = data;
}



/*************************************
 *
 *  Blitter setup and control
 *
 *************************************/

void williams_state::blitter_init(int blitter_config, const UINT8 *remap_prom)
{
	static const UINT8 dummy_table[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
	int i,j;

	/* by default, there is no clipping window - this will be touched only by games that have one */
	m_blitter_window_enable = 0;

	/* switch off the video config */
	m_blitter_xor = (blitter_config == WILLIAMS_BLITTER_SC01) ? 4 : 0;

	/* create the remap table; if no PROM, make an identity remap table */
	m_blitter_remap_lookup = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_blitter_remap_index = 0;
	m_blitter_remap = m_blitter_remap_lookup;
	for (i = 0; i < 256; i++)
	{
		const UINT8 *table = remap_prom ? (remap_prom + (i & 0x7f) * 16) : dummy_table;
		for (j = 0; j < 256; j++)
			m_blitter_remap_lookup[i * 256 + j] = (table[j >> 4] << 4) | table[j & 0x0f];
	}
}


WRITE8_MEMBER(williams_state::williams_blitter_w)
{
	int sstart, dstart, w, h, accesses;
	int estimated_clocks_at_4MHz;

	/* store the data */
	m_blitterram[offset] = data;

	/* only writes to location 0 trigger the blit */
	if (offset != 0)
		return;

	/* compute the starting locations */
	sstart = (m_blitterram[2] << 8) + m_blitterram[3];
	dstart = (m_blitterram[4] << 8) + m_blitterram[5];

	/* compute the width and height */
	w = m_blitterram[6] ^ m_blitter_xor;
	h = m_blitterram[7] ^ m_blitter_xor;

	/* adjust the width and height */
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	/* do the actual blit */
	accesses = blitter_core(space, sstart, dstart, w, h, data);

	/* based on the number of memory accesses needed to do the blit, compute how long the blit will take */
	if(data & WMS_BLITTER_CONTROLBYTE_SLOW)
	{
		estimated_clocks_at_4MHz = 4 + 4 * (accesses + 2);
	}
	else
	{
		estimated_clocks_at_4MHz = 4 + 2 * (accesses + 3);
	}

	space.device().execute().adjust_icount(-((estimated_clocks_at_4MHz + 3) / 4));

	/* Log blits */
	logerror("%04X:Blit @ %3d : %02X%02X -> %02X%02X, %3dx%3d, mask=%02X, flags=%02X, icount=%d, win=%d\n",
			space.device().safe_pc(), m_screen->vpos(),
			m_blitterram[2], m_blitterram[3],
			m_blitterram[4], m_blitterram[5],
			m_blitterram[6], m_blitterram[7],
			m_blitterram[1], m_blitterram[0],
			((estimated_clocks_at_4MHz + 3) / 4), m_blitter_window_enable);
}


WRITE8_MEMBER(williams2_state::williams2_blit_window_enable_w)
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
	int source, sxadv, syadv;
	int dest, dxadv, dyadv;
	int x, y;
	int accesses = 0;

	/* compute how much to advance in the x and y loops */
	sxadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256) ? 0x100 : 1;
	syadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256) ? 1 : w;
	dxadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256) ? 0x100 : 1;
	dyadv = (controlbyte & WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256) ? 1 : w;

	int pixdata=0;

	/* loop over the height */
	for (y = 0; y < h; y++)
	{
		source = sstart & 0xffff;
		dest = dstart & 0xffff;

		/* loop over the width */
		for (x = 0; x < w; x++)
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
