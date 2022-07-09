// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*
    The MLC graphics hardware is quite complicated - the usual method of having 'object ram' that
    controls sprites is expanded into object ram that controls sprite blocks that may be stored
    in RAM or ROM.  Each tile in a block may be specified explicitly via a display list in ROM or
    calculated as part of a block offset.

    Blocks can be scaled and subpositioned, and are usually 4-6bpp but two 4bpp blocks can be
    combined into 8bpp with a flag.
*/

#include "emu.h"
#include "deco_mlc.h"

#include <algorithm>

/******************************************************************************/

void deco_mlc_state::video_start()
{
	int max_color = (0x800 / m_gfxdecode->gfx(0)->granularity());
	m_colour_mask = max_color - 1;
	m_shadow_mask = max_color | (max_color << 1);
	for (int s = 0; s <= 8; s++)
	{
		if (BIT(m_gfxdecode->gfx(0)->granularity(), s))
		m_shadow_shift = 11 - s;
	}

//  temp_bitmap = std::make_unique<bitmap_rgb32>(512, 512);
	m_buffered_spriteram = std::make_unique<u16[]>(0x3000 / 4);
	m_spriteram_spare = std::make_unique<u16[]>(0x3000 / 4);
	m_spriteram = std::make_unique<u16[]>(0x3000 / 4);

	save_pointer(NAME(m_spriteram), 0x3000 / 4);
	save_pointer(NAME(m_spriteram_spare), 0x3000 / 4);
	save_pointer(NAME(m_buffered_spriteram), 0x3000 / 4);
}


void deco_mlc_state::drawgfxzoomline(u32* dest, u8* pri,const rectangle &clip,gfx_element *gfx,
		u32 code1,u32 code2, u32 color,int flipx,int sx,
		int transparent_color,int use8bpp,
		int scalex, int srcline, int shadowMode )
{
	if (!scalex) return;
	const u32 alphaMode = m_irq_ram[0x04 / 4] & 0xc0;

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/

	/* KW 991012 -- Added code to force clip to bitmap boundary */

	const int sprite_screen_width = (scalex * 16 + (sx & 0xffff)) >> 16;

	sx >>= 16;
	if (sprite_screen_width)
	{
		/* compute sprite increment per screen pixel */
		int dx = (16 << 16) / sprite_screen_width;

		int ex = sx + sprite_screen_width;

		int x_index_base;

		if (flipx)
		{
			x_index_base = (sprite_screen_width - 1) * dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if (sx < clip.left())
		{ /* clip left */
			int pixels = clip.left() - sx;
			sx += pixels;
			x_index_base += pixels * dx;
		}
		/* NS 980211 - fixed incorrect clipping */
		if (ex > clip.right() + 1)
		{ /* clip right */
			int pixels = ex-clip.right() - 1;
			ex -= pixels;
		}

		if (ex>sx)
		{ /* skip if inner loop doesn't draw anything */
			const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const u8 *code_base1 = gfx->get_data(code1 % gfx->elements());
			const u8 *code_base2 = gfx->get_data(code2 % gfx->elements());
			const u8 *source1 = code_base1 + (srcline) * gfx->rowbytes();
			const u8 *source2 = code_base2 + (srcline) * gfx->rowbytes();
			// alphaMode & 0xc0 = 0xc0 : Shadow, 0 : Alpha or Pre-shadowed, Other bits unknown
			if (shadowMode && (alphaMode & 0xc0))
			{   /* TODO : 8bpp and shadow can use simultaneously? */
				int x, x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					if (!(pri[x] & 0x80))
					{
						int c = source1[x_index >> 16];
						if (use8bpp)
							c = (c << 4) | source2[x_index >> 16];

						if (c != transparent_color)
							pri[x] |= shadowMode; // Mark it shadow / hilighted
					}
					x_index += dx;
				}
			}
			else
			{
				int x, x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					if (!(pri[x] & 0x80))
					{
						int c = source1[x_index >> 16];
						if (use8bpp)
							c = (c << 4) | source2[x_index >> 16];

						if (c != transparent_color)
						{
							dest[x] = pal[c];
							pri[x] |= 0x80 | shadowMode; // Mark it drawn
						}
					}
					x_index += dx;
				}
			}
		}
	}
}


