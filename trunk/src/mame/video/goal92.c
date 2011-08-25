/***************************************************************************

    Goal '92 video hardware

***************************************************************************/

#include "emu.h"
#include "includes/goal92.h"

READ16_HANDLER( goal92_fg_bank_r )
{
	goal92_state *state = space->machine().driver_data<goal92_state>();
	return state->m_fg_bank;
}

WRITE16_HANDLER( goal92_fg_bank_w )
{
	goal92_state *state = space->machine().driver_data<goal92_state>();
	COMBINE_DATA(&state->m_fg_bank);

	if (ACCESSING_BITS_0_7)
	{
		tilemap_mark_all_tiles_dirty(state->m_fg_layer);
	}
}

WRITE16_HANDLER( goal92_text_w )
{
	goal92_state *state = space->machine().driver_data<goal92_state>();
	COMBINE_DATA(&state->m_tx_data[offset]);
	tilemap_mark_tile_dirty(state->m_tx_layer, offset);
}

WRITE16_HANDLER( goal92_background_w )
{
	goal92_state *state = space->machine().driver_data<goal92_state>();
	COMBINE_DATA(&state->m_bg_data[offset]);
	tilemap_mark_tile_dirty(state->m_bg_layer, offset);
}

WRITE16_HANDLER( goal92_foreground_w )
{
	goal92_state *state = space->machine().driver_data<goal92_state>();
	COMBINE_DATA(&state->m_fg_data[offset]);
	tilemap_mark_tile_dirty(state->m_fg_layer, offset);
}

static TILE_GET_INFO( get_text_tile_info )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	int tile = state->m_tx_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tile |= 0xc000;

	SET_TILE_INFO(1, tile, color, 0);
}

static TILE_GET_INFO( get_back_tile_info )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	int tile = state->m_bg_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO(2, tile, color, 0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	int tile = state->m_fg_data[tile_index];
	int color = (tile >> 12) & 0xf;
	int region;

	tile &= 0xfff;

	if(state->m_fg_bank & 0xff)
	{
		region = 3;
		tile |= 0x1000;
	}
	else
	{
		region = 4;
		tile |= 0x2000;
	}

	SET_TILE_INFO(region, tile, color, 0);
}

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	UINT16 *buffered_spriteram16 = state->m_buffered_spriteram;
	int offs, fx, fy, x, y, color, sprite;

	for (offs = 3; offs <= 0x400 - 5; offs += 4)
	{
		UINT16 data = buffered_spriteram16[offs + 2];

		y = buffered_spriteram16[offs + 0];

		if (y & 0x8000)
			break;

		if (!(data & 0x8000))
			continue;

		sprite = buffered_spriteram16[offs + 1];

		if ((sprite >> 14) != pri)
			continue;

		x = buffered_spriteram16[offs + 3];

		sprite &= 0x1fff;

		x &= 0x1ff;
		y &= 0x1ff;

		color = (data & 0x3f) + 0x40;
		fx = (data & 0x4000) >> 14;
		fy = 0;

		x -= 320 / 4 - 16 - 1;

		y = 256 - (y + 7);

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				sprite,
				color,fx,fy,x,y,15);
	}
}


VIDEO_START( goal92 )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	state->m_bg_layer = tilemap_create(machine, get_back_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_layer = tilemap_create(machine, get_fore_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tx_layer = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_buffered_spriteram = auto_alloc_array(machine, UINT16, 0x400 * 2);
	state_save_register_global_pointer(machine, state->m_buffered_spriteram, 0x400 * 2);

	tilemap_set_transparent_pen(state->m_bg_layer, 15);
	tilemap_set_transparent_pen(state->m_fg_layer, 15);
	tilemap_set_transparent_pen(state->m_tx_layer, 15);
}

SCREEN_UPDATE( goal92 )
{
	goal92_state *state = screen->machine().driver_data<goal92_state>();
	tilemap_set_scrollx(state->m_bg_layer, 0, state->m_scrollram[0] + 60);
	tilemap_set_scrolly(state->m_bg_layer, 0, state->m_scrollram[1] + 8);

	if (state->m_fg_bank & 0xff)
	{
		tilemap_set_scrollx(state->m_fg_layer, 0, state->m_scrollram[0] + 60);
		tilemap_set_scrolly(state->m_fg_layer, 0, state->m_scrollram[1] + 8);
	}
	else
	{
		tilemap_set_scrollx(state->m_fg_layer, 0, state->m_scrollram[2] + 60);
		tilemap_set_scrolly(state->m_fg_layer, 0, state->m_scrollram[3] + 8);
	}

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	tilemap_draw(bitmap, cliprect, state->m_bg_layer, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect, 2);

	if (!(state->m_fg_bank & 0xff))
		draw_sprites(screen->machine(), bitmap, cliprect, 1);

	tilemap_draw(bitmap, cliprect, state->m_fg_layer, 0, 0);

	if(state->m_fg_bank & 0xff)
		draw_sprites(screen->machine(), bitmap, cliprect, 1);

	draw_sprites(screen->machine(), bitmap, cliprect, 0);
	draw_sprites(screen->machine(), bitmap, cliprect, 3);
	tilemap_draw(bitmap, cliprect, state->m_tx_layer, 0, 0);
	return 0;
}

SCREEN_EOF( goal92 )
{
	goal92_state *state = machine.driver_data<goal92_state>();
	memcpy(state->m_buffered_spriteram, state->m_spriteram, 0x400 * 2);
}
