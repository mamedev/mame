/***************************************************************************

    Goal '92 video hardware

***************************************************************************/

#include "emu.h"
#include "includes/goal92.h"

READ16_MEMBER(goal92_state::goal92_fg_bank_r)
{
	return m_fg_bank;
}

WRITE16_MEMBER(goal92_state::goal92_fg_bank_w)
{
	COMBINE_DATA(&m_fg_bank);

	if (ACCESSING_BITS_0_7)
	{
		m_fg_layer->mark_all_dirty();
	}
}

WRITE16_MEMBER(goal92_state::goal92_text_w)
{
	COMBINE_DATA(&m_tx_data[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(goal92_state::goal92_background_w)
{
	COMBINE_DATA(&m_bg_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(goal92_state::goal92_foreground_w)
{
	COMBINE_DATA(&m_fg_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
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

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
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

	state->m_bg_layer->set_transparent_pen(15);
	state->m_fg_layer->set_transparent_pen(15);
	state->m_tx_layer->set_transparent_pen(15);
}

SCREEN_UPDATE_IND16( goal92 )
{
	goal92_state *state = screen.machine().driver_data<goal92_state>();
	state->m_bg_layer->set_scrollx(0, state->m_scrollram[0] + 60);
	state->m_bg_layer->set_scrolly(0, state->m_scrollram[1] + 8);

	if (state->m_fg_bank & 0xff)
	{
		state->m_fg_layer->set_scrollx(0, state->m_scrollram[0] + 60);
		state->m_fg_layer->set_scrolly(0, state->m_scrollram[1] + 8);
	}
	else
	{
		state->m_fg_layer->set_scrollx(0, state->m_scrollram[2] + 60);
		state->m_fg_layer->set_scrolly(0, state->m_scrollram[3] + 8);
	}

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	state->m_bg_layer->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 2);

	if (!(state->m_fg_bank & 0xff))
		draw_sprites(screen.machine(), bitmap, cliprect, 1);

	state->m_fg_layer->draw(bitmap, cliprect, 0, 0);

	if(state->m_fg_bank & 0xff)
		draw_sprites(screen.machine(), bitmap, cliprect, 1);

	draw_sprites(screen.machine(), bitmap, cliprect, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 3);
	state->m_tx_layer->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_VBLANK( goal92 )
{
	// rising edge
	if (vblank_on)
	{
		goal92_state *state = screen.machine().driver_data<goal92_state>();
		memcpy(state->m_buffered_spriteram, state->m_spriteram, 0x400 * 2);
	}
}
