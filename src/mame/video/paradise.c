/***************************************************************************

                              -= Paradise / Target Ball / Torus =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background layer
        W       shows the midground layer
        E       shows the foreground layer
        R       shows the pixmap layer
        A       shows sprites

        There are 4 Fixed 256 x 256 Layers.

        Background tiles are 8x8x4 with a register selecting which
        color code to use.

        midground and foreground tiles are 8x8x8 with no color code.
        Then there's a 16 color pixel layer.

        Bog standard 16x16x8 sprites, apparently with no color code nor flipping.

***************************************************************************/

#include "driver.h"
#include "paradise.h"

/* Variables that driver has access to: */

UINT8 *paradise_vram_0,*paradise_vram_1,*paradise_vram_2;
int paradise_sprite_inc;

/* Variables only used here */

static UINT8 paradise_palbank, paradise_priority;

WRITE8_HANDLER( paradise_flipscreen_w )
{
	flip_screen_set(data ? 0 : 1);
}

WRITE8_HANDLER( tgtball_flipscreen_w )
{
	flip_screen_set(data ? 1 : 0);
}


/* 800 bytes for red, followed by 800 bytes for green & 800 bytes for blue */
WRITE8_HANDLER( paradise_palette_w )
{
	paletteram[offset] = data;
	offset %= 0x800;
	palette_set_color_rgb(Machine,offset,	paletteram[offset + 0x800 * 0],
											paletteram[offset + 0x800 * 1],
											paletteram[offset + 0x800 * 2]	);
}

/***************************************************************************

                                    Tilemaps

    Offset:

    $000.b      Code (Low  Bits)
    $400.b      Code (High Bits)

***************************************************************************/

static tilemap *tilemap_0,*tilemap_1,*tilemap_2;

/* Background */
WRITE8_HANDLER( paradise_vram_0_w )
{
	paradise_vram_0[offset] = data;
	tilemap_mark_tile_dirty(tilemap_0, offset % 0x400);
}

/* 16 color tiles with paradise_palbank as color code */
WRITE8_HANDLER( paradise_palbank_w )
{
	int i;
	int bank1 = (data & 0x0e) | 1;
	int bank2 = (data & 0xf0);

	for (i = 0; i < 15; i++)
		palette_set_color_rgb(Machine,0x800+i,	paletteram[0x200 + bank2 + i + 0x800 * 0],
												paletteram[0x200 + bank2 + i + 0x800 * 1],
												paletteram[0x200 + bank2 + i + 0x800 * 2]	);
	if (paradise_palbank != bank1)
	{
		paradise_palbank = bank1;
		tilemap_mark_all_tiles_dirty(tilemap_0);
	}
}

static TILE_GET_INFO( get_tile_info_0 )
{
	int code = paradise_vram_0[tile_index] + (paradise_vram_0[tile_index + 0x400] << 8);
	SET_TILE_INFO(1, code, paradise_palbank, 0);
}


/* Midground */
WRITE8_HANDLER( paradise_vram_1_w )
{
	paradise_vram_1[offset] = data;
	tilemap_mark_tile_dirty(tilemap_1, offset % 0x400);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	int code = paradise_vram_1[tile_index] + (paradise_vram_1[tile_index + 0x400] << 8);
	SET_TILE_INFO(2, code, 0, 0);
}


/* Foreground */
WRITE8_HANDLER( paradise_vram_2_w )
{
	paradise_vram_2[offset] = data;
	tilemap_mark_tile_dirty(tilemap_2, offset % 0x400);
}

static TILE_GET_INFO( get_tile_info_2 )
{
	int code = paradise_vram_2[tile_index] + (paradise_vram_2[tile_index + 0x400] << 8);
	SET_TILE_INFO(3, code, 0, 0);
}

/* 256 x 256 bitmap. 4 bits per pixel so every byte encodes 2 pixels */

WRITE8_HANDLER( paradise_pixmap_w )
{
	int x,y;

	videoram[offset] = data;

	x = (offset & 0x7f) << 1;
	y = (offset >> 7);

	*BITMAP_ADDR16(tmpbitmap, y, x+0) = 0x80f - (data >> 4);
	*BITMAP_ADDR16(tmpbitmap, y, x+1) = 0x80f - (data & 0x0f);
}


/***************************************************************************

                            Vide Hardware Init

***************************************************************************/

