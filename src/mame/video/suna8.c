/***************************************************************************

                            -=  SunA 8 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)

    These games have only sprites, of a peculiar type:

    there is a region of memory where 4 pages of 32x32 tile codes can
    be written like a tilemap made of 4 pages of 256x256 pixels. Each
    tile uses 2 bytes. Later games may use more pages through RAM
    banking.

    Sprites are rectangular regions of *tiles* fetched from there and
    sent to the screen. Each sprite uses 4 bytes, held within the last
    page of tiles.

    * Note: later games use a more complex format than the following,
            which is yet to be completely understood.

                            [ Sprites Format ]


    Offset:         Bits:               Value:

        0.b                             Y (Bottom up)

        1.b         7--- ----           Sprite Size (1 = 2x32 tiles; 0 = 2x2)

                    2x2 Sprites:
                    -65- ----           Tiles Row (height = 8 tiles)
                    ---4 ----           Page

                    2x32 Sprites:
                    -6-- ----           Ignore X (Multisprite)
                    --54 ----           Page

                    ---- 3210           Tiles Column (width = 2 tiles)

        2.b                             X

        3.b         7--- ----
                    -6-- ----           X (Sign Bit)
                    --54 3---
                    ---- -210           Tiles Bank


                        [ Sprite's Tiles Format ]


    Offset:         Bits:                   Value:

        0.b                             Code (Low Bits)

        1.b         7--- ----           Flip Y
                    -6-- ----           Flip X
                    --54 32--           Color
                    ---- --10           Code (High Bits)



    Set TILEMAPS to 1 to debug.
    Press Z (you see the "tilemaps" in RAM) or
    Press X (you see the "tilemaps" in ROM) then

    - use Q&W to cycle through the pages.
    - Use E&R to cycle through the tiles banks.
    - Use A&S to cycle through the ROM banks (only with X pressed, of course).

***************************************************************************/

#include "emu.h"
#include "includes/suna8.h"

/***************************************************************************
    For Debug: there's no tilemap, just sprites.
***************************************************************************/
#if TILEMAPS
static TILE_GET_INFO( get_tile_info )
{
	suna8_state *state = machine->driver_data<suna8_state>();
	UINT8 code, attr;

	if (input_code_pressed(machine, KEYCODE_X))
	{
		UINT8 *rom = memory_region(machine, "maincpu") + 0x10000 + 0x4000 * state->trombank;
		code = rom[ 2 * tile_index + 0 ];
		attr = rom[ 2 * tile_index + 1 ];
	}
	else
	{
		code = state->spriteram[ 2 * tile_index + 0 ];
		attr = state->spriteram[ 2 * tile_index + 1 ];
	}
	SET_TILE_INFO(
			0,
			( (attr & 0x03) << 8 ) + code + state->tiles*0x400,
			(attr >> 2) & 0xf,
			TILE_FLIPYX( (attr >> 6) & 3 ));
}
#endif


READ8_HANDLER( suna8_banked_paletteram_r )
{
	suna8_state *state = space->machine->driver_data<suna8_state>();

	offset += state->palettebank * 0x200;
	return space->machine->generic.paletteram.u8[offset];
}

READ8_HANDLER( suna8_banked_spriteram_r )
{
	suna8_state *state = space->machine->driver_data<suna8_state>();

	offset += state->spritebank * 0x2000;
	return state->spriteram[offset];
}

WRITE8_HANDLER( suna8_spriteram_w )
{
	suna8_state *state = space->machine->driver_data<suna8_state>();

	state->spriteram[offset] = data;
#if TILEMAPS
	tilemap_mark_tile_dirty(state->bg_tilemap, offset/2);
#endif
}

WRITE8_HANDLER( suna8_banked_spriteram_w )
{
	suna8_state *state = space->machine->driver_data<suna8_state>();

	offset += state->spritebank * 0x2000;
	state->spriteram[offset] = data;
#if TILEMAPS
	tilemap_mark_tile_dirty(state->bg_tilemap, offset/2);
#endif
}

