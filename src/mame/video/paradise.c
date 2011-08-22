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

#include "emu.h"
#include "includes/paradise.h"

WRITE8_HANDLER( paradise_flipscreen_w )
{
	flip_screen_set(space->machine(), data ? 0 : 1);
}

WRITE8_HANDLER( tgtball_flipscreen_w )
{
	flip_screen_set(space->machine(), data ? 1 : 0);
}

/* Note: Penky updates pixel palette bank register BEFORE actually writing to the paletteram. */
static void update_pix_palbank(running_machine &machine)
{
	paradise_state *state = machine.driver_data<paradise_state>();
	int i;

	for (i = 0; i < 15; i++)
		palette_set_color_rgb(machine, 0x800 + i, state->m_paletteram[0x200 + state->m_pixbank + i + 0x800 * 0], state->m_paletteram[0x200 + state->m_pixbank + i + 0x800 * 1],
								state->m_paletteram[0x200 + state->m_pixbank + i + 0x800 * 2]);
}

/* 800 bytes for red, followed by 800 bytes for green & 800 bytes for blue */
WRITE8_HANDLER( paradise_palette_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	state->m_paletteram[offset] = data;
	offset %= 0x800;
	palette_set_color_rgb(space->machine(), offset, state->m_paletteram[offset + 0x800 * 0], state->m_paletteram[offset + 0x800 * 1],
		state->m_paletteram[offset + 0x800 * 2]);

	update_pix_palbank(space->machine());
}

/***************************************************************************

                                    Tilemaps

    Offset:

    $000.b      Code (Low  Bits)
    $400.b      Code (High Bits)

***************************************************************************/

/* Background */
WRITE8_HANDLER( paradise_vram_0_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	state->m_vram_0[offset] = data;
	tilemap_mark_tile_dirty(state->m_tilemap_0, offset % 0x400);
}

/* 16 color tiles with paradise_palbank as color code */
WRITE8_HANDLER( paradise_palbank_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	int bank1 = (data & 0x0e) | 1;
	int bank2 = (data & 0xf0);

	state->m_pixbank = bank2;

	update_pix_palbank(space->machine());

	if (state->m_palbank != bank1)
	{
		state->m_palbank = bank1;
		tilemap_mark_all_tiles_dirty(state->m_tilemap_0);
	}
}

static TILE_GET_INFO( get_tile_info_0 )
{
	paradise_state *state = machine.driver_data<paradise_state>();
	int code = state->m_vram_0[tile_index] + (state->m_vram_0[tile_index + 0x400] << 8);
	SET_TILE_INFO(1, code, state->m_palbank, 0);
}


/* Midground */
WRITE8_HANDLER( paradise_vram_1_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	state->m_vram_1[offset] = data;
	tilemap_mark_tile_dirty(state->m_tilemap_1, offset % 0x400);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	paradise_state *state = machine.driver_data<paradise_state>();
	int code = state->m_vram_1[tile_index] + (state->m_vram_1[tile_index + 0x400] << 8);
	SET_TILE_INFO(2, code, 0, 0);
}


/* Foreground */
WRITE8_HANDLER( paradise_vram_2_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	state->m_vram_2[offset] = data;
	tilemap_mark_tile_dirty(state->m_tilemap_2, offset % 0x400);
}

static TILE_GET_INFO( get_tile_info_2 )
{
	paradise_state *state = machine.driver_data<paradise_state>();
	int code = state->m_vram_2[tile_index] + (state->m_vram_2[tile_index + 0x400] << 8);
	SET_TILE_INFO(3, code, 0, 0);
}

/* 256 x 256 bitmap. 4 bits per pixel so every byte encodes 2 pixels */

WRITE8_HANDLER( paradise_pixmap_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	int x, y;

	state->m_videoram[offset] = data;

	x = (offset & 0x7f) << 1;
	y = (offset >> 7);

	*BITMAP_ADDR16(state->m_tmpbitmap, y, x + 0) = 0x80f - (data >> 4);
	*BITMAP_ADDR16(state->m_tmpbitmap, y, x + 1) = 0x80f - (data & 0x0f);
}


/***************************************************************************

                            Vide Hardware Init

***************************************************************************/

