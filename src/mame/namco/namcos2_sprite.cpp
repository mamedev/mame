// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
    Namco System 2 Sprites - found on Namco System 2 video board (standard type)

    based on namcoic.txt this probably consists of the following
    C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
    C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
    C135 - Checks is object is displayed on Current output line.
    C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

    Metal Hawk requires a different draw function, so might use a different chip unless the hookup
    is just scrambled (needs checking).

    "Shadow" sprites are used in the baseball games to remap the background tile pen, and they're
    also used a lot in valkyrie.

    Device used by the following drivers:
    namcos2.cpp (all games EXCEPT Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours,
    Suzuka 8 Hours 2 which use the newer Namco NB1 style sprites, see namco_c355spr.cpp).

*/

#include "emu.h"
#include "namcos2_sprite.h"

DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE, namcos2_sprite_device, "namcos2_sprite", "Namco System 2 Sprites (C106,C134,C135,C146)")
DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE_FINALLAP, namcos2_sprite_finallap_device, "namcos2_sprite_finallap", "Namco System 2 Sprites (C106,C134,C135,C146) (Final Lap)")
DEFINE_DEVICE_TYPE(NAMCOS2_SPRITE_METALHAWK, namcos2_sprite_metalhawk_device, "namcos2_sprite_metalhawk", "Namco System 2 Sprites (C106,C134,C135,C146) (Metal Hawk)")

namcos2_sprite_device::namcos2_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE, tag, owner, clock)
{
}

namcos2_sprite_device::namcos2_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_gfxdecode(*this, finder_base::DUMMY_TAG),
	m_spriteram(*this, finder_base::DUMMY_TAG)
{
}


namcos2_sprite_metalhawk_device::namcos2_sprite_metalhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE_METALHAWK, tag, owner, clock)
{
}

namcos2_sprite_finallap_device::namcos2_sprite_finallap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namcos2_sprite_device(mconfig, NAMCOS2_SPRITE_FINALLAP, tag, owner, clock)
{
}

void namcos2_sprite_device::device_start()
{
}

/**************************************************************************************/

/**************************************************************************************/