/*
    Banked Palette RAM. The data is scrambled
*/
WRITE8_HANDLER( brickzn_banked_paletteram_w )
{
	suna8_state *state = space->machine->driver_data<suna8_state>();
	int r,g,b;
	UINT16 rgb;

	offset += state->palettebank * 0x200;
	space->machine->generic.paletteram.u8[offset] = data;
	rgb = (space->machine->generic.paletteram.u8[offset&~1] << 8) + space->machine->generic.paletteram.u8[offset|1];
	r	=	(((rgb & (1<<0xc))?1:0)<<0) |
			(((rgb & (1<<0xb))?1:0)<<1) |
			(((rgb & (1<<0xe))?1:0)<<2) |
			(((rgb & (1<<0xf))?1:0)<<3);
	g	=	(((rgb & (1<<0x8))?1:0)<<0) |
			(((rgb & (1<<0x9))?1:0)<<1) |
			(((rgb & (1<<0xa))?1:0)<<2) |
			(((rgb & (1<<0xd))?1:0)<<3);
	b	=	(((rgb & (1<<0x4))?1:0)<<0) |
			(((rgb & (1<<0x3))?1:0)<<1) |
			(((rgb & (1<<0x6))?1:0)<<2) |
			(((rgb & (1<<0x7))?1:0)<<3);

	palette_set_color_rgb(space->machine,offset/2,pal4bit(r),pal4bit(g),pal4bit(b));
}



static void suna8_vh_start_common(running_machine *machine, int dim)
{
	suna8_state *state = machine->driver_data<suna8_state>();

	state->text_dim = dim;
	if (!(state->text_dim > 0))
	{
		machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200 * 2);
		state->spriteram = auto_alloc_array(machine, UINT8, 0x2000 * 2);
		state->spritebank  = 0;
		state->palettebank = 0;
	}

#if TILEMAPS
	state->bg_tilemap = tilemap_create(	machine, get_tile_info, tilemap_scan_cols,

								8, 8, 0x20*((state->text_dim > 0)?4:8), 0x20);

	tilemap_set_transparent_pen(state->bg_tilemap, 15);
#endif
}

VIDEO_START( suna8_textdim0 )	{ suna8_vh_start_common(machine, 0);  }
VIDEO_START( suna8_textdim8 )	{ suna8_vh_start_common(machine, 8);  }
VIDEO_START( suna8_textdim12 )	{ suna8_vh_start_common(machine, 12); }

/***************************************************************************


                                Sprites Drawing


***************************************************************************/