void deco_mlc_state::draw_sprites(const rectangle &cliprect, int scanline, u32* dest, u8* pri)
{
	u32 *index_ptr = nullptr;
	int sprite,h,w,fx1,fy1;
	int xoffs,yoffs;
	const u8 *index_ptr8;
	int blockIsTilemapIndex = 0;
	int sprite2 = 0,use8bppMode = 0;
	int useIndicesInRom = 0;
	int hibits = 0;
	int tileFormat = 0;

	rectangle user_clip;
	const u16* spriteram = m_buffered_spriteram.get();

	//printf("%d - (%08x %08x %08x) (%08x %08x %08x) (%08x %08x %08x)\n", scanline, m_irq_ram[6], m_irq_ram[7], m_irq_ram[8], m_irq_ram[9], m_irq_ram[10], m_irq_ram[11] , m_irq_ram[12] , m_irq_ram[13] , m_irq_ram[14]);

	for (int offs = 0; offs < (0x3000 / 4); offs += 8)
	{
		/* If this bit is set, combine this block with the next one */
		use8bppMode = (offs + 8 < (0x3000 / 4)) && (spriteram[offs + 8 + 1] & 0x1000) && (spriteram[offs + 8 + 0] & 0x8000);
		if (use8bppMode)
			offs += 8;

		if ((spriteram[offs + 0] & 0x8000) == 0)
			continue;
		if ((spriteram[offs + 1] & 0x2000) && (m_screen->frame_number() & 1))
			continue;

		/*
		    Spriteram (1) format (16 bit):

		    Word 0: 0x8000 - Sprite enable
		            0x4000 - Use ROM or RAM for spriteram 2 (really top bit of index)
		            0x3fff - Index into spriteram 2
		    Word 1: 0x8000 - X flip
		            0x4000 - Y flip
		            0x2000 - Auto-flicker (display sprite only every other frame)
		            0x1000 - If set combine this 4bpp sprite & next one, into 8bpp sprite
		            0x0800 - upper clip bit (stadhr96 view of bases)
		            0x0400 - Use raster IRQ lookup table when drawing object - or not? (OK for stadhr96, NOT enabled for avengrgs)
		            0x0300 - lower clipping window to use (swapped) - and upper bits of raster select (stadhr96)
		            0x0080 - seems to be both the lower bit of the raster select (stadhr96) AND the upper bit of colour / alpha (avngrgs?) - might depend on other bits?
		            0x007f - Colour/alpha shadow enable
		    Word 2: 0x07ff - Y position
		    Word 3: 0x07ff - X position
		    Word 4: 0x03ff - X scale
		    Word 5: 0x03ff - Y scale

		    Spriteram (2) format (16 bit):

		    Word 0: 0xe000 - ? (Always 0x2000?)
		            0x1000 - X flip
		            0x0f00 - Width in tiles (0==16)
		            0x00ff - X position offset
		    Word 1: 0xe000 - ? (Always 0x2000?)
		            0x1000 - Y flip
		            0x0f00 - Height in tiles (0==16)
		            0x00ff - Y position offset
		    Word 2: 0xfe00 - ? (Always 0?)
		            0x0100 - Autoflicker
		            0x00c0 - If set use tile index as pointer into tile index array, else use as tile index directly
		            0x0080 - If set tile index array format is 12 bit tile, 4 bit colour
		            0x0040 - If set tile index array is 16 bit tile, 0 bit colour
		            0x003c - Hi-bits of tile index after array lookup
		            0x0003 - Hi-bits of tile index, selects ROM or RAM for array
		    Word 3: 0xffff - Low-bits of tile index
		*/

		int y = spriteram[offs + 2] & 0x7ff;
		int x = spriteram[offs + 3] & 0x7ff;

		if (x & 0x400) x = -(0x400 - (x & 0x3ff));
		if (y & 0x400) y = -(0x400 - (y & 0x3ff));

		int fx = spriteram[offs + 1] & 0x8000;
		int fy = spriteram[offs + 1] & 0x4000;
		int color = spriteram[offs + 1] & 0xff;

		const int raster_select = (spriteram[offs + 1] & 0x0180)>>7;

		// there are 3 different sets of raster values, must be a way to selects
		// between them? furthermore avengrgs doesn't even enable this
		// although it doesn't seem to set the scroll values very often either
		// so the irq mechanism might be wrong
		//
		// actually avengrgs has our current clipper&1 set on the areas that should
		// have the scroll effect applied to them.  all clip windows are the same
		// so there is no reason to select a clip window other than to be using it
		// to select a set of raster-set scroll regs?
		int rasterMode = (spriteram[offs + 1] >> 10) & 0x1;

		int clipper = (spriteram[offs + 1] >> 8) & 0x3;

		int indx = spriteram[offs + 0] & 0x7fff;
		int yscale = spriteram[offs + 4] & 0x3ff;
		int xscale = spriteram[offs + 5] & 0x3ff;
		int colorOffset = 0;

		/* Clip windows - this mapping seems odd, but is correct for Skull Fang and StadHr96,
		however there are space for 8 clipping windows, where is the high bit? (Or is it ~0x400?) */
		clipper = ((clipper & 2) >> 1) | ((clipper & 1) << 1); // Swap low two bits

		int upperclip = (spriteram[offs + 1] >> 10) & 0x2;
		// this is used on some ingame gfx in stadhr96
		// to clip the images of your guys on the bases
		if (upperclip)
			clipper |= 0x4;

		int min_y = m_clip_ram[(clipper * 4) + 0];
		int max_y = m_clip_ram[(clipper * 4) + 1];

		if ((scanline < min_y) || (scanline > max_y))
			continue;

		user_clip.setx(m_clip_ram[(clipper * 4) + 2], m_clip_ram[(clipper * 4) + 3]);
		user_clip &= cliprect;

		/* Any colours out of range (for the bpp value) trigger 'shadow' mode */
		int shadowMode = (color & m_shadow_mask) >> m_shadow_shift; // shadow mode (OK for skullfng)

		color &= m_colour_mask;

		/* In 8bpp the palette base is stored in the next block */
		if (use8bppMode)
		{
			color = (spriteram[offs + 1 - 8] & 0x7f);
		}

		/* Lookup tiles / size in sprite index ram OR in the lookup rom */
		if (indx & 0x4000)
		{
			index_ptr8 = m_gfx2 + indx * 8; /* Byte ptr */

			if (index_ptr8[5] & 0x01 && m_screen->frame_number() & 1)
				continue;

			h = (index_ptr8[1] >> 0) & 0xf;
			w = (index_ptr8[3] >> 0) & 0xf;

			if (!h) h = 16;
			if (!w) w = 16;

			sprite = (index_ptr8[7] << 8) | index_ptr8[6];
			sprite |= (index_ptr8[4] & 3) << 16;

			yoffs = index_ptr8[0] & 0xff;
			xoffs = index_ptr8[2] & 0xff;

			fy1 = (index_ptr8[1] & 0x10) >> 4;
			fx1 = (index_ptr8[3] & 0x10) >> 4;

			tileFormat = index_ptr8[4] & 0x80;

			if (index_ptr8[4] & 0xc0)
				blockIsTilemapIndex = 1;
			else
				blockIsTilemapIndex = 0;

			useIndicesInRom = 0;
		}
		else
		{
			indx &= 0x1fff;
			index_ptr = m_vram + indx * 4;

			if (index_ptr[2] & 0x0100 && m_screen->frame_number() & 1)
				continue;

			h = (index_ptr[0] >> 8) & 0xf;
			w = (index_ptr[1] >> 8) & 0xf;

			if (!h) h = 16;
			if (!w) w = 16;

			sprite = ((index_ptr[2] & 0x3) << 16) | (index_ptr[3] & 0xffff);
			if (index_ptr[2] & 0xc0)
				blockIsTilemapIndex = 1;
			else
				blockIsTilemapIndex = 0;

			tileFormat = index_ptr[2] & 0x80;

			hibits = (index_ptr[2] & 0x3c) << 10;
			useIndicesInRom = index_ptr[2] & 3;

			yoffs = index_ptr[0] & 0xff;
			xoffs = index_ptr[1] & 0xff;

			fy1 = (index_ptr[0] & 0x1000) >> 12;
			fx1 = (index_ptr[1] & 0x1000) >> 12;
		}

		if (use8bppMode)
		{
			indx = spriteram[offs + 0 - 8] & 0x7fff;
			if (indx & 0x4000)
			{
				index_ptr8 = m_gfx2 + indx * 8;
				sprite2 = (index_ptr8[7] << 8) | index_ptr8[6];
			}
			else
			{
				indx &= 0x1fff;
				index_ptr = m_vram + indx * 4;
				sprite2 = ((index_ptr[2] & 0x3) << 16) | (index_ptr[3] & 0xffff);
			}
		}

		if (fx1) fx ^= 0x8000;
		if (fy1) fy ^= 0x4000;

		int extra_x_scale = 0x100;

		// I think we need some hardware tests..
		//  see notes about how this can't be our enable register (avengrgs doesn't touch it
		//  and relies on something else, probably just the bits we use to select the window)
		//  (although as previously noted, it isn't writing valid per-scanline values either)
		if (rasterMode)
		{
				// use of these is a bit weird.  (it's probably just 16-bit .. like spriteram so the upper dupes are ignored?)
				// -ZZZ -yyy   ---- -xxx   -YYY -zzz

				// xxx = x offset?
				// yyy = y offset?
				// zzz = xzoom (confirmed? stadium hero)
				//  0x100 = no zoom
				//
				// YYY = duplicate bits of yyy?
				// ZZZ = (sometimes) duplicate bits of zzz

			if (raster_select == 1 || raster_select == 2 || raster_select == 3)
			{
				//shadowMode = 0; // TODO : Raster enable and shadow mode can use simultaneously?
				int irq_base_reg; /* 6, 9, 12  are possible */
				if (raster_select == 1) irq_base_reg = 6;    // OK upper screen.. left?
				else if (raster_select == 2) irq_base_reg = 9; // OK upper screen.. main / center
				else irq_base_reg = 12;

				int extra_y_off = m_irq_ram[irq_base_reg + 0] & 0x7ff;
				int extra_x_off = m_irq_ram[irq_base_reg + 1] & 0x7ff;
				extra_x_scale = (m_irq_ram[irq_base_reg + 2] >> 0) & 0x3ff;

				if (extra_x_off & 0x400) { extra_x_off -= 0x800; }
				if (extra_y_off & 0x400) { extra_y_off -= 0x800; }

				x += extra_x_off;
				y += extra_y_off;
			}
			else if (raster_select == 0x0)
			{
				// possibly disabled?
			}
		}

		const int yscale_frac = (yscale << 8);
		const int real_h = h * 16;

		xscale *= extra_x_scale;

		int ybase = y << 16;
		const int yinc = yscale_frac * 16;

		if (fy)
			ybase += (yoffs - 15) * yscale_frac - ((h - 1) * yinc);
		else
			ybase -= yoffs * yscale_frac;

		int xbase = x << 16;
		const int xinc = xscale * 16;

		if (fx)
			xbase += (xoffs - 15) * xscale - ((w - 1) * xinc);
		else
			xbase -= xoffs * xscale;

		const int full_realybase = ybase;
		const int full_sprite_screen_height = (yscale_frac * real_h + 0);
		const int full_sprite_screen_height_unscaled = (1 * real_h + 0);

		if (!full_sprite_screen_height_unscaled)
			continue;

		const int ratio = full_sprite_screen_height / full_sprite_screen_height_unscaled;

		if (!ratio)
			continue;

		const int bby = scanline - (full_realybase >> 16);

		if (bby < 0)
			continue;

//      color = machine().rand();

		int srcline = ((bby << 16) / ratio);

		const int by = srcline >> 4;

		if (by >= h)
			continue;

		srcline &= 0xf;
		if (fy)
		{
			srcline = 15 - srcline;
		}

		for (int bx = 0; bx < w; bx++)
		{
			const int realxbase = xbase + bx * xinc;
			int count = 0;
			if (fx)
			{
				if (fy)
					count = (h - 1 - by) * w + (w - 1 - bx);
				else
					count = by * w + (w - 1 - bx);
			}
			else
			{
				if (fy)
					count = (h - 1 - by) * w + bx;
				else
					count = by * w + bx;
			}

			int tile = sprite + count;
			int tile2 = sprite2 + count;

			if (blockIsTilemapIndex)
			{
				if (useIndicesInRom)
				{
					const u8* ptr = m_gfx2 + (tile * 2);
					tile = (*ptr) + ((*(ptr + 1)) << 8);

					if (use8bppMode)
					{
						const u8* ptr2 = m_gfx2 + (tile2 * 2);
						tile2 = (*ptr2) + ((*(ptr2 + 1)) << 8);
					}
					else
					{
						tile2 = 0;
					}

					if (tileFormat)
					{
						colorOffset = (tile & 0xf000) >> 12;
						tile = (tile & 0x0fff) | hibits;
						tile2 = (tile2 & 0x0fff) | hibits;
					}
					else
					{
						colorOffset = 0;
						tile = (tile & 0xffff) | (hibits << 2);
						tile2 = (tile2 & 0xffff) | (hibits << 2);
					}
				}
				else
				{
					const u32* ptr = m_vram + (tile & 0x7fff);
					tile = (*ptr) & 0xffff;

					if (tileFormat)
					{
						colorOffset = (tile & 0xf000) >> 12;
						tile = (tile & 0x0fff) | hibits;
					}
					else
					{
						colorOffset = 0;
						tile = (tile & 0xffff) | (hibits << 2);
					}

					tile2 = 0;
				}
			}

			drawgfxzoomline(dest,pri,user_clip,m_gfxdecode->gfx(0),
							tile,tile2,
							color + colorOffset,fx,realxbase,
							0,
							use8bppMode,xscale, srcline,
							shadowMode);

		}
	}
}

