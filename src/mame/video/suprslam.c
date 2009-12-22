/* Super Slams - video, see notes in driver file */

#include "driver.h"
#include "video/konicdev.h"
#include "includes/suprslam.h"


/* todo, fix zooming correctly, it's _not_ like aerofgt */
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
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

	suprslam_state *state = (suprslam_state *)machine->driver_data;
	const gfx_element *gfx = machine->gfx[1];
	UINT16 *source = state->spriteram;
	UINT16 *source2 = state->spriteram;
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
						int tileno = state->sp_videoram[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						loopno ++;
					}
				}
				else
				{
					for (xcnt = wide; xcnt >= 0; xcnt --)
					{
						int tileno = state->sp_videoram[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
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
	suprslam_state *state = (suprslam_state *)space->machine->driver_data;

	state->screen_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->screen_tilemap, offset);
}


static TILE_GET_INFO( get_suprslam_tile_info )
{
	suprslam_state *state = (suprslam_state *)machine->driver_data;
	int tileno = state->screen_videoram[tile_index] & 0x0fff;
	int colour = state->screen_videoram[tile_index] & 0xf000;

	tileno += state->screen_bank;
	colour = colour >> 12;

	SET_TILE_INFO(0, tileno, colour, 0);
}


/* BG LAYER */
WRITE16_HANDLER( suprslam_bg_videoram_w )
{
	suprslam_state *state = (suprslam_state *)space->machine->driver_data;

	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


static TILE_GET_INFO( get_suprslam_bg_tile_info )
{
	suprslam_state *state = (suprslam_state *)machine->driver_data;
	int tileno = state->bg_videoram[tile_index] & 0x0fff;
	int colour = state->bg_videoram[tile_index] & 0xf000;

	tileno += state->bg_bank;
	colour = colour >> 12;

	SET_TILE_INFO(2, tileno, colour, 0);
}


VIDEO_START( suprslam )
{
	suprslam_state *state = (suprslam_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_suprslam_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->screen_tilemap = tilemap_create(machine, get_suprslam_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->screen_tilemap, 15);
}

VIDEO_UPDATE( suprslam )
{
	suprslam_state *state = (suprslam_state *)screen->machine->driver_data;
	tilemap_set_scrollx( state->screen_tilemap,0, state->screen_vregs[0x04/2] );

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	k053936_zoom_draw(state->k053936, bitmap, cliprect, state->bg_tilemap, 0, 0, 1);
	if(!(state->spr_ctrl[0] & 8))
		draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->screen_tilemap, 0, 0);
	if(state->spr_ctrl[0] & 8)
		draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

WRITE16_HANDLER (suprslam_bank_w)
{
	suprslam_state *state = (suprslam_state *)space->machine->driver_data;
	UINT16 old_screen_bank, old_bg_bank;
	old_screen_bank = state->screen_bank;
	old_bg_bank = state->bg_bank;

	state->screen_bank = data & 0xf000;
	state->bg_bank = (data & 0x0f00) << 4;

	if (state->screen_bank != old_screen_bank)
		tilemap_mark_all_tiles_dirty(state->screen_tilemap);
	if (state->bg_bank != old_bg_bank)
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
}