static void draw_normal_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	suna8_state *state = machine->driver_data<suna8_state>();
	UINT8 *spriteram = state->spriteram;
	int i;
	int mx = 0;	// multisprite x counter

	int max_x = machine->primary_screen->width() - 8;
	int max_y = machine->primary_screen->height() - 8;

	for (i = 0x1d00; i < 0x2000; i += 4)
	{
		int srcpg, srcx,srcy, dimx,dimy, tx, ty;
		int gfxbank, colorbank = 0, flipx,flipy, multisprite;

		int y		=	spriteram[i + 0];
		int code	=	spriteram[i + 1];
		int x		=	spriteram[i + 2];
		int bank	=	spriteram[i + 3];

		if (state->text_dim > 0)
		{
			/* Older, simpler hardware */
			flipx = 0;
			flipy = 0;
			gfxbank = bank & 0x3f;
			switch( code & 0x80 )
			{
			case 0x80:
				dimx = 2;					dimy =	32;
				srcx  = (code & 0xf) * 2;	srcy = 0;
				srcpg = (code >> 4) & 3;
				break;
			case 0x00:
			default:
				dimx = 2;					dimy =	2;
				srcx  = (code & 0xf) * 2;	srcy = ((code >> 5) & 0x3) * 8 + 6;
				srcpg = (code >> 4) & 1;
				break;
			}
			multisprite = ((code & 0x80) && (code & 0x40));
		}
		else
		{
			/* Newer, more complex hardware (not finished yet!) */
			switch( code & 0xc0 )
			{
			case 0xc0:
				dimx = 4;					dimy = 32;
				srcx  = (code & 0xe) * 2;	srcy = 0;
				flipx = (code & 0x1);
				flipy = 0;
				gfxbank = bank & 0x1f;
				srcpg = (code >> 4) & 3;
				break;
			case 0x80:
				dimx = 2;					dimy = 32;
				srcx  = (code & 0xf) * 2;	srcy = 0;
				flipx = 0;
				flipy = 0;
				gfxbank = bank & 0x1f;
				srcpg = (code >> 4) & 3;
				break;
// hardhea2: fire code=52/54 bank=a4; player code=02/04/06 bank=08; arrow:code=16 bank=27
			case 0x40:
				dimx = 4;					dimy = 4;
				srcx  = (code & 0xe) * 2;
				flipx = code & 0x01;
				flipy = bank & 0x10;
				srcy  = (((bank & 0x80)>>4) + (bank & 0x04) + ((~bank >> 4)&2)) * 2;
				srcpg = (code >> 4) & 7;
				gfxbank = (bank & 0x3) + (srcpg & 4);	// brickzn: 06,a6,a2,b2->6. starfigh: 01->01,4->0
				colorbank = (bank & 8) >> 3;
				break;
			case 0x00:
			default:
				dimx = 2;					dimy = 2;
				srcx  = (code & 0xf) * 2;
				flipx = 0;
				flipy = 0;
				gfxbank = bank & 0x03;
				srcy  = (((bank & 0x80)>>4) + (bank & 0x04) + ((~bank >> 4)&3)) * 2;
				srcpg = (code >> 4) & 3;
				break;
			}
			multisprite = ((code & 0x80) && (bank & 0x80));
		}

		x = x - ((bank & 0x40) ? 0x100 : 0);
		y = (0x100 - y - dimy*8 ) & 0xff;

		/* Multi Sprite */
		if ( multisprite )	{	mx += dimx*8;	x = mx;	}
		else					mx = x;

		gfxbank	*= 0x400;

		for (ty = 0; ty < dimy; ty ++)
		{
			for (tx = 0; tx < dimx; tx ++)
			{
				int addr	=	(srcpg * 0x20 * 0x20) +
								((srcx + (flipx?dimx-tx-1:tx)) & 0x1f) * 0x20 +
								((srcy + (flipy?dimy-ty-1:ty)) & 0x1f);

				int tile	=	spriteram[addr*2 + 0];
				int attr	=	spriteram[addr*2 + 1];

				int tile_flipx	=	attr & 0x40;
				int tile_flipy	=	attr & 0x80;

				int sx		=	 x + tx * 8;
				int sy		=	(y + ty * 8) & 0xff;

				if (flipx)	tile_flipx = !tile_flipx;
				if (flipy)	tile_flipy = !tile_flipy;

				if (flip_screen_get(machine))
				{
					sx = max_x - sx;	tile_flipx = !tile_flipx;
					sy = max_y - sy;	tile_flipy = !tile_flipy;
				}

				drawgfx_transpen(	bitmap,cliprect,machine->gfx[0],
							tile + (attr & 0x3)*0x100 + gfxbank,
							((attr >> 2) & 0xf) | colorbank,	// hardhea2 player2
							tile_flipx, tile_flipy,
							sx, sy,15);
			}
		}

	}
}

