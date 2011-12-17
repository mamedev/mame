/*************************************************************************

    System1 / System 2
    original driver by Jarek Parchanski & Mirko Buffoni

    Many thanks to Roberto Ventura, for precious information about
    System 1 hardware.

**************************************************************************

    The System 1/System 2 video hardware is composed of two tilemap
    layers and a sprite layer.

    The tilemap layers are built up out of "pages" of 32x32 tilemaps.
    Each tile is described by two bytes, meaning each page is 2k bytes
    in size. One of the tilemaps is fixed in position, while the other
    has registers for scrolling that vary between board variants.

    The original System 1 hardware simply had two fixed pages. Page 0
    was the scrolling tilemap, and page 1 was the fixed tilemap.

    With later boards and the introduction of System 2, this was
    expanded to support up to 8 pages. The fixed tilemap was hard-
    coded to page 0, but the scrolling tilemap was extended. Instead
    of a single page, the scrolling tilemap consisted of 4 pages glued
    together to form an effective large 64x64 tilemap. Further, each
    of the 4 pages that made up the scrolling tilemap could be
    independently selected from one of the 8 available pages. This
    unique paged tilemap system would continue on to form the basis of
    Sega's tilemap systems for their 16-bit era.

    Up to 32 sprites can be displayed. They are rendered one scanline
    ahead of the beam into 12-bit line buffers which store the sprite
    pixel data and sprite index. During rendering, collisions are
    checked between sprites and if one is found a bit is set in a
    special 32x32x1 collision RAM indiciating which pair of sprites
    collided. Note that the sprite color is derived directly from the
    sprite index, giving each sprite its own set of 16 colors.

    The 11-bit output from the two tilemaps (3 bits of pixel data,
    6 bits of color, 2 bits of priority), plus the 9-bit output from
    the sprite line buffer (4 bits of pixel data, 5 bits of color)
    are combined in a final step to produce the final pixel value. To
    do this, a lookup PROM is used which accepts as input the priority
    bits from the two tilemaps and the whether each of the incoming
    pixel values is transparent (color 0).

    The output of the lookup PROM is a 4-bit value. The lower 2 bits
    select sprite data (0), fixed tilemap (1) or scrolling tilemap (2).
    9 bits of data from the appropriate source are used as a lookup
    into a palette RAM, and the lookup PROM's low 2 bits are used as
    the upper 2 bits of the palette RAM address, providing 512
    independent colors for each source.

    The upper 2 bits of the lookup PROM are used for an additional
    mixer collision detection. Bit 2 indicates that a collision
    should be recorded, and bit 3 indicates which of two banks of
    collision flags should be set. Each bank is 32 entries long, and
    the sprite index is used to select which bit within the bank to
    set.

    On the original System 1 hardware, the palette RAM value was used
    directly as RGB, with 3 bits each of red and green, and 2 bits of
    blue. Later hardware added an extra indirection layer, where the
    8-bit palette RAM value passed into 3 256x4 palette PROMs, one for
    each color.

    Collision data is accessed via a 4k window that is broken into
    4 equal-sized sections. The first section returns data from the
    2x32x1 mixer collision; the data for the collision is returned in
    D0, and a summary bit indicating that some sort of collision has
    occurred is returned in D7. The specific collision bit is cleared
    by writing to the equivalent address in the same region. The
    collision summary bit is cleared by writing to the second region.

    The third and fourth collision regions operate similarly, but
    return data for the 32x32x1 sprite collisions.

**************************************************************************

    TODO:
    - Sprite vs background alignment is off sometimes, best visible when
      scrolling, eg. in regulus, brain. Yet it is correct in other games,
      such as wboy.
    - not sure if sprite priorities are completely accurate

*************************************************************************/

#include "emu.h"
#include "includes/system1.h"


/*************************************
 *
 *  Tile callback
 *
 *************************************/

