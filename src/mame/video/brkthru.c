/***************************************************************************

    video/brkthru.c

***************************************************************************/

#include "emu.h"
#include "includes/brkthru.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Break Thru has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( brkthru )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));

		color_prom++;
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	brkthru_state *state = machine.driver_data<brkthru_state>();
	/* BG RAM format
        0         1
        ---- -c-- ---- ---- = Color
        ---- --xx xxxx xxxx = Code
    */

	int code = (state->m_videoram[tile_index * 2] | ((state->m_videoram[tile_index * 2 + 1]) << 8)) & 0x3ff;
	int region = 1 + (code >> 7);
	int colour = state->m_bgbasecolor + ((state->m_videoram[tile_index * 2 + 1] & 0x04) >> 2);

	SET_TILE_INFO(region, code & 0x7f, colour,0);
}

WRITE8_MEMBER(brkthru_state::brkthru_bgram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	brkthru_state *state = machine.driver_data<brkthru_state>();
	UINT8 code = state->m_fg_videoram[tile_index];
	SET_TILE_INFO(0, code, 0, 0);
}

WRITE8_MEMBER(brkthru_state::brkthru_fgram_w)
{

	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

VIDEO_START( brkthru )
{
	brkthru_state *state = machine.driver_data<brkthru_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 16);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_transparent_pen(0);
}


WRITE8_MEMBER(brkthru_state::brkthru_1800_w)
{

	if (offset == 0)	/* low 8 bits of scroll */
		m_bgscroll = (m_bgscroll & 0x100) | data;
	else if (offset == 1)
	{
		/* bit 0-2 = ROM bank select */
		memory_set_bank(machine(), "bank1", data & 0x07);

		/* bit 3-5 = background tiles color code */
		if (((data & 0x38) >> 2) != m_bgbasecolor)
		{
			m_bgbasecolor = (data & 0x38) >> 2;
			m_bg_tilemap->mark_all_dirty();
		}

		/* bit 6 = screen flip */
		if (m_flipscreen != (data & 0x40))
		{
			m_flipscreen = data & 0x40;
			m_bg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_fg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		}

		/* bit 7 = high bit of scroll */
		m_bgscroll = (m_bgscroll & 0xff) | ((data & 0x80) << 1);
	}
}


#if 0
static void show_register( bitmap_ind16 &bitmap, int x, int y, UINT32 data )
{
	char buf[5];

	sprintf(buf, "%04X", data);
	ui_draw_text(y, x, buf);
}
#endif


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int prio )
{
	brkthru_state *state = machine.driver_data<brkthru_state>();
	int offs;
	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */

	/* Sprite RAM format
        0         1         2         3
        ccc- ---- ---- ---- ---- ---- ---- ---- = Color
        ---d ---- ---- ---- ---- ---- ---- ---- = Double Size
        ---- p--- ---- ---- ---- ---- ---- ---- = Priority
        ---- -bb- ---- ---- ---- ---- ---- ---- = Bank
        ---- ---e ---- ---- ---- ---- ---- ---- = Enable/Disable
        ---- ---- ssss ssss ---- ---- ---- ---- = Sprite code
        ---- ---- ---- ---- yyyy yyyy ---- ---- = Y position
        ---- ---- ---- ---- ---- ---- xxxx xxxx = X position
    */

	for (offs = 0;offs < state->m_spriteram_size; offs += 4)
	{
		if ((state->m_spriteram[offs] & 0x09) == prio)	/* Enable && Low Priority */
		{
			int sx, sy, code, color;

			sx = 240 - state->m_spriteram[offs + 3];
			if (sx < -7)
				sx += 256;

			sy = 240 - state->m_spriteram[offs + 2];
			code = state->m_spriteram[offs + 1] + 128 * (state->m_spriteram[offs] & 0x06);
			color = (state->m_spriteram[offs] & 0xe0) >> 5;
			if (state->m_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (state->m_spriteram[offs] & 0x10)	/* double height */
			{
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code & ~1,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx, state->m_flipscreen ? sy + 16 : sy - 16,0);
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code | 1,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code & ~1,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx,(state->m_flipscreen ? sy + 16 : sy - 16) + 256,0);
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code | 1,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx,sy + 256,0);

			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine.gfx[9],
						code,
						color,
						state->m_flipscreen, state->m_flipscreen,
						sx,sy + 256,0);

			}
		}
	}
}

SCREEN_UPDATE_IND16( brkthru )
{
	brkthru_state *state = screen.machine().driver_data<brkthru_state>();

	state->m_bg_tilemap->set_scrollx(0, state->m_bgscroll);
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	/* low priority sprites */
	draw_sprites(screen.machine(), bitmap, cliprect, 0x01);

	/* draw background over low priority sprites */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* high priority sprites */
	draw_sprites(screen.machine(), bitmap, cliprect, 0x09);

	/* fg layer */
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

/*  show_register(bitmap, 8, 8, (UINT32)state->m_flipscreen); */

	return 0;
}