static void draw_text_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	suna8_state *state = machine->driver_data<suna8_state>();
	UINT8 *spriteram = state->spriteram;
	int i;

	int max_x = machine->primary_screen->width() - 8;
	int max_y = machine->primary_screen->height() - 8;

	/* Earlier games only */
	if (!(state->text_dim > 0))	return;

	for (i = 0x1900; i < 0x19ff; i += 4)
	{
		int srcpg, srcx,srcy, dimx,dimy, tx, ty;

		int y		=	spriteram[i + 0];
		int code	=	spriteram[i + 1];
		int x		=	spriteram[i + 2];
		int bank	=	spriteram[i + 3];

		if (~code & 0x80)	continue;

		dimx = 2;					dimy = state->text_dim;
		srcx  = (code & 0xf) * 2;	srcy = (y & 0xf0) / 8;
		srcpg = (code >> 4) & 3;

		x = x - ((bank & 0x40) ? 0x100 : 0);
		y = 0;

		bank	=	(bank & 0x3f) * 0x400;

		for (ty = 0; ty < dimy; ty ++)
		{
			for (tx = 0; tx < dimx; tx ++)
			{
				int real_ty	=	(ty < (dimy/2)) ? ty : (ty + 0x20 - dimy);

				int addr	=	(srcpg * 0x20 * 0x20) +
								((srcx + tx) & 0x1f) * 0x20 +
								((srcy + real_ty) & 0x1f);

				int tile	=	spriteram[addr*2 + 0];
				int attr	=	spriteram[addr*2 + 1];

				int flipx	=	attr & 0x40;
				int flipy	=	attr & 0x80;

				int sx		=	 x + tx * 8;
				int sy		=	(y + real_ty * 8) & 0xff;

				if (flip_screen_get(machine))
				{
					sx = max_x - sx;	flipx = !flipx;
					sy = max_y - sy;	flipy = !flipy;
				}

				drawgfx_transpen(	bitmap,cliprect,machine->gfx[0],
							tile + (attr & 0x3)*0x100 + bank,
							(attr >> 2) & 0xf,
							flipx, flipy,
							sx, sy,15);
			}
		}

	}
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( suna8 )
{
	/* see hardhead, hardhea2 test mode (press button 2 for both players) */
	bitmap_fill(bitmap,cliprect,0xff);

#ifdef MAME_DEBUG
#if TILEMAPS
	if (input_code_pressed(screen->machine, KEYCODE_Z) || input_code_pressed(screen->machine, KEYCODE_X))
	{
		suna8_state *state = screen->machine->driver_data<suna8_state>();
		int max_tiles = memory_region_length(screen->machine, "gfx1") / (0x400 * 0x20);

		if (input_code_pressed_once(screen->machine, KEYCODE_Q))	{ state->page--;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}
		if (input_code_pressed_once(screen->machine, KEYCODE_W))	{ state->page++;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}
		if (input_code_pressed_once(screen->machine, KEYCODE_E))	{ state->tiles--;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}
		if (input_code_pressed_once(screen->machine, KEYCODE_R))	{ state->tiles++;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}
		if (input_code_pressed_once(screen->machine, KEYCODE_A))	{ state->trombank--;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}
		if (input_code_pressed_once(screen->machine, KEYCODE_S))	{ state->trombank++;	tilemap_mark_all_tiles_dirty_all(screen->machine);	}

		state->rombank  &= 0xf;
		state->page  &= (state->text_dim > 0)?3:7;
		state->tiles %= max_tiles;
		if (state->tiles < 0) state->tiles += max_tiles;

		tilemap_set_scrollx(state->bg_tilemap, 0, 0x100 * state->page);
		tilemap_set_scrolly(state->bg_tilemap, 0, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
#if 1
	popmessage("%02X %02X %02X %02X - p%2X g%02X r%02X",
						state->rombank, state->palettebank, state->spritebank, state->unknown,
						state->page, state->tiles, state->trombank);
#endif
	}
	else
#endif
#endif
	{
		draw_normal_sprites(screen->machine ,bitmap,cliprect);
		draw_text_sprites(screen->machine, bitmap,cliprect);
	}
	return 0;
}