static TILE_GET_INFO( tile_get_info )
{
	const UINT8 *rambase = (const UINT8 *)param;
	UINT32 tiledata = rambase[tile_index*2+0] | (rambase[tile_index*2+1] << 8);
	UINT32 code = ((tiledata >> 4) & 0x800) | (tiledata & 0x7ff);
	UINT32 color = (tiledata >> 5) & 0xff;

	SET_TILE_INFO(0, code, color, 0);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static void video_start_common(running_machine &machine, int pagecount)
{
	system1_state *state = machine.driver_data<system1_state>();
	int pagenum;

	/* allocate memory for the collision arrays */
	state->m_mix_collide = auto_alloc_array_clear(machine, UINT8, 64);
	state->m_sprite_collide = auto_alloc_array_clear(machine, UINT8, 1024);

	/* allocate memory for videoram */
	state->m_tilemap_pages = pagecount;
	state->m_videoram = auto_alloc_array_clear(machine, UINT8, 0x800 * pagecount);

	/* create the tilemap pages */
	for (pagenum = 0; pagenum < pagecount; pagenum++)
	{
		state->m_tilemap_page[pagenum] = tilemap_create(machine, tile_get_info, tilemap_scan_rows, 8,8, 32,32);
		tilemap_set_transparent_pen(state->m_tilemap_page[pagenum], 0);
		tilemap_set_user_data(state->m_tilemap_page[pagenum], state->m_videoram + 0x800 * pagenum);
	}

	/* allocate a temporary bitmap for sprite rendering */
	state->m_sprite_bitmap = auto_bitmap_alloc(machine, 512, 256, BITMAP_FORMAT_INDEXED16);

	/* register for save stats */
	state_save_register_global(machine, state->m_video_mode);
	state_save_register_global(machine, state->m_mix_collide_summary);
	state_save_register_global(machine, state->m_sprite_collide_summary);
	state->save_pointer(NAME(state->m_videoram), 0x800 * pagecount);
	state_save_register_global_pointer(machine, state->m_mix_collide, 64);
	state_save_register_global_pointer(machine, state->m_sprite_collide, 1024);
}


VIDEO_START( system1 )
{
	video_start_common(machine, 2);
}


VIDEO_START( system2 )
{
	video_start_common(machine, 8);
}



/*************************************
 *
 *  Video control
 *
 *************************************/

WRITE8_HANDLER( system1_videomode_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	if (data & 0x6e) logerror("videomode = %02x\n",data);

	/* bit 4 is screen blank */
	state->m_video_mode = data;

	/* bit 7 is flip screen */
	flip_screen_set(space->machine(), data & 0x80);
}



/*************************************
 *
 *  Mixer collision I/O
 *
 *************************************/

READ8_HANDLER( system1_mixer_collision_r )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	return state->m_mix_collide[offset & 0x3f] | 0x7e | (state->m_mix_collide_summary << 7);
}

WRITE8_HANDLER( system1_mixer_collision_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	state->m_mix_collide[offset & 0x3f] = 0;
}

WRITE8_HANDLER( system1_mixer_collision_reset_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	state->m_mix_collide_summary = 0;
}



/*************************************
 *
 *  Sprite collision I/O
 *
 *************************************/

READ8_HANDLER( system1_sprite_collision_r )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	return state->m_sprite_collide[offset & 0x3ff] | 0x7e | (state->m_sprite_collide_summary << 7);
}

WRITE8_HANDLER( system1_sprite_collision_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	state->m_sprite_collide[offset & 0x3ff] = 0;
}

WRITE8_HANDLER( system1_sprite_collision_reset_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	space->machine().primary_screen->update_now();
	state->m_sprite_collide_summary = 0;
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

INLINE void videoram_wait_states(cpu_device *cpu)
{
	/* The main Z80's CPU clock is halted whenever an access to VRAM happens,
       and is only restarted by the FIXST signal, which occurs once every
       'n' pixel clocks. 'n' is determined by the horizontal control PAL. */

	/* this assumes 4 5MHz pixel clocks per FIXST, or 8*4 20MHz CPU clocks,
       and is based on a dump of 315-5137 */
	const UINT32 cpu_cycles_per_fixst = 4 * 4;
	const UINT32 fixst_offset = 2 * 4;
	UINT32 cycles_until_next_fixst = cpu_cycles_per_fixst - ((cpu->total_cycles() - fixst_offset) % cpu_cycles_per_fixst);

	device_adjust_icount(cpu, -cycles_until_next_fixst);
}

READ8_HANDLER( system1_videoram_r )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	UINT8 *videoram = state->m_videoram;
	videoram_wait_states(space->machine().firstcpu);
	offset |= 0x1000 * ((state->m_videoram_bank >> 1) % (state->m_tilemap_pages / 2));
	return videoram[offset];
}

WRITE8_HANDLER( system1_videoram_w )
{
	system1_state *state = space->machine().driver_data<system1_state>();
	UINT8 *videoram = state->m_videoram;
	videoram_wait_states(space->machine().firstcpu);
	offset |= 0x1000 * ((state->m_videoram_bank >> 1) % (state->m_tilemap_pages / 2));
	videoram[offset] = data;

	tilemap_mark_tile_dirty(state->m_tilemap_page[offset / 0x800], (offset % 0x800) / 2);

	/* force a partial update if the page is changing */
	if (state->m_tilemap_pages > 2 && offset >= 0x740 && offset < 0x748 && offset % 2 == 0)
		space->machine().primary_screen->update_now();
}

