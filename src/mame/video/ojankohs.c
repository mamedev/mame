/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/06/10 -
    Driver by Uki 2001/12/10 -

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/ojankohs.h"

/******************************************************************************

    Palette system

******************************************************************************/

PALETTE_INIT( ojankoy )
{
	int i;
	int bit0, bit1, bit2, bit3, bit4, r, g, b;

	for (i = 0; i < machine->total_colors(); i++)
	{
		bit0 = BIT(color_prom[0], 2);
		bit1 = BIT(color_prom[0], 3);
		bit2 = BIT(color_prom[0], 4);
		bit3 = BIT(color_prom[0], 5);
		bit4 = BIT(color_prom[0], 6);
		r = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[machine->total_colors()], 5);
		bit1 = BIT(color_prom[machine->total_colors()], 6);
		bit2 = BIT(color_prom[machine->total_colors()], 7);
		bit3 = BIT(color_prom[0], 0);
		bit4 = BIT(color_prom[0], 1);
		g = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[machine->total_colors()], 0);
		bit1 = BIT(color_prom[machine->total_colors()], 1);
		bit2 = BIT(color_prom[machine->total_colors()], 2);
		bit3 = BIT(color_prom[machine->total_colors()], 3);
		bit4 = BIT(color_prom[machine->total_colors()], 4);
		b = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

