/* Super Slams - video, see notes in driver file */

#include "driver.h"
#include "video/konamiic.h"


UINT16 *suprslam_screen_videoram, *suprslam_bg_videoram,*suprslam_sp_videoram, *suprslam_spriteram;
static UINT16 screen_bank, bg_bank;
static tilemap *suprslam_screen_tilemap, *suprslam_bg_tilemap;

/* todo, fix zooming correctly, its _not_ like aerofgt */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
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


	const gfx_element *gfx = machine->gfx[1];
	UINT16 *source = suprslam_spriteram;
	UINT16 *source2 = suprslam_spriteram;
	UINT16 *finish = source + 0x2000/2;

	while( source<finish )
	{
		UINT32 sprnum = source[0] & 0x03ff;
		if (source[0] == 0x4000) break;

		sprnum *= 4;

		source++;
		/* DRAW START */
		{
			int ypos = source2[sprnum+0] & 0x1ff;
			int high = (source2[sprnum+0] & 0x0e00) >> 9;
			int yzoom = (source2[sprnum+0] & 0xf000) >> 12;

			int xpos = source2[sprnum+1] & 0x1ff;
			int wide = (source2[sprnum+1] & 0x0e00) >> 9;
			int xzoom = (source2[sprnum+1] & 0xf000) >> 12;

			int col = (source2[sprnum+2] & 0x3f00) >> 8;
			int flipx = (source2[sprnum+2] & 0x4000) >> 14;
//          int flipy = (source2[sprnum+2] & 0x8000) >> 15;

			int word_offset = source2[sprnum+3] & 0x7fff;
			int xcnt, ycnt;

			int loopno = 0;

			xzoom = 32 - xzoom;
			yzoom = 32 - yzoom;

			if (ypos > 0xff) ypos -=0x200;

			for (ycnt = 0; ycnt < high+1; ycnt ++) {
				if (!flipx) {
					for (xcnt = 0; xcnt < wide+1; xcnt ++)	{
						int tileno = suprslam_sp_videoram[word_offset+loopno];
						drawgfxzoom(bitmap, gfx, tileno, col, 0, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2, cliprect, TRANSPARENCY_PEN, 15,xzoom << 11, yzoom << 11);
						drawgfxzoom(bitmap, gfx, tileno, col, 0, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2, cliprect, TRANSPARENCY_PEN, 15,xzoom << 11, yzoom << 11);
						loopno ++;
					}
				} else {
					for (xcnt = wide; xcnt >= 0; xcnt --)	{
						int tileno = suprslam_sp_videoram[word_offset+loopno];
						drawgfxzoom(bitmap, gfx, tileno, col, 1, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2, cliprect, TRANSPARENCY_PEN, 15,xzoom << 11, yzoom << 11);
						drawgfxzoom(bitmap, gfx, tileno, col, 1, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2, cliprect, TRANSPARENCY_PEN, 15,xzoom << 11, yzoom << 11);
						loopno ++;
					}
				}
			}
		}
	}
}

/* FG 'SCREEN' LAYER */

WRITE16_HANDLER( suprslam_screen_videoram_w )
{
	suprslam_screen_videoram[offset] = data;
	tilemap_mark_tile_dirty(suprslam_screen_tilemap,offset);
}


static TILE_GET_INFO( get_suprslam_tile_info )
{
	int tileno, colour;

	tileno = suprslam_screen_videoram[tile_index] & 0x0fff;
	colour = suprslam_screen_videoram[tile_index] & 0xf000;

	tileno += screen_bank;
	colour = colour >> 12;

	SET_TILE_INFO(0,tileno,colour,0);
}


/* BG LAYER */
WRITE16_HANDLER( suprslam_bg_videoram_w )
{
	suprslam_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(suprslam_bg_tilemap,offset);
}


static TILE_GET_INFO( get_suprslam_bg_tile_info )
{
	int tileno, colour;

	tileno = suprslam_bg_videoram[tile_index] & 0x0fff;
	colour = suprslam_bg_videoram[tile_index] & 0xf000;

	tileno += bg_bank;
	colour = colour >> 12;

	SET_TILE_INFO(2,tileno,colour,0);
}


VIDEO_START( suprslam )
{
	suprslam_bg_tilemap = tilemap_create(get_suprslam_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 16, 16,64,64);
	suprslam_screen_tilemap = tilemap_create(get_suprslam_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);

	K053936_wraparound_enable(0, 1);
	K053936_set_offset(0, -45, -21);

	tilemap_set_transparent_pen(suprslam_screen_tilemap,15);
}

VIDEO_UPDATE( suprslam )
{
	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	K053936_0_zoom_draw(bitmap,cliprect,suprslam_bg_tilemap,0,0);

	draw_sprites(machine, bitmap, cliprect);

	tilemap_draw(bitmap,cliprect,suprslam_screen_tilemap,0,0);
	return 0;
}

WRITE16_HANDLER (suprslam_bank_w)
{
	UINT16 old_screen_bank, old_bg_bank;
	old_screen_bank = screen_bank;
	old_bg_bank = bg_bank;

	screen_bank = data & 0xf000;
	bg_bank = (data & 0x0f00) << 4;

	if (screen_bank != old_screen_bank) tilemap_mark_all_tiles_dirty (suprslam_screen_tilemap);
	if (bg_bank != old_bg_bank) tilemap_mark_all_tiles_dirty (suprslam_bg_tilemap);
}
