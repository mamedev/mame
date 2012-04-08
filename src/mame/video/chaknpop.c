/*
 *  Chack'n Pop (C) 1983 TAITO Corp.
 *  emulate video hardware
 */

#include "emu.h"
#include "includes/chaknpop.h"

#define GFX_FLIP_X	0x01
#define GFX_FLIP_Y	0x02
#define GFX_VRAM_BANK	0x04
#define GFX_UNKNOWN1	0x08
#define GFX_TX_BANK1	0x20
#define GFX_UNKNOWN2	0x40
#define GFX_TX_BANK2	0x80

#define TX_COLOR1	0x0b
#define TX_COLOR2	0x01


/***************************************************************************
  palette decode
***************************************************************************/

PALETTE_INIT( chaknpop )
{
	int i;

	for (i = 0; i < 1024; i++)
	{
		int col, r, g, b;
		int bit0, bit1, bit2;

		col = (color_prom[i] & 0x0f) + ((color_prom[i + 1024] & 0x0f) << 4);

		/* red component */
		bit0 = (col >> 0) & 0x01;
		bit1 = (col >> 1) & 0x01;
		bit2 = (col >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (col >> 3) & 0x01;
		bit1 = (col >> 4) & 0x01;
		bit2 = (col >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (col >> 6) & 0x01;
		bit2 = (col >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

/***************************************************************************
  Memory handlers
***************************************************************************/

static void tx_tilemap_mark_all_dirty( running_machine &machine )
{
	chaknpop_state *state = machine.driver_data<chaknpop_state>();

	state->m_tx_tilemap->mark_all_dirty();
	state->m_tx_tilemap->set_flip(state->m_flip_x | state->m_flip_y);
}

READ8_MEMBER(chaknpop_state::chaknpop_gfxmode_r)
{
	return m_gfxmode;
}

WRITE8_MEMBER(chaknpop_state::chaknpop_gfxmode_w)
{

	if (m_gfxmode != data)
	{
		int all_dirty = 0;

		m_gfxmode = data;
		memory_set_bank(machine(), "bank1", (m_gfxmode & GFX_VRAM_BANK) ? 1 : 0);	/* Select 2 banks of 16k */

		if (m_flip_x != (m_gfxmode & GFX_FLIP_X))
		{
			m_flip_x = m_gfxmode & GFX_FLIP_X;
			all_dirty = 1;
		}

		if (m_flip_y != (m_gfxmode & GFX_FLIP_Y))
		{
			m_flip_y = m_gfxmode & GFX_FLIP_Y;
			all_dirty = 1;
		}

		if (all_dirty)
			tx_tilemap_mark_all_dirty(machine());
	}
}

WRITE8_MEMBER(chaknpop_state::chaknpop_txram_w)
{

	m_tx_ram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(chaknpop_state::chaknpop_attrram_w)
{

	if (m_attr_ram[offset] != data)
	{
		m_attr_ram[offset] = data;

		if (offset == TX_COLOR1 || offset == TX_COLOR2)
			tx_tilemap_mark_all_dirty(machine());
	}
}


/***************************************************************************
  Callback for the tilemap code
***************************************************************************/

/*
 *  I'm not sure how to handle attributes about color
 */

static TILE_GET_INFO( chaknpop_get_tx_tile_info )
{
	chaknpop_state *state = machine.driver_data<chaknpop_state>();
	int tile = state->m_tx_ram[tile_index];
	int tile_h_bank = (state->m_gfxmode & GFX_TX_BANK2) << 2;	/* 0x00-0xff -> 0x200-0x2ff */
	int color = state->m_attr_ram[TX_COLOR2];

	if (tile == 0x74)
		color = state->m_attr_ram[TX_COLOR1];

	if (state->m_gfxmode & GFX_TX_BANK1 && tile >= 0xc0)
		tile += 0xc0;					/* 0xc0-0xff -> 0x180-0x1bf */

	tile |= tile_h_bank;

	SET_TILE_INFO(1, tile, color, 0);
}


/***************************************************************************
  Initialize video hardware emulation
***************************************************************************/

VIDEO_START( chaknpop )
{
	chaknpop_state *state = machine.driver_data<chaknpop_state>();
	UINT8 *RAM = machine.region("maincpu")->base();

	/*                          info                       offset             type             w   h  col row */
	state->m_tx_tilemap = tilemap_create(machine, chaknpop_get_tx_tile_info, tilemap_scan_rows,   8,  8, 32, 32);

	state->m_vram1 = &RAM[0x10000];
	state->m_vram2 = &RAM[0x12000];
	state->m_vram3 = &RAM[0x14000];
	state->m_vram4 = &RAM[0x16000];

	state->save_pointer(NAME(state->m_vram1), 0x2000);
	state->save_pointer(NAME(state->m_vram2), 0x2000);
	state->save_pointer(NAME(state->m_vram3), 0x2000);
	state->save_pointer(NAME(state->m_vram4), 0x2000);

	memory_set_bank(machine, "bank1", 0);
	tx_tilemap_mark_all_dirty(machine);

	machine.save().register_postload(save_prepost_delegate(FUNC(tx_tilemap_mark_all_dirty), &machine));
}


/***************************************************************************
  Screen refresh
***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	chaknpop_state *state = machine.driver_data<chaknpop_state>();
	int offs;

	/* Draw the sprites */
	for (offs = 0; offs < state->m_spr_ram_size; offs += 4)
	{
		int sx = state->m_spr_ram[offs + 3];
		int sy = 256 - 15 - state->m_spr_ram[offs];
		int flipx = state->m_spr_ram[offs+1] & 0x40;
		int flipy = state->m_spr_ram[offs+1] & 0x80;
		int color = (state->m_spr_ram[offs + 2] & 7);
		int tile = (state->m_spr_ram[offs + 1] & 0x3f) | ((state->m_spr_ram[offs + 2] & 0x38) << 3);

		if (state->m_flip_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (state->m_flip_y)
		{
			sy = 242 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,
				machine.gfx[0],
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

static void draw_bitmap( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	chaknpop_state *state = machine.driver_data<chaknpop_state>();
	int dx = state->m_flip_x ? -1 : 1;
	int offs, i;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int x = ((offs & 0x1f) << 3) + 7;
		int y = offs >> 5;

		if (!state->m_flip_x)
			x = 255 - x;

		if (!state->m_flip_y)
			y = 255 - y;

		for (i = 0x80; i > 0; i >>= 1, x += dx)
		{
			pen_t color = 0;

			if (state->m_vram1[offs] & i)
				color |= 0x200;	// green lower cage
			if (state->m_vram2[offs] & i)
				color |= 0x080;
			if (state->m_vram3[offs] & i)
				color |= 0x100;	// green upper cage
			if (state->m_vram4[offs] & i)
				color |= 0x040;	// tx mask

			if (color)
			{
				pen_t pen = bitmap.pix16(y, x);
				pen |= color;
				bitmap.pix16(y, x) = pen;
			}
		}
	}
}

SCREEN_UPDATE_IND16( chaknpop )
{
	chaknpop_state *state = screen.machine().driver_data<chaknpop_state>();

	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	return 0;
}
