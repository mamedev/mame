// Video System Sprites
// todo:
//  move various vsystem sprite functions here
//  unify common ones + convert to device

#include "emu.h"

// zooming is wrong for 3on3dunk ... suprslam implementation is probably better
void draw_sprites_inufuku( UINT16* spriteram1, int spriteram1_bytes, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	int end = 0;

	for (offs = 0; offs < (spriteram1_bytes / 16 ); offs++)
	{
		if (spriteram1[offs] & 0x4000) break;
	}
	end = offs;

	for (offs = end - 1; offs >= 0; offs--)
	{
		if ((spriteram1[offs] & 0x8000) == 0x0000)
		{
			int attr_start;
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
			int priority, priority_mask;

			attr_start = 4 * (spriteram1[offs] & 0x03ff);

			/*
                attr_start + 0x0000
                ---- ---x xxxx xxxx oy
                ---- xxx- ---- ---- ysize
                xxxx ---- ---- ---- zoomy

                attr_start + 0x0001
                ---- ---x xxxx xxxx ox
                ---- xxx- ---- ---- xsize
                xxxx ---- ---- ---- zoomx

                attr_start + 0x0002
                -x-- ---- ---- ---- flipx
                x--- ---- ---- ---- flipy
                --xx xxxx ---- ---- color
                --xx ---- ---- ---- priority?
                ---- ---- xxxx xxxx unused?

                attr_start + 0x0003
                -xxx xxxx xxxx xxxx map start
                x--- ---- ---- ---- unused?
            */

			ox = (spriteram1[attr_start + 1] & 0x01ff) + 0;
			xsize = (spriteram1[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (spriteram1[attr_start + 1] & 0xf000) >> 12;
			oy = (spriteram1[attr_start + 0] & 0x01ff) + 1;
			ysize = (spriteram1[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (spriteram1[attr_start + 0] & 0xf000) >> 12;
			flipx = spriteram1[attr_start + 2] & 0x4000;
			flipy = spriteram1[attr_start + 2] & 0x8000;
			color = (spriteram1[attr_start + 2] & 0x3f00) >> 8;
			priority = (spriteram1[attr_start + 2] & 0x3000) >> 12;
			map_start = (spriteram1[attr_start + 3] & 0x7fff) << 1;

			switch (priority)
			{
				default:
				case 0:	priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = (oy + zoomy * (ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (oy + zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = (ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (ox + zoomx * x / 2 + 16) & 0x1ff;

					code  = ((spriteram2[map_start] & 0x0007) << 16) + spriteram2[map_start + 1];

					pdrawgfxzoom_transpen(bitmap, cliprect, machine.gfx[2],
							code,
							color,
							flipx, flipy,
							sx - 16, sy - 16,
							zoomx << 11, zoomy << 11,
							machine.priority_bitmap,priority_mask, 15);

					map_start += 2;
				}
			}
		}
	}
}



/* todo, fix zooming correctly, it's _not_ like aerofgt */
void draw_sprites_suprslam( UINT16* spriteram, int spriteram_bytes, UINT16* sp_videoram, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* SPRITE INFO

    Video System hardware, like aerofgt etc.

    the sprites use 2 areas of ram, one containing a spritelist + sprite attributes, the other
    contains the sprite tile #'s to use

    sprite attribute info (4 words per sprite)

    |  ZZZZ hhhy yyyy yyyy  |  zzzz wwwx xxxx xxxx  |  -fpp pppp ---- ----  |  -ooo oooo oooo oooo  |

    x  = x position
    y  = y position
    w  = width
    h  = height
    zZ = y zoom / x zoom
    f  = xflip
    p  = palette / colour
    o  = offset to tile data in other ram area

    */

	gfx_element *gfx = machine.gfx[1];
	UINT16 *source = spriteram;
	UINT16 *source2 = spriteram;
	UINT16 *finish = source + 0x2000/2;

	while (source < finish)
	{
		UINT32 sprnum = source[0] & 0x03ff;
		if (source[0] == 0x4000) break;

		sprnum *= 4;

		source++;
		/* DRAW START */
		{
			int ypos = source2[sprnum + 0] & 0x1ff;
			int high = (source2[sprnum + 0] & 0x0e00) >> 9;
			int yzoom = (source2[sprnum + 0] & 0xf000) >> 12;

			int xpos = source2[sprnum + 1] & 0x1ff;
			int wide = (source2[sprnum + 1] & 0x0e00) >> 9;
			int xzoom = (source2[sprnum + 1] & 0xf000) >> 12;

			int col = (source2[sprnum + 2] & 0x3f00) >> 8;
			int flipx = (source2[sprnum + 2] & 0x4000) >> 14;
//          int flipy = (source2[sprnum + 2] & 0x8000) >> 15;

			int word_offset = source2[sprnum + 3] & 0x7fff;
			int xcnt, ycnt;

			int loopno = 0;

			xzoom = 32 - xzoom;
			yzoom = 32 - yzoom;

			if (ypos > 0xff) ypos -=0x200;

			for (ycnt = 0; ycnt < high+1; ycnt ++)
			{
				if (!flipx)
			{
					for (xcnt = 0; xcnt < wide+1; xcnt ++)
					{
						int tileno = sp_videoram[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						loopno ++;
					}
				}
				else
				{
					for (xcnt = wide; xcnt >= 0; xcnt --)
					{
						int tileno = sp_videoram[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						loopno ++;
					}
				}
			}
		}
	}
}