WRITE_LINE_MEMBER(deco_mlc_state::screen_vblank_mlc)
{
	// rising edge
	if (state)
	{
		/* Spriteram is definitely double buffered, as the vram lookup tables
		are often updated a frame after spriteram is setup to point to a new
		lookup table.  Without buffering incorrect one frame glitches are seen
		in several places, especially in Hoops.
		*/
		std::copy(&m_spriteram[0], &m_spriteram[0x3000 / 4], &m_buffered_spriteram[0]);
	}
}

u32 deco_mlc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
//  temp_bitmap->fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0), cliprect); /* Pen 0 fill colour confirmed from Skull Fang level 2 */

	for (int i = cliprect.top(); i <= cliprect.bottom(); i++)
	{
		u32 *dest = &bitmap.pix(i);
		u8 *pri = &screen.priority().pix(i);

		/*
		printf("%d -", i);
		for (int j = 0; j < 0x20; j++)
		{
		    printf("%08x, ",m_irq_ram[j]);
		}
		printf("\n");
		*/
		draw_sprites(cliprect, i, dest, pri);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u32 col = dest[x];
			const u8 shadow_flag = pri[x] & 3;
			if ((shadow_flag & 3) && (shadow_flag != 3))
			{
				if (shadow_flag & 1) // shadow
					col = (col & 0xff000000) | ((col & 0xfefefe) >> 1);
				else if (shadow_flag & 2) // hilight
					col = (col & 0xff000000) | ((col & 0xfefefe) >> 1) | 0x808080;

				dest[x] = col;
			}
		}
	}
	return 0;
}