WRITE8_DEVICE_HANDLER( system1_videoram_bank_w )
{
	system1_state *state = device->machine().driver_data<system1_state>();
	state->m_videoram_bank = data;
}



/*************************************
 *
 *  Palette RAM access
 *
 *************************************/

WRITE8_HANDLER( system1_paletteram_w )
{
	const UINT8 *color_prom = space->machine().region("palette")->base();
	int val,r,g,b;

	/*
      There are two kind of color handling: in the System 1 games, values in the
      palette RAM are directly mapped to colors with the usual BBGGGRRR format;
      in the System 2 ones (Choplifter, WBML, etc.), the value in the palette RAM
      is a lookup offset for three palette PROMs in RRRRGGGGBBBB format.

      It's hard to tell for sure because they use resistor packs, but here's
      what I think the values are from measurment with a volt meter:

      Blue: .250K ohms
      Blue: .495K ohms
      Green:.250K ohms
      Green:.495K ohms
      Green:.995K ohms
      Red:  .495K ohms
      Red:  .250K ohms
      Red:  .995K ohms

      accurate to +/- .003K ohms.
    */

	space->machine().generic.paletteram.u8[offset] = data;

	if (color_prom != NULL)
	{
		int bit0,bit1,bit2,bit3;

		val = color_prom[data+0*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		val = color_prom[data+1*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		val = color_prom[data+2*256];
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		bit3 = (val >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
	}
	else
	{
		r = pal3bit(data >> 0);
		g = pal3bit(data >> 3);
		b = pal2bit(data >> 6);
	}

	palette_set_color(space->machine(),offset,MAKE_RGB(r,g,b));
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int xoffset)
{
	system1_state *state = machine.driver_data<system1_state>();
	UINT32 gfxbanks = machine.region("sprites")->bytes() / 0x8000;
	const UINT8 *gfxbase = machine.region("sprites")->base();
	UINT8 *spriteram = state->m_spriteram;
	int flipscreen = flip_screen_get(machine);
	int spritenum;

	/* up to 32 sprites total */
	for (spritenum = 0; spritenum < 32; spritenum++)
	{
		const UINT8 *spritedata = &spriteram[spritenum * 0x10];
		UINT16 srcaddr = spritedata[6] + (spritedata[7] << 8);
		UINT16 stride = spritedata[4] + (spritedata[5] << 8);
		UINT8 bank = ((spritedata[3] & 0x80) >> 7) | ((spritedata[3] & 0x40) >> 5) | ((spritedata[3] & 0x20) >> 3);
		int xstart = ((spritedata[2] | (spritedata[3] << 8)) & 0x1ff) + xoffset;
		int bottom = spritedata[1] + 1;
		int top = spritedata[0] + 1;
		UINT16 palettebase = spritenum * 0x10;
		const UINT8 *gfxbankbase;
		int x, y, i;

		/* writing an 0xff into the first byte of sprite RAM seems to disable all sprites;
           not sure if this applies to each sprite or only to the first one; see pitfall2
           and wmatch for examples where this is done */
		if (spritedata[0] == 0xff)
			return;

		/* clamp the bank to the size of the sprite ROMs */
		bank %= gfxbanks;
		gfxbankbase = gfxbase + bank * 0x8000;

		/* flip sprites vertically */
		if (flipscreen)
		{
			int temp = top;
			top = 256 - bottom;
			bottom = 256 - temp;
		}

		/* iterate over all rows of the sprite */
		for (y = top; y < bottom; y++)
		{
			UINT16 *destbase = BITMAP_ADDR16(bitmap, y, 0);
			UINT16 curaddr;
			int addrdelta;

			/* advance by the row counter */
			srcaddr += stride;

			/* skip if outside of our clipping area */
			if (y < cliprect->min_y || y > cliprect->max_y)
				continue;

			/* iterate over X */
			addrdelta = (srcaddr & 0x8000) ? -1 : 1;
			for (x = xstart, curaddr = srcaddr; ; x += 4, curaddr += addrdelta)
			{
				UINT8 color1, color2;
				UINT8 data;

				data = gfxbankbase[curaddr & 0x7fff];

				/* non-flipped case */
				if (!(curaddr & 0x8000))
				{
					color1 = data >> 4;
					color2 = data & 0x0f;
				}
				else
				{
					color1 = data & 0x0f;
					color2 = data >> 4;
				}

				/* stop when we see color 0x0f */
				if (color1 == 0x0f)
					break;

				/* draw if non-transparent */
				if (color1 != 0)
				{
					for (i = 0; i < 2; i++)
					{
						int effx = flipscreen ? 0x1fe - (x + i) : (x + i);
						if (effx >= cliprect->min_x && effx <= cliprect->max_x)
						{
							int prevpix = destbase[effx];
	
							if ((prevpix & 0x0f) != 0)
								state->m_sprite_collide[((prevpix >> 4) & 0x1f) + 32 * spritenum] = state->m_sprite_collide_summary = 1;
							destbase[effx] = color1 | palettebase;
						}
					}
				}

				/* stop when we see color 0x0f */
				if (color2 == 0x0f)
					break;

				/* draw if non-transparent */
				if (color2 != 0)
				{
					for (i = 0; i < 2; i++)
					{
						int effx = flipscreen ? 0x1fe - (x + 2 + i) : (x + 2 + i);
						if (effx >= cliprect->min_x && effx <= cliprect->max_x)
						{
							int prevpix = destbase[effx];
	
							if ((prevpix & 0x0f) != 0)
								state->m_sprite_collide[((prevpix >> 4) & 0x1f) + 32 * spritenum] = state->m_sprite_collide_summary = 1;
							destbase[effx] = color2 | palettebase;
						}
					}
				}
			}
		}
	}
}



/*************************************
 *
 *  Generic update code
 *
 *************************************/

static void video_update_common(device_t *screen, bitmap_t *bitmap, const rectangle *cliprect, bitmap_t *fgpixmap, bitmap_t **bgpixmaps, const int *bgrowscroll, int bgyscroll, int spritexoffs)
{
	system1_state *state = screen->machine().driver_data<system1_state>();
	const UINT8 *lookup = screen->machine().region("proms")->base();
	int x, y;

	/* first clear the sprite bitmap and draw sprites within this area */
	bitmap_fill(state->m_sprite_bitmap, cliprect, 0);
	draw_sprites(screen->machine(), state->m_sprite_bitmap, cliprect, spritexoffs);

	/* iterate over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *fgbase = BITMAP_ADDR16(fgpixmap, y & 0xff, 0);
		UINT16 *sprbase = BITMAP_ADDR16(state->m_sprite_bitmap, y & 0xff, 0);
		UINT16 *dstbase = BITMAP_ADDR16(bitmap, y, 0);
		int bgy = (y + bgyscroll) & 0x1ff;
		int bgxscroll = bgrowscroll[y / 8];
		UINT16 *bgbase[2];

		/* get the base of the left and right pixmaps for the effective background Y */
		bgbase[0] = BITMAP_ADDR16(bgpixmaps[(bgy >> 8) * 2 + 0], bgy & 0xff, 0);
		bgbase[1] = BITMAP_ADDR16(bgpixmaps[(bgy >> 8) * 2 + 1], bgy & 0xff, 0);

		/* iterate over pixels */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			int bgx = ((x - bgxscroll) / 2) & 0x1ff;
			UINT16 fgpix = fgbase[x / 2];
			UINT16 bgpix = bgbase[bgx >> 8][bgx & 0xff];
			UINT16 sprpix = sprbase[x];
			UINT8 lookup_index;
			UINT8 lookup_value;

			/* using the sprite, background, and foreground pixels, look up the color behavior */
			lookup_index =	(((sprpix & 0xf) == 0) << 0) |
							(((fgpix & 7) == 0) << 1) |
							(((fgpix >> 9) & 3) << 2) |
							(((bgpix & 7) == 0) << 4) |
							(((bgpix >> 9) & 3) << 5);
			lookup_value = lookup[lookup_index];

			/* compute collisions based on two of the PROM bits */
			if (!(lookup_value & 4))
				state->m_mix_collide[((lookup_value & 8) << 2) | ((sprpix >> 4) & 0x1f)] = state->m_mix_collide_summary = 1;

			/* the lower 2 PROM bits select the palette and which pixels */
			lookup_value &= 3;
			if (state->m_video_mode & 0x10)
				dstbase[x] = 0;
			else if (lookup_value == 0)
				dstbase[x] = 0x000 | (sprpix & 0x1ff);
			else if (lookup_value == 1)
				dstbase[x] = 0x200 | (fgpix & 0x1ff);
			else
				dstbase[x] = 0x400 | (bgpix & 0x1ff);
		}
	}
}



/*************************************
 *
 *  Board-specific update front-ends
 *
 *************************************/

SCREEN_UPDATE( system1 )
{
	system1_state *state = screen->machine().driver_data<system1_state>();
	UINT8 *videoram = state->m_videoram;
	bitmap_t *bgpixmaps[4], *fgpixmap;
	int bgrowscroll[32];
	int xscroll, yscroll;
	int y;

	/* all 4 background pages are the same, fixed to page 0 */
	bgpixmaps[0] = bgpixmaps[1] = bgpixmaps[2] = bgpixmaps[3] = tilemap_get_pixmap(state->m_tilemap_page[0]);

	/* foreground is fixed to page 1 */
	fgpixmap = tilemap_get_pixmap(state->m_tilemap_page[1]);

	/* get fixed scroll offsets */
	xscroll = (INT16)((videoram[0xffc] | (videoram[0xffd] << 8)) + 28);
	yscroll = videoram[0xfbd];

	/* adjust for flipping */
	if (flip_screen_get(screen->machine()))
	{
		xscroll = 640 - (xscroll & 0x1ff);
		yscroll = 764 - (yscroll & 0x1ff);
	}

	/* fill in the row scroll table */
	for (y = 0; y < 32; y++)
		bgrowscroll[y] = xscroll;

	/* common update */
	video_update_common(screen, bitmap, cliprect, fgpixmap, bgpixmaps, bgrowscroll, yscroll, 0);
	return 0;
}


SCREEN_UPDATE( system2 )
{
	system1_state *state = screen->machine().driver_data<system1_state>();
	UINT8 *videoram = state->m_videoram;
	bitmap_t *bgpixmaps[4], *fgpixmap;
	int rowscroll[32];
	int xscroll, yscroll;
	int sprxoffset;
	int y;

	/* 4 independent background pages */
	bgpixmaps[0] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x740] & 7]);
	bgpixmaps[1] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x742] & 7]);
	bgpixmaps[2] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x744] & 7]);
	bgpixmaps[3] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x746] & 7]);

	/* foreground is fixed to page 0 */
	fgpixmap = tilemap_get_pixmap(state->m_tilemap_page[0]);

	/* get scroll offsets */
	if (!flip_screen_get(screen->machine()))
	{
		xscroll = ((videoram[0x7c0] | (videoram[0x7c1] << 8)) & 0x1ff) - 512 + 10;
		yscroll = videoram[0x7ba];
		sprxoffset = 14;
	}
	else
	{
		xscroll = 512 + 512 + 10 - (((videoram[0x7f6] | (videoram[0x7f7] << 8)) & 0x1ff) - 512 + 10);
		yscroll = 512 + 512 - videoram[0x784];
		sprxoffset = -14;
	}

	/* fill in the row scroll table */
	for (y = 0; y < 32; y++)
		rowscroll[y] = xscroll;

	/* common update */
	video_update_common(screen, bitmap, cliprect, fgpixmap, bgpixmaps, rowscroll, yscroll, sprxoffset);
	return 0;
}