void namcos2_sprite_device::zdrawgfxzoom(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		u32 code,u32 color,bool flipx,bool flipy,int sx,int sy,
		int scalex, int scaley, int zpos)
{
	if (!scalex || !scaley) return;
	if (dest_bmp.bpp() == 16)
	{
		if (gfx)
		{
			device_palette_interface &palette = gfx->palette();
			const pen_t *pal = &palette.pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const u8 *source_base = gfx->get_data(code % gfx->elements());
			const int sprite_screen_height = (scaley * gfx->height() + 0x8000) >> 16;
			const int sprite_screen_width = (scalex * gfx->width() + 0x8000) >> 16;
			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width() << 16) / sprite_screen_width;
				int dy = (gfx->height() << 16) / sprite_screen_height;

				int ex = sx + sprite_screen_width;
				int ey = sy + sprite_screen_height;

				int x_index_base;
				int y_index;

				if (flipx)
				{
					x_index_base = (sprite_screen_width - 1) * dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if (flipy)
				{
					y_index = (sprite_screen_height - 1) * dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if (sx < clip.min_x)
				{ /* clip left */
					int pixels = clip.min_x - sx;
					sx += pixels;
					x_index_base += pixels * dx;
				}
				if (sy < clip.min_y)
				{ /* clip top */
					int pixels = clip.min_y - sy;
					sy += pixels;
					y_index += pixels * dy;
				}
				if (ex > clip.max_x + 1)
				{ /* clip right */
					int pixels = ex - clip.max_x - 1;
					ex -= pixels;
				}
				if (ey > clip.max_y + 1)
				{ /* clip bottom */
					int pixels = ey - clip.max_y - 1;
					ey -= pixels;
				}

				if (ex > sx)
				{ /* skip if inner loop doesn't draw anything */
					bitmap_ind8 &priority_bitmap = screen.priority();
					if (priority_bitmap.valid())
					{
						for (int y = sy; y < ey; y++)
						{
							u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
							u16 *const dest = &dest_bmp.pix(y);
							u8 *const pri = &priority_bitmap.pix(y);
							int x_index = x_index_base;
							/* this code was previously shared with the c355 where this was needed
							if (m_palxor)
							{
							    for (int x = sx; x < ex; x++)
							    {
							        const u8 c = source[x_index >> 16];
							        if (c != 0xff)
							        {
							            if (pri[x] <= zpos)
							            {
							                switch(c)
							                {
							                case 0:
							                    dest[x] = 0x4000 | (dest[x] & 0x1fff);
							                    break;
							                case 1:
							                    dest[x] = 0x6000 | (dest[x] & 0x1fff);
							                    break;
							                default:
							                    dest[x] = pal[c];
							                    break;
							                }
							                pri[x] = zpos;
							            }
							        }
							        x_index += dx;
							    }
							    y_index += dy;
							}
							else
							*/
							{
								for (int x = sx; x < ex; x++)
								{
									const u8 c = source[x_index >> 16];
									if (c != 0xff)
									{
										if (pri[x] <= zpos)
										{
											if (color == 0xf && c == 0xfe)
											{
												if (dest[x] & 0x1000)
													dest[x] |= 0x800;
												else
													dest[x] = palette.black_pen();
											}
											else
											{
												dest[x] = pal[c];
											}
											pri[x] = zpos;
										}
									}
									x_index += dx;
								}
								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
} /* zdrawgfxzoom */

void namcos2_sprite_device::zdrawgfxzoom(
		screen_device &screen,
		bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		u32 code,u32 color,bool flipx,bool flipy,int sx,int sy,
		int scalex, int scaley, int zpos)
{
	/* nop */
}

void namcos2_sprite_device::get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32)
{
	sprn = (word1 >> 2) & 0x0fff;
	is_32 = bool(word0 & 0x200);
}

void namcos2_sprite_finallap_device::get_tilenum_and_size(const u16 word0, const u16 word1, u32 &sprn, bool &is_32)
{
	// Final Lap schematics show an older sprite board with lower capacity
	// and the 32/16 pixel mode select on a different bit
	// this is needed for the title screen to look correct, in addition to various in game sparks effects etc.

	sprn = (word1 >> 2) & 0x07ff;
	is_32 = bool((word1 >> 2) & 0x800);
}

void namcos2_sprite_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control)
{
	int offset = (control & 0x000f) * (128 * 4);
	if (pri == 0)
	{
		screen.priority().fill(0, cliprect);
	}
	for (int loop = 0; loop < 128; loop++)
	{
		/****************************************
		* word#0
		*   Sprite Y position           D00-D08
		*   Sprite Size 16/32           D09
		*   Sprite Size Y               D10-D15
		*
		* word#1
		*   Sprite Quadrant             D00-D01
		*   Sprite Number               D02-D13
		*   Sprite flip X               D14
		*   Sprite flip Y               D15
		*
		* word#2
		*   Sprite X position           D00-D10
		*
		* word#3
		*   Sprite priority             D00-D02
		*   Sprite colour index         D04-D07
		*   Sprite Size X               D10-D15
		*/
		const u16 word3 = m_spriteram[offset + (loop * 4) + 3];
		if ((word3 & 0xf) == pri)
		{
			const u16 word0   = m_spriteram[offset + (loop * 4) + 0];
			const u16 word1   = m_spriteram[offset + (loop * 4) + 1];
			const u16 offset4 = m_spriteram[offset + (loop * 4) + 2];
			const int sizey   = ((word0 >> 10) & 0x003f) + 1;

			u32 sprn;
			bool is_32;

			get_tilenum_and_size(word0, word1, sprn, is_32);

			int sizex = (word3 >> 10) & 0x003f;
			if (!is_32) sizex >>= 1;

			if ((sizey - 1) && sizex)
			{
				const u32 color  = (word3 >> 4) & 0x000f;
				const int ypos   = (0x1ff - (word0 & 0x01ff)) - 0x50 + 0x02;
				const int xpos   = (offset4 & 0x07ff) - 0x50 + 0x07;
				const bool flipy = word1 & 0x8000;
				const bool flipx = word1 & 0x4000;
				const int scalex = (sizex << 16) / (is_32 ? 0x20 : 0x10);
				const int scaley = (sizey << 16) / (is_32 ? 0x20 : 0x10);
				if (scalex && scaley)
				{
					gfx_element *gfx = m_gfxdecode->gfx(0);

					if (!is_32)
						gfx->set_source_clip((word1 & 0x0001) ? 16 : 0, 16, (word1 & 0x0002) ? 16 : 0, 16);
					else
						gfx->set_source_clip(0, 32, 0, 32);

					zdrawgfxzoom(
						screen,
						bitmap,
						cliprect,
						gfx,
						sprn,
						color,
						flipx,flipy,
						xpos,ypos,
						scalex,scaley,
						loop);
				}
			}
		}
	}
} /* draw_sprites */

void namcos2_sprite_metalhawk_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control)
{
	/**
	 * word#0
	 *  xxxxxx---------- ysize
	 *  ------x--------- sprite tile size
	 *  -------xxxxxxxxx screeny
	 *
	 * word#1
	 *  --xxxxxxxxxxxxxx tile
	 *
	 * word#2 (unused)
	 *
	 * word#3
	 *  xxxxxx---------- xsize
	 *  ------xxxxxxxxxx screenx
	 *
	 * word#4 (unused)
	 * word#5 (unused)
	 *
	 * word#6 (orientation)
	 *  ---------------x rot90
	 *  --------------x- flipx
	 *  -------------x-- flipy
	 *  ------------x--- tile size
	 *
	 * word#7
	 *  ------------xxxx priority
	 *  --------xxxx---- color
	 *  x--------------- unknown
	 */
	const u16 *pSource = m_spriteram;
	if (pri == 0)
	{
		screen.priority().fill(0, cliprect);
	}
	for (int loop=0; loop < 128; loop++)
	{
		const u16 ypos  = pSource[0];
		const u16 tile  = pSource[1];
		const u16 xpos  = pSource[3];
		const u16 flags = pSource[6];
		const u16 attrs = pSource[7];
		const int sizey = ((ypos >> 10) & 0x003f) + 1;
		const int sizex =  (xpos >> 10) & 0x003f;
		const u32 sprn  =  (tile >>  2) & 0x0fff;

		if ((sizey - 1) && sizex && (attrs & 0xf) == pri)
		{
			const bool bBigSprite = flags & 0x0008;
			const u32 color  = (attrs >> 4) & 0x000f;
			int sx           = (xpos & 0x03ff) - 0x50 + 0x07;
			int sy           = (0x1ff - (ypos & 0x01ff)) - 0x50 + 0x02;
			const bool flipx = flags & 0x0002;
			const bool flipy = flags & 0x0004;
			const int scalex = (sizex << 16) / (0x20);//(sizex << 16) / (bBigSprite ? 0x20 : 0x10); correct formula?
			const int scaley = (sizey << 16) / (bBigSprite ? 0x20 : 0x10);

			/* swap xy */
			const int rgn = (flags & 0x0001);

			gfx_element *gfx = m_gfxdecode->gfx(rgn);

			if (bBigSprite)
			{
				if (sizex < 0x20)
				{
					sx -= (0x20 - sizex) / 0x8;
				}
				if (sizey < 0x20)
				{
					sy += (0x20 - sizey) / 0xc;
				}
				gfx->set_source_clip(0, 32, 0, 32);
			}
			else
				gfx->set_source_clip((tile & 0x0001) ? 16 : 0, 16, (tile & 0x0002) ? 16 : 0, 16);

			zdrawgfxzoom(
				screen,
				bitmap,
				cliprect,
				gfx,
				sprn, color,
				flipx,flipy,
				sx,sy,
				scalex, scaley,
				loop);
		}
		pSource += 8;
	}
} /* draw_sprites_metalhawk */
