/***************************************************************************

    xain.c

    The priority prom has 7 inputs:

    A0: Text layer (MAP)
    A1: Sprite layer (OBJ)
    A2: BG1
    A3: BG2
    A4-A6:  From CPU priority register

    The 2 bit data output from the prom selects:

    0 - Text layer
    1 - Sprite layer
    2 - BG1
    3 - BG2

    Decoding the prom manually gives the following rules:

    PRI mode 0 - text (top) -> sprite -> bg1 -> bg2 (bottom)
    PRI mode 1 - text (top) -> sprite -> bg2 -> bg1 (bottom)
    PRI mode 2 - bg1 (top) -> sprite -> bg2 -> text (bottom)
    PRI mode 3 - bg2 (top) -> sprite -> bg1 -> text (bottom)
    PRI mode 4 - bg1 (top) -> sprite -> text -> bg2 (bottom)
    PRI mode 5 - bg2 (top) -> sprite -> text -> bg1 (bottom)
    PRI mode 6 - text (top) -> bg1 -> sprite -> bg2 (bottom)
    PRI mode 7 - text (top) -> bg2 -> sprite -> bg1 (bottom)

***************************************************************************/

#include "emu.h"
#include "includes/xain.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( back_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

static TILE_GET_INFO( get_bgram0_tile_info )
{
	xain_state *state = machine.driver_data<xain_state>();
	int attr = state->m_bgram0[tile_index | 0x400];
	SET_TILE_INFO(
			2,
			state->m_bgram0[tile_index] | ((attr & 7) << 8),
			(attr & 0x70) >> 4,
			(attr & 0x80) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_bgram1_tile_info )
{
	xain_state *state = machine.driver_data<xain_state>();
	int attr = state->m_bgram1[tile_index | 0x400];
	SET_TILE_INFO(
			1,
			state->m_bgram1[tile_index] | ((attr & 7) << 8),
			(attr & 0x70) >> 4,
			(attr & 0x80) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_char_tile_info )
{
	xain_state *state = machine.driver_data<xain_state>();
	int attr = state->m_charram[tile_index | 0x400];
	SET_TILE_INFO(
			0,
			state->m_charram[tile_index] | ((attr & 3) << 8),
			(attr & 0xe0) >> 5,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( xain )
{
	xain_state *state = machine.driver_data<xain_state>();
	state->m_bgram0_tilemap = tilemap_create(machine, get_bgram0_tile_info,back_scan,    16,16,32,32);
	state->m_bgram1_tilemap = tilemap_create(machine, get_bgram1_tile_info,back_scan,    16,16,32,32);
	state->m_char_tilemap = tilemap_create(machine, get_char_tile_info,tilemap_scan_rows, 8, 8,32,32);

	state->m_bgram0_tilemap->set_transparent_pen(0);
	state->m_bgram1_tilemap->set_transparent_pen(0);
	state->m_char_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(xain_state::xain_bgram0_w)
{
	m_bgram0[offset] = data;
	m_bgram0_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(xain_state::xain_bgram1_w)
{
	m_bgram1[offset] = data;
	m_bgram1_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(xain_state::xain_charram_w)
{
	m_charram[offset] = data;
	m_char_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(xain_state::xain_scrollxP0_w)
{

	m_scrollxP0[offset] = data;
	m_bgram0_tilemap->set_scrollx(0, m_scrollxP0[0]|(m_scrollxP0[1]<<8));
}

WRITE8_MEMBER(xain_state::xain_scrollyP0_w)
{

	m_scrollyP0[offset] = data;
	m_bgram0_tilemap->set_scrolly(0, m_scrollyP0[0]|(m_scrollyP0[1]<<8));
}

WRITE8_MEMBER(xain_state::xain_scrollxP1_w)
{

	m_scrollxP1[offset] = data;
	m_bgram1_tilemap->set_scrollx(0, m_scrollxP1[0]|(m_scrollxP1[1]<<8));
}

WRITE8_MEMBER(xain_state::xain_scrollyP1_w)
{

	m_scrollyP1[offset] = data;
	m_bgram1_tilemap->set_scrolly(0, m_scrollyP1[0]|(m_scrollyP1[1]<<8));
}


WRITE8_MEMBER(xain_state::xain_flipscreen_w)
{
	flip_screen_set(machine(), data & 1);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	xain_state *state = machine.driver_data<xain_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram_size;offs += 4)
	{
		int sx,sy,flipx,flipy;
		int attr = spriteram[offs+1];
		int numtile = spriteram[offs+2] | ((attr & 7) << 8);
		int color = (attr & 0x38) >> 3;

		sx = 238 - spriteram[offs+3];
		if (sx <= -7) sx += 256;
		sy = 240 - spriteram[offs];
		if (sy <= -7) sy += 256;
		flipx = attr & 0x40;
		flipy = 0;
		if (flip_screen_get(machine))
		{
			sx = 238 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (attr & 0x80)	/* double height */
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					numtile,
					color,
					flipx,flipy,
					sx,flipy ? sy+16:sy-16,0);
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					numtile+1,
					color,
					flipx,flipy,
					sx,sy,0);
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					numtile,
					color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

SCREEN_UPDATE_IND16( xain )
{
	xain_state *state = screen.machine().driver_data<xain_state>();
	switch (state->m_pri&0x7)
	{
	case 0:
		state->m_bgram0_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_bgram1_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 1:
		state->m_bgram1_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_bgram0_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 2:
		state->m_char_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_bgram0_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram1_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 3:
		state->m_char_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_bgram1_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram0_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 4:
		state->m_bgram0_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram1_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 5:
		state->m_bgram1_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram0_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 6:
		state->m_bgram0_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram1_tilemap->draw(bitmap, cliprect, 0,0);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	case 7:
		state->m_bgram1_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(screen.machine(), bitmap,cliprect);
		state->m_bgram0_tilemap->draw(bitmap, cliprect, 0,0);
		state->m_char_tilemap->draw(bitmap, cliprect, 0,0);
		break;
	}
	return 0;
}