SCREEN_UPDATE( system2_rowscroll )
{
	system1_state *state = screen->machine().driver_data<system1_state>();
	UINT8 *videoram = state->m_videoram;
	bitmap_t *bgpixmaps[4], *fgpixmap;
	int rowscroll[32];
	int yscroll;
	int sprxoffset;
	int y;

	/* 4 independent background pages */
	bgpixmaps[0] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x740] & 7]);
	bgpixmaps[1] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x742] & 7]);
	bgpixmaps[2] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x744] & 7]);
	bgpixmaps[3] = tilemap_get_pixmap(state->m_tilemap_page[videoram[0x746] & 7]);

	/* foreground is fixed to page 0 */
	fgpixmap = tilemap_get_pixmap(state->m_tilemap_page[0]);

	/* get scroll offsets */
	if (!flip_screen_get(screen->machine()))
	{
		for (y = 0; y < 32; y++)
			rowscroll[y] = ((videoram[0x7c0 + y * 2] | (videoram[0x7c1 + y * 2] << 8)) & 0x1ff) - 512 + 10;

		yscroll = videoram[0x7ba];
		sprxoffset = 14;
	}
	else
	{
		for (y = 0; y < 32; y++)
			rowscroll[y] = 512 + 512 + 10 - (((videoram[0x7fe - y * 2] | (videoram[0x7ff - y * 2] << 8)) & 0x1ff) - 512 + 10);

		yscroll = 512 + 512 - videoram[0x784];
		sprxoffset = -14;
	}

	/* common update */
	video_update_common(screen, bitmap, cliprect, fgpixmap, bgpixmaps, rowscroll, yscroll, sprxoffset);
	return 0;
}
