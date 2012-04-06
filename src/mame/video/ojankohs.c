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

	for (i = 0; i < machine.total_colors(); i++)
	{
		bit0 = BIT(color_prom[0], 2);
		bit1 = BIT(color_prom[0], 3);
		bit2 = BIT(color_prom[0], 4);
		bit3 = BIT(color_prom[0], 5);
		bit4 = BIT(color_prom[0], 6);
		r = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[machine.total_colors()], 5);
		bit1 = BIT(color_prom[machine.total_colors()], 6);
		bit2 = BIT(color_prom[machine.total_colors()], 7);
		bit3 = BIT(color_prom[0], 0);
		bit4 = BIT(color_prom[0], 1);
		g = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[machine.total_colors()], 0);
		bit1 = BIT(color_prom[machine.total_colors()], 1);
		bit2 = BIT(color_prom[machine.total_colors()], 2);
		bit3 = BIT(color_prom[machine.total_colors()], 3);
		bit4 = BIT(color_prom[machine.total_colors()], 4);
		b = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

WRITE8_MEMBER(ojankohs_state::ojankohs_palette_w)
{
	int r, g, b;

	m_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(machine(), offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(ojankohs_state::ccasino_palette_w)
{
	int r, g, b;

	/* get top 8 bits of the I/O port address */
	offset = (offset << 8) | (cpu_get_reg(&space.device(), Z80_BC) >> 8);

	m_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(machine(), offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(ojankohs_state::ojankoc_palette_w)
{
	int r, g, b, color;

	if (m_paletteram[offset] == data)
		return;

	m_paletteram[offset] = data;
	m_screen_refresh = 1;

	color = (m_paletteram[offset & 0x1e] << 8) | m_paletteram[offset | 0x01];

	r = (color >> 10) & 0x1f;
	g = (color >>  5) & 0x1f;
	b = (color >>  0) & 0x1f;

	palette_set_color_rgb(machine(), offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}


/******************************************************************************

    Tilemap system

******************************************************************************/

WRITE8_MEMBER(ojankohs_state::ojankohs_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_colorram_w)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_gfxreg_w)
{

	if (m_gfxreg != data)
	{
		m_gfxreg = data;
		m_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(ojankohs_state::ojankohs_flipscreen_w)
{

	if (m_flipscreen != BIT(data, 0))
	{

		m_flipscreen = BIT(data, 0);

		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		if (m_flipscreen)
		{
			m_scrollx = -0xe0;
			m_scrolly = -0x20;
		}
		else
		{
			m_scrollx = 0;
			m_scrolly = 0;
		}
	}
}

static TILE_GET_INFO( ojankohs_get_tile_info )
{
	ojankohs_state *state = machine.driver_data<ojankohs_state>();
	int tile = state->m_videoram[tile_index] | ((state->m_colorram[tile_index] & 0x0f) << 8);
	int color = (state->m_colorram[tile_index] & 0xe0) >> 5;

	if (state->m_colorram[tile_index] & 0x10)
	{
		tile |= (state->m_gfxreg & 0x07) << 12;
		color |= (state->m_gfxreg & 0xe0) >> 2;
	}

	SET_TILE_INFO(0, tile, color, 0);
}

static TILE_GET_INFO( ojankoy_get_tile_info )
{
	ojankohs_state *state = machine.driver_data<ojankohs_state>();
	int tile = state->m_videoram[tile_index] | (state->m_videoram[tile_index + 0x1000] << 8);
	int color = state->m_colorram[tile_index] & 0x3f;
	int flipx = ((state->m_colorram[tile_index] & 0x40) >> 6) ? TILEMAP_FLIPX : 0;
	int flipy = ((state->m_colorram[tile_index] & 0x80) >> 7) ? TILEMAP_FLIPY : 0;

	SET_TILE_INFO(0, tile, color, (flipx | flipy));
}


/******************************************************************************

    Pixel system

******************************************************************************/

void ojankoc_flipscreen( address_space *space, int data )
{
	ojankohs_state *state = space->machine().driver_data<ojankohs_state>();
	int x, y;
	UINT8 color1, color2;

	state->m_flipscreen = BIT(data, 7);

	if (state->m_flipscreen == state->m_flipscreen_old)
		return;

	for (y = 0; y < 0x40; y++)
	{
		for (x = 0; x < 0x100; x++)
		{
			color1 = state->m_videoram[0x0000 + ((y * 256) + x)];
			color2 = state->m_videoram[0x3fff - ((y * 256) + x)];
			state->ojankoc_videoram_w(*space, 0x0000 + ((y * 256) + x), color2);
			state->ojankoc_videoram_w(*space, 0x3fff - ((y * 256) + x), color1);

			color1 = state->m_videoram[0x4000 + ((y * 256) + x)];
			color2 = state->m_videoram[0x7fff - ((y * 256) + x)];
			state->ojankoc_videoram_w(*space, 0x4000 + ((y * 256) + x), color2);
			state->ojankoc_videoram_w(*space, 0x7fff - ((y * 256) + x), color1);
		}
	}

	state->m_flipscreen_old = state->m_flipscreen;
}

WRITE8_MEMBER(ojankohs_state::ojankoc_videoram_w)
{
	int i;
	UINT8 x, y, xx, px, py ;
	UINT8 color, color1, color2;

	m_videoram[offset] = data;

	color1 = m_videoram[offset & 0x3fff];
	color2 = m_videoram[offset | 0x4000];

	y = offset >> 6;
	x = (offset & 0x3f) << 2;
	xx = 0;

	if (m_flipscreen)
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

		m_tmpbitmap.pix16(py, px) = color;

		color1 >>= 1;
		color2 >>= 1;
	}
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( ojankohs )
{
	ojankohs_state *state = machine.driver_data<ojankohs_state>();

	state->m_tilemap = tilemap_create(machine, ojankohs_get_tile_info, tilemap_scan_rows,  8, 4, 64, 64);
//  state->m_videoram = auto_alloc_array(machine, UINT8, 0x1000);
//  state->m_colorram = auto_alloc_array(machine, UINT8, 0x1000);
//  state->m_paletteram = auto_alloc_array(machine, UINT8, 0x800);

	state->save_pointer(NAME(state->m_videoram), 0x1000);
	state->save_pointer(NAME(state->m_colorram), 0x1000);
	state->save_pointer(NAME(state->m_paletteram), 0x800);
}

VIDEO_START( ojankoy )
{
	ojankohs_state *state = machine.driver_data<ojankohs_state>();

	state->m_tilemap = tilemap_create(machine, ojankoy_get_tile_info, tilemap_scan_rows,  8, 4, 64, 64);
//  state->m_videoram = auto_alloc_array(machine, UINT8, 0x2000);
//  state->m_colorram = auto_alloc_array(machine, UINT8, 0x1000);

	state->save_pointer(NAME(state->m_videoram), 0x2000);
	state->save_pointer(NAME(state->m_colorram), 0x1000);
}

VIDEO_START( ojankoc )
{
	ojankohs_state *state = machine.driver_data<ojankohs_state>();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap);
	state->m_videoram = auto_alloc_array(machine, UINT8, 0x8000);
	state->m_paletteram = auto_alloc_array(machine, UINT8, 0x20);

	state->save_pointer(NAME(state->m_videoram), 0x8000);
	state->save_pointer(NAME(state->m_paletteram), 0x20);
	state->save_item(NAME(state->m_tmpbitmap));
}


/******************************************************************************

    Display refresh

******************************************************************************/

SCREEN_UPDATE_IND16( ojankohs )
{
	ojankohs_state *state = screen.machine().driver_data<ojankohs_state>();

	state->m_tilemap->set_scrollx(0, state->m_scrollx);
	state->m_tilemap->set_scrolly(0, state->m_scrolly);

	state->m_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( ojankoc )
{
	ojankohs_state *state = screen.machine().driver_data<ojankohs_state>();
	int offs;

	if (state->m_screen_refresh)
	{
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);

		/* redraw bitmap */
		for (offs = 0; offs < 0x8000; offs++)
		{
			state->ojankoc_videoram_w(*space, offs, state->m_videoram[offs]);
		}
		state->m_screen_refresh = 0;
	}

	copybitmap(bitmap, state->m_tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
