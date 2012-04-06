/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/lastduel.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( ld_get_bg_tile_info )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	int tile = state->m_scroll2[2 * tile_index] & 0x1fff;
	int color = state->m_scroll2[2 * tile_index + 1];
	SET_TILE_INFO(
			2,
			tile,color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

static TILE_GET_INFO( ld_get_fg_tile_info )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	int tile = state->m_scroll1[2 * tile_index] & 0x1fff;
	int color = state->m_scroll1[2 * tile_index + 1];
	SET_TILE_INFO(
			3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x80) >> 7;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	int tile = state->m_scroll2[tile_index] & 0x1fff;
	int color = state->m_scroll2[tile_index + 0x0800];
	SET_TILE_INFO(
			2,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	int tile = state->m_scroll1[tile_index] & 0x1fff;
	int color = state->m_scroll1[tile_index + 0x0800];
	SET_TILE_INFO(
			3,
			tile,
			color & 0xf,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = (color & 0x10) >> 4;
}

static TILE_GET_INFO( get_fix_info )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	int tile = state->m_vram[tile_index];
	SET_TILE_INFO(
			1,
			tile & 0x7ff,
			tile>>12,
			(tile & 0x800) ? TILE_FLIPY : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( lastduel )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	state->m_bg_tilemap = tilemap_create(machine, ld_get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, ld_get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_tx_tilemap = tilemap_create(machine, get_fix_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transmask(0, 0xffff, 0x0001);
	state->m_fg_tilemap->set_transmask(1, 0xf07f, 0x0f81);
	state->m_tx_tilemap->set_transparent_pen(3);

	state->m_sprite_flipy_mask = 0x40;
	state->m_sprite_pri_mask = 0x00;
	state->m_tilemap_priority = 0;
}

VIDEO_START( madgear )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,16,16,64,32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,16,16,64,32);
	state->m_tx_tilemap = tilemap_create(machine, get_fix_info,tilemap_scan_rows,8,8,64,32);

	state->m_fg_tilemap->set_transmask(0, 0xffff, 0x8000);
	state->m_fg_tilemap->set_transmask(1, 0x80ff, 0xff00);
	state->m_tx_tilemap->set_transparent_pen(3);
	state->m_bg_tilemap->set_transparent_pen(15);

	state->m_sprite_flipy_mask = 0x80;
	state->m_sprite_pri_mask = 0x10;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(lastduel_state::lastduel_flip_w)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(machine(), data & 0x01);

		coin_lockout_w(machine(), 0, ~data & 0x10);
		coin_lockout_w(machine(), 1, ~data & 0x20);
		coin_counter_w(machine(), 0, data & 0x40);
		coin_counter_w(machine(), 1, data & 0x80);
	}
}

WRITE16_MEMBER(lastduel_state::lastduel_scroll_w)
{

	data = COMBINE_DATA(&m_scroll[offset]);
	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrolly(0, data); break;
		case 1: m_fg_tilemap->set_scrollx(0, data); break;
		case 2: m_bg_tilemap->set_scrolly(0, data); break;
		case 3: m_bg_tilemap->set_scrollx(0, data); break;
		case 7: m_tilemap_priority = data; break;
		default:
			logerror("Unmapped video write %d %04x\n", offset, data);
			break;
	}
}

WRITE16_MEMBER(lastduel_state::lastduel_scroll1_w)
{

	COMBINE_DATA(&m_scroll1[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(lastduel_state::lastduel_scroll2_w)
{

	COMBINE_DATA(&m_scroll2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(lastduel_state::lastduel_vram_w)
{

	COMBINE_DATA(&m_vram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(lastduel_state::madgear_scroll1_w)
{

	COMBINE_DATA(&m_scroll1[offset]);
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE16_MEMBER(lastduel_state::madgear_scroll2_w)
{

	COMBINE_DATA(&m_scroll2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE16_MEMBER(lastduel_state::lastduel_palette_word_w)
{

	int red, green, blue, bright;
	data = COMBINE_DATA(&m_paletteram[offset]);

	// Brightness parameter interpreted same way as CPS1
	bright = 0x10 + (data & 0x0f);

	red   = ((data >> 12) & 0x0f) * bright * 0x11 / 0x1f;
	green = ((data >> 8)  & 0x0f) * bright * 0x11 / 0x1f;
	blue  = ((data >> 4)  & 0x0f) * bright * 0x11 / 0x1f;

	palette_set_color (machine(), offset, MAKE_RGB(red, green, blue));
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	lastduel_state *state = machine.driver_data<lastduel_state>();

	UINT16 *buffered_spriteram16 = state->m_spriteram->buffer();
	int offs;

	if (!state->m_sprite_pri_mask)
		if (pri == 1)
			return;	/* only low priority sprites in lastduel */

	for (offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		int attr, sy, sx, flipx, flipy, code, color;

		attr = buffered_spriteram16[offs + 1];
		if (state->m_sprite_pri_mask)	/* only madgear seems to have this */
		{
			if (pri == 1 && (attr & state->m_sprite_pri_mask))
				continue;
			if (pri == 0 && !(attr & state->m_sprite_pri_mask))
				continue;
		}

		code = buffered_spriteram16[offs];
		sx = buffered_spriteram16[offs + 3] & 0x1ff;
		sy = buffered_spriteram16[offs + 2] & 0x1ff;
		if (sy > 0x100)
			sy -= 0x200;

		flipx = attr & 0x20;
		flipy = attr & state->m_sprite_flipy_mask;	/* 0x40 for lastduel, 0x80 for madgear */
		color = attr & 0x0f;

		if (flip_screen_get(machine))
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,
				machine.gfx[0],
				code,
				color,
				flipx,flipy,
				sx,sy,15);
	}
}

SCREEN_UPDATE_IND16( lastduel )
{
	lastduel_state *state = screen.machine().driver_data<lastduel_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 1);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( madgear )
{
	lastduel_state *state = screen.machine().driver_data<lastduel_state>();

	if (state->m_tilemap_priority)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1 | TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
	}
	else
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect, 1);
	}
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