VIDEO_START( paradise )
{
	tilemap_0 = tilemap_create(	get_tile_info_0, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 8,8, 0x20,0x20 );

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 8,8, 0x20,0x20 );

	tilemap_2 = tilemap_create(	get_tile_info_2, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 8,8, 0x20,0x20 );

	/* pixmap */
	tmpbitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	/* paletteram and videoram (pixmap) are accessed through CPU ports, that don't
       get memory automatically allocated for them */
	paletteram	=	auto_malloc(0x1800);
	videoram	=	auto_malloc(0x8000);

	tilemap_set_transparent_pen(tilemap_0,0x0f);
	tilemap_set_transparent_pen(tilemap_1,0xff);
	tilemap_set_transparent_pen(tilemap_2,0xff);
}


/***************************************************************************

                            Sprites Drawing

***************************************************************************/

/* Sprites / Layers priority */
WRITE8_HANDLER( paradise_priority_w )
{
	paradise_priority = data;
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int i;
	for (i = 0; i < spriteram_size ; i += paradise_sprite_inc)
	{
		int code	=	spriteram[i+0];
		int x		=	spriteram[i+1];
		int y		=	spriteram[i+2] - 2;
		int attr	=	spriteram[i+3];

		int flipx	=	0;	// ?
		int flipy	=	0;

		if (flip_screen)	{	x = 0xf0 - x;	flipx = !flipx;
								y = 0xf0 - y;	flipy = !flipy;	}

		drawgfx(bitmap,machine->gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x,y,
				cliprect,TRANSPARENCY_PEN, 0xff );

		/* wrap around x */
		drawgfx(bitmap,machine->gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x - 256,y,
				cliprect,TRANSPARENCY_PEN, 0xff );

		drawgfx(bitmap,machine->gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x + 256,y,
				cliprect,TRANSPARENCY_PEN, 0xff );
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

VIDEO_UPDATE( paradise )
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (input_code_pressed(KEYCODE_Z))
{
	int mask = 0;
	if (input_code_pressed(KEYCODE_Q))	mask |= 1;
	if (input_code_pressed(KEYCODE_W))	mask |= 2;
	if (input_code_pressed(KEYCODE_E))	mask |= 4;
	if (input_code_pressed(KEYCODE_R))	mask |= 8;
	if (input_code_pressed(KEYCODE_A))	mask |= 16;
	if (mask != 0) layers_ctrl &= mask;
}
#endif

	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if (!(paradise_priority & 4))	/* Screen blanking */
		return 0;

	if (paradise_priority & 1)
		if (layers_ctrl&16)	draw_sprites(machine,bitmap,cliprect);

	if (layers_ctrl&1)	tilemap_draw(bitmap,cliprect, tilemap_0, 0,0);
	if (layers_ctrl&2)	tilemap_draw(bitmap,cliprect, tilemap_1, 0,0);
	if (layers_ctrl&4)	copybitmap(bitmap,tmpbitmap,flip_screen,flip_screen,0,0,cliprect,TRANSPARENCY_PEN, 0x80f);

	if (paradise_priority & 2)
	{
		if (!(paradise_priority & 1))
			if (layers_ctrl&16)	draw_sprites(machine, bitmap,cliprect);
		if (layers_ctrl&8)	tilemap_draw(bitmap,cliprect, tilemap_2, 0,0);
	}
	else
	{
		if (layers_ctrl&8)	tilemap_draw(bitmap,cliprect, tilemap_2, 0,0);
		if (!(paradise_priority & 1))
			if (layers_ctrl&16)	draw_sprites(machine, bitmap,cliprect);
	}
	return 0;
}

/* no pix layer, no tilemap_0, different priority bits */
VIDEO_UPDATE( torus )
{
	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if (!(paradise_priority & 2))	/* Screen blanking */
		return 0;

	if (paradise_priority & 1)
		draw_sprites(machine, bitmap,cliprect);

	tilemap_draw(bitmap,cliprect, tilemap_1, 0,0);

	if(paradise_priority & 4)
	{
		if (!(paradise_priority & 1))
			draw_sprites(machine, bitmap,cliprect);

		tilemap_draw(bitmap,cliprect, tilemap_2, 0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect, tilemap_2, 0,0);

		if (!(paradise_priority & 1))
			draw_sprites(machine, bitmap,cliprect);
	}
	return 0;
}

/* I don't know how the priority bits work on this one */
VIDEO_UPDATE( madball )
{
	fillbitmap(bitmap,get_black_pen(machine),cliprect);
	tilemap_draw(bitmap,cliprect, tilemap_0, 0,0);
	tilemap_draw(bitmap,cliprect, tilemap_1, 0,0);
	tilemap_draw(bitmap,cliprect, tilemap_2, 0,0);
	draw_sprites(machine, bitmap,cliprect);
	return 0;
}


