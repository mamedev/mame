/***************************************************************************

    Bionic Commando Video Hardware

    This board handles tile/tile and tile/sprite priority with a PROM. Its
    working is complicated and hardcoded in the driver.

    The PROM is a 256x4 chip, with address inputs wired as follows:

    A0 bg opaque
    A1 \
    A2 |  fg pen
    A3 |
    A4 /
    A5 fg has priority over sprites (bit 5 of tile attribute)
    A6 fg has not priority over bg (bits 6 & 7 of tile attribute both set)
    A7 sprite opaque

    The output selects the active layer, it can be:
    0  bg
    1  fg
    2  sprite

***************************************************************************/

#include "emu.h"
#include "includes/bionicc.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	bionicc_state *state = machine.driver_data<bionicc_state>();

	int attr = state->m_bgvideoram[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			(state->m_bgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			TILE_FLIPXY((attr & 0xc0) >> 6));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bionicc_state *state = machine.driver_data<bionicc_state>();

	int attr = state->m_fgvideoram[2 * tile_index + 1];
	int flags;

	if ((attr & 0xc0) == 0xc0)
	{
		tileinfo.category = 1;
		tileinfo.group = 0;
		flags = 0;
	}
	else
	{
		tileinfo.category = 0;
		tileinfo.group = (attr & 0x20) >> 5;
		flags = TILE_FLIPXY((attr & 0xc0) >> 6);
	}

	SET_TILE_INFO(
			2,
			(state->m_fgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			flags);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	bionicc_state *state = machine.driver_data<bionicc_state>();

	int attr = state->m_txvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			(state->m_txvideoram[tile_index] & 0xff) + ((attr & 0x00c0) << 2),
			attr & 0x3f,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bionicc )
{
	bionicc_state *state = machine.driver_data<bionicc_state>();

	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,  8, 8, 64, 64);

	state->m_tx_tilemap->set_transparent_pen(3);
	state->m_fg_tilemap->set_transmask(0, 0xffff, 0x8000); /* split type 0 is completely transparent in front half */
	state->m_fg_tilemap->set_transmask(1, 0xffc1, 0x803e); /* split type 1 has pens 1-5 opaque in front half */
	state->m_bg_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(bionicc_state::bionicc_bgvideoram_w)
{

	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(bionicc_state::bionicc_fgvideoram_w)
{

	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(bionicc_state::bionicc_txvideoram_w)
{

	COMBINE_DATA(&m_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE16_MEMBER(bionicc_state::bionicc_paletteram_w)
{
	int r, g, b, bright;
	data = COMBINE_DATA(&m_paletteram[offset]);

	bright = (data & 0x0f);

	r = ((data >> 12) & 0x0f) * 0x11;
	g = ((data >> 8 ) & 0x0f) * 0x11;
	b = ((data >> 4 ) & 0x0f) * 0x11;

	if ((bright & 0x08) == 0)
	{
		r = r * (0x07 + bright) / 0x0e;
		g = g * (0x07 + bright) / 0x0e;
		b = b * (0x07 + bright) / 0x0e;
	}

	palette_set_color (machine(), offset, MAKE_RGB(r, g, b));
}

WRITE16_MEMBER(bionicc_state::bionicc_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:
			m_fg_tilemap->set_scrollx(0, data);
			break;
		case 1:
			m_fg_tilemap->set_scrolly(0, data);
			break;
		case 2:
			m_bg_tilemap->set_scrollx(0, data);
			break;
		case 3:
			m_bg_tilemap->set_scrolly(0, data);
			break;
	}
}

WRITE16_MEMBER(bionicc_state::bionicc_gfxctrl_w)
{

	if (ACCESSING_BITS_8_15)
	{
		flip_screen_set(machine(), data & 0x0100);

		m_bg_tilemap->enable(data & 0x2000);	/* guess */
		m_fg_tilemap->enable(data & 0x1000);	/* guess */

		coin_counter_w(machine(), 0, data & 0x8000);
		coin_counter_w(machine(), 1, data & 0x4000);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bionicc_state *state = machine.driver_data<bionicc_state>();
	UINT16 *buffered_spriteram = state->m_spriteram->buffer();
	int offs;
	const gfx_element *gfx = machine.gfx[3];

	for (offs = (state->m_spriteram->bytes() - 8) / 2; offs >= 0; offs -= 4)
	{
		int tile_number = buffered_spriteram[offs] & 0x7ff;
		if( tile_number != 0x7ff )
		{
			int attr = buffered_spriteram[offs + 1];
			int color = (attr & 0x3c) >> 2;
			int flipx = attr & 0x02;
			int flipy = 0;
			int sx = (INT16)buffered_spriteram[offs + 3];	/* signed */
			int sy = (INT16)buffered_spriteram[offs + 2];	/* signed */

			if (sy > 512 - 16)
				sy -= 512;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen( bitmap, cliprect,gfx,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,15);
		}
	}
}

SCREEN_UPDATE_IND16( bionicc )
{
	bionicc_state *state = screen.machine().driver_data<bionicc_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);	/* nothing in FRONT */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