VIDEO_START( paradise )
{
	paradise_state *state = machine.driver_data<paradise_state>();

	state->m_tilemap_0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, 0x20, 0x20);
	state->m_tilemap_1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, 0x20, 0x20);
	state->m_tilemap_2 = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, 0x20, 0x20);

	/* pixmap */
	state->m_tmpbitmap = machine.primary_screen->alloc_compatible_bitmap();

	tilemap_set_transparent_pen(state->m_tilemap_0, 0x0f);
	tilemap_set_transparent_pen(state->m_tilemap_1, 0xff);
	tilemap_set_transparent_pen(state->m_tilemap_2, 0xff);

	state->save_item(NAME(*state->m_tmpbitmap));
}


/***************************************************************************

                            Sprites Drawing

***************************************************************************/

/* Sprites / Layers priority */
WRITE8_HANDLER( paradise_priority_w )
{
	paradise_state *state = space->machine().driver_data<paradise_state>();
	state->m_priority = data;
}

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	paradise_state *state = machine.driver_data<paradise_state>();
	UINT8 *spriteram = state->m_spriteram;
	int i;
	for (i = 0; i < state->m_spriteram_size ; i += state->m_sprite_inc)
	{
		int code = spriteram[i + 0];
		int x    = spriteram[i + 1];
		int y    = spriteram[i + 2] - 2;
		int attr = spriteram[i + 3];

		int flipx = 0;	// ?
		int flipy = 0;

		if (flip_screen_get(machine))
		{
			x = 0xf0 - x;	flipx = !flipx;
			y = 0xf0 - y;	flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x,y, 0xff );

		/* wrap around x */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x - 256,y, 0xff );

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code + (attr << 8),
				0,
				flipx, flipy,
				x + 256,y, 0xff );
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

SCREEN_UPDATE( paradise )
{
	paradise_state *state = screen->machine().driver_data<paradise_state>();
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (screen->machine().input().code_pressed(KEYCODE_Z))
{
	int mask = 0;
	if (screen->machine().input().code_pressed(KEYCODE_Q))	mask |= 1;
	if (screen->machine().input().code_pressed(KEYCODE_W))	mask |= 2;
	if (screen->machine().input().code_pressed(KEYCODE_E))	mask |= 4;
	if (screen->machine().input().code_pressed(KEYCODE_R))	mask |= 8;
	if (screen->machine().input().code_pressed(KEYCODE_A))	mask |= 16;
	if (mask != 0) layers_ctrl &= mask;
}
#endif

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	if (!(state->m_priority & 4))	/* Screen blanking */
		return 0;

	if (state->m_priority & 1)
		if (layers_ctrl & 16)
			draw_sprites(screen->machine(), bitmap, cliprect);

	if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->m_tilemap_0, 0, 0);
	if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->m_tilemap_1, 0, 0);
	if (layers_ctrl & 4)	copybitmap_trans(bitmap, state->m_tmpbitmap, flip_screen_get(screen->machine()), flip_screen_get(screen->machine()), 0, 0, cliprect, 0x80f);

	if (state->m_priority & 2)
	{
		if (!(state->m_priority & 1))
			if (layers_ctrl & 16)
				draw_sprites(screen->machine(), bitmap, cliprect);
		if (layers_ctrl & 8)
			tilemap_draw(bitmap,cliprect, state->m_tilemap_2, 0, 0);
	}
	else
	{
		if (layers_ctrl & 8)
			tilemap_draw(bitmap, cliprect, state->m_tilemap_2, 0, 0);
		if (!(state->m_priority & 1))
			if (layers_ctrl & 16)
				draw_sprites(screen->machine(), bitmap, cliprect);
	}
	return 0;
}

/* no pix layer, no tilemap_0, different priority bits */
SCREEN_UPDATE( torus )
{
	paradise_state *state = screen->machine().driver_data<paradise_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	if (!(state->m_priority & 2))	/* Screen blanking */
		return 0;

	if (state->m_priority & 1)
		draw_sprites(screen->machine(), bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, state->m_tilemap_1, 0,0);

	if (state->m_priority & 4)
	{
		if (!(state->m_priority & 1))
			draw_sprites(screen->machine(), bitmap, cliprect);

		tilemap_draw(bitmap, cliprect, state->m_tilemap_2, 0, 0);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->m_tilemap_2, 0, 0);

		if (!(state->m_priority & 1))
			draw_sprites(screen->machine(), bitmap,cliprect);
	}
	return 0;
}

/* I don't know how the priority bits work on this one */
SCREEN_UPDATE( madball )
{
	paradise_state *state = screen->machine().driver_data<paradise_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	tilemap_draw(bitmap, cliprect, state->m_tilemap_0, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_tilemap_1, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_tilemap_2, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