WRITE8_HANDLER( ojankohs_palette_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	int r, g, b;

	state->paletteram[offset] = data;

	offset &= 0x7fe;

	r = (state->paletteram[offset + 0] & 0x7c) >> 2;
	g = ((state->paletteram[offset + 0] & 0x03) << 3) | ((state->paletteram[offset + 1] & 0xe0) >> 5);
	b = (state->paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(space->machine, offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_HANDLER( ccasino_palette_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	int r, g, b;

	/* get top 8 bits of the I/O port address */
	offset = (offset << 8) | (cpu_get_reg(space->cpu, Z80_BC) >> 8);

	state->paletteram[offset] = data;

	offset &= 0x7fe;

	r = (state->paletteram[offset + 0] & 0x7c) >> 2;
	g = ((state->paletteram[offset + 0] & 0x03) << 3) | ((state->paletteram[offset + 1] & 0xe0) >> 5);
	b = (state->paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(space->machine, offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_HANDLER( ojankoc_palette_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	int r, g, b, color;

	if (state->paletteram[offset] == data)
		return;

	state->paletteram[offset] = data;
	state->screen_refresh = 1;

	color = (state->paletteram[offset & 0x1e] << 8) | state->paletteram[offset | 0x01];

	r = (color >> 10) & 0x1f;
	g = (color >>  5) & 0x1f;
	b = (color >>  0) & 0x1f;

	palette_set_color_rgb(space->machine, offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}


/******************************************************************************

    Tilemap system

******************************************************************************/

WRITE8_HANDLER( ojankohs_videoram_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap, offset);
}

WRITE8_HANDLER( ojankohs_colorram_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap, offset);
}

WRITE8_HANDLER( ojankohs_gfxreg_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();

	if (state->gfxreg != data)
	{
		state->gfxreg = data;
		tilemap_mark_all_tiles_dirty(state->tilemap);
	}
}

WRITE8_HANDLER( ojankohs_flipscreen_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();

	if (state->flipscreen != BIT(data, 0))
	{

		state->flipscreen = BIT(data, 0);

		tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		if (state->flipscreen)
		{
			state->scrollx = -0xe0;
			state->scrolly = -0x20;
		}
		else
		{
			state->scrollx = 0;
			state->scrolly = 0;
		}
	}
}

static TILE_GET_INFO( ojankohs_get_tile_info )
{
	ojankohs_state *state = machine->driver_data<ojankohs_state>();
	int tile = state->videoram[tile_index] | ((state->colorram[tile_index] & 0x0f) << 8);
	int color = (state->colorram[tile_index] & 0xe0) >> 5;

	if (state->colorram[tile_index] & 0x10)
	{
		tile |= (state->gfxreg & 0x07) << 12;
		color |= (state->gfxreg & 0xe0) >> 2;
	}

	SET_TILE_INFO(0, tile, color, 0);
}

static TILE_GET_INFO( ojankoy_get_tile_info )
{
	ojankohs_state *state = machine->driver_data<ojankohs_state>();
	int tile = state->videoram[tile_index] | (state->videoram[tile_index + 0x1000] << 8);
	int color = state->colorram[tile_index] & 0x3f;
	int flipx = ((state->colorram[tile_index] & 0x40) >> 6) ? TILEMAP_FLIPX : 0;
	int flipy = ((state->colorram[tile_index] & 0x80) >> 7) ? TILEMAP_FLIPY : 0;

	SET_TILE_INFO(0, tile, color, (flipx | flipy));
}


/******************************************************************************

    Pixel system

******************************************************************************/

void ojankoc_flipscreen( address_space *space, int data )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	int x, y;
	UINT8 color1, color2;

	state->flipscreen = BIT(data, 7);

	if (state->flipscreen == state->flipscreen_old)
		return;

	for (y = 0; y < 0x40; y++)
	{
		for (x = 0; x < 0x100; x++)
		{
			color1 = state->videoram[0x0000 + ((y * 256) + x)];
			color2 = state->videoram[0x3fff - ((y * 256) + x)];
			ojankoc_videoram_w(space, 0x0000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(space, 0x3fff - ((y * 256) + x), color1);

			color1 = state->videoram[0x4000 + ((y * 256) + x)];
			color2 = state->videoram[0x7fff - ((y * 256) + x)];
			ojankoc_videoram_w(space, 0x4000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(space, 0x7fff - ((y * 256) + x), color1);
		}
	}

	state->flipscreen_old = state->flipscreen;
}

WRITE8_HANDLER( ojankoc_videoram_w )
{
	ojankohs_state *state = space->machine->driver_data<ojankohs_state>();
	int i;
	UINT8 x, y, xx, px, py ;
	UINT8 color, color1, color2;

	state->videoram[offset] = data;

	color1 = state->videoram[offset & 0x3fff];
	color2 = state->videoram[offset | 0x4000];

	y = offset >> 6;
	x = (offset & 0x3f) << 2;
	xx = 0;

	if (state->flipscreen)
	{
		x = 0xfc - x;
		y = 0xff - y;
		xx = 3;
	}

	for (i = 0; i < 4; i++)
	{
		color = ((color1 & 0x01) >> 0) | ((color1 & 0x10) >> 3) | ((color2 & 0x01) << 2) | ((color2 & 0x10) >> 1);

		px = x + (i ^ xx);
		py = y;

		*BITMAP_ADDR16(state->tmpbitmap, py, px) = color;

		color1 >>= 1;
		color2 >>= 1;
	}
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( ojankohs )
{
	ojankohs_state *state = machine->driver_data<ojankohs_state>();

	state->tilemap = tilemap_create(machine, ojankohs_get_tile_info, tilemap_scan_rows,  8, 4, 64, 64);
	state->videoram = auto_alloc_array(machine, UINT8, 0x2000);
	state->colorram = auto_alloc_array(machine, UINT8, 0x1000);
	state->paletteram = auto_alloc_array(machine, UINT8, 0x800);

	state_save_register_global_pointer(machine, state->videoram, 0x2000);
	state_save_register_global_pointer(machine, state->colorram, 0x1000);
	state_save_register_global_pointer(machine, state->paletteram, 0x800);
}

VIDEO_START( ojankoy )
{
	ojankohs_state *state = machine->driver_data<ojankohs_state>();

	state->tilemap = tilemap_create(machine, ojankoy_get_tile_info, tilemap_scan_rows,  8, 4, 64, 64);
	state->videoram = auto_alloc_array(machine, UINT8, 0x2000);
	state->colorram = auto_alloc_array(machine, UINT8, 0x1000);
	state->paletteram = auto_alloc_array(machine, UINT8, 0x800);

	state_save_register_global_pointer(machine, state->videoram, 0x2000);
	state_save_register_global_pointer(machine, state->colorram, 0x1000);
	state_save_register_global_pointer(machine, state->paletteram, 0x800);
}

VIDEO_START( ojankoc )
{
	ojankohs_state *state = machine->driver_data<ojankohs_state>();

	state->tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();
	state->videoram = auto_alloc_array(machine, UINT8, 0x8000);
	state->paletteram = auto_alloc_array(machine, UINT8, 0x20);

	state_save_register_global_pointer(machine, state->videoram, 0x8000);
	state_save_register_global_pointer(machine, state->paletteram, 0x20);
	state_save_register_global_bitmap(machine, state->tmpbitmap);
}


/******************************************************************************

    Display refresh

******************************************************************************/

VIDEO_UPDATE( ojankohs )
{
	ojankohs_state *state = screen->machine->driver_data<ojankohs_state>();

	tilemap_set_scrollx(state->tilemap, 0, state->scrollx);
	tilemap_set_scrolly(state->tilemap, 0, state->scrolly);

	tilemap_draw(bitmap, cliprect, state->tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( ojankoc )
{
	ojankohs_state *state = screen->machine->driver_data<ojankohs_state>();
	int offs;

	if (state->screen_refresh)
	{
		address_space *space = cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

		/* redraw bitmap */
		for (offs = 0; offs < 0x8000; offs++)
		{
			ojankoc_videoram_w(space, offs, state->videoram[offs]);
		}
		state->screen_refresh = 0;
	}

	copybitmap(bitmap, state->tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
