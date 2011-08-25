#include "emu.h"
#include "includes/raiden.h"


/******************************************************************************/

WRITE16_HANDLER( raiden_background_w )
{
	raiden_state *state = space->machine().driver_data<raiden_state>();
	COMBINE_DATA(&state->m_back_data[offset]);
	tilemap_mark_tile_dirty(state->m_bg_layer, offset);
}

WRITE16_HANDLER( raiden_foreground_w )
{
	raiden_state *state = space->machine().driver_data<raiden_state>();
	COMBINE_DATA(&state->m_fore_data[offset]);
	tilemap_mark_tile_dirty(state->m_fg_layer, offset);
}

WRITE16_HANDLER( raiden_text_w )
{
	raiden_state *state = space->machine().driver_data<raiden_state>();
	UINT16 *videoram = state->m_videoram;
	COMBINE_DATA(&videoram[offset]);
	tilemap_mark_tile_dirty(state->m_tx_layer, offset);
}

static TILE_GET_INFO( get_back_tile_info )
{
	raiden_state *state = machine.driver_data<raiden_state>();
	int tile=state->m_back_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	raiden_state *state = machine.driver_data<raiden_state>();
	int tile=state->m_fore_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	raiden_state *state = machine.driver_data<raiden_state>();
	UINT16 *videoram = state->m_videoram;
	int tiledata = videoram[tile_index];
	int tile = (tiledata & 0xff) | ((tiledata >> 6) & 0x300);
	int color = (tiledata >> 8) & 0x0f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( raiden )
{
	raiden_state *state = machine.driver_data<raiden_state>();
	state->m_bg_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_cols,     16,16,32,32);
	state->m_fg_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_cols,16,16,32,32);
	state->m_tx_layer = tilemap_create(machine, get_text_tile_info,tilemap_scan_cols,8,8,32,32);
	state->m_alternate=0;

	tilemap_set_transparent_pen(state->m_fg_layer,15);
	tilemap_set_transparent_pen(state->m_tx_layer,15);
}

VIDEO_START( raidena )
{
	raiden_state *state = machine.driver_data<raiden_state>();
	state->m_bg_layer = tilemap_create(machine, get_back_tile_info,tilemap_scan_cols,     16,16,32,32);
	state->m_fg_layer = tilemap_create(machine, get_fore_tile_info,tilemap_scan_cols,16,16,32,32);
	state->m_tx_layer = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,32,32);
	state->m_alternate=1;

	tilemap_set_transparent_pen(state->m_fg_layer,15);
	tilemap_set_transparent_pen(state->m_tx_layer,15);
}

WRITE16_HANDLER( raiden_control_w )
{
	raiden_state *state = space->machine().driver_data<raiden_state>();
	/* All other bits unknown - could be playfield enables */

	/* Flipscreen */
	if (offset==3 && ACCESSING_BITS_0_7) {
		state->m_flipscreen=data&0x2;
		tilemap_set_flip_all(space->machine(),state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
}

WRITE16_HANDLER( raidena_control_w )
{
	raiden_state *state = space->machine().driver_data<raiden_state>();
	/* raidena uses 0x40 instead of 0x02 */

	/* Flipscreen */
	if (offset==3 && ACCESSING_BITS_0_7) {
		state->m_flipscreen=data&0x40;
		tilemap_set_flip_all(space->machine(),state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect,int pri_mask)
{
	raiden_state *state = machine.driver_data<raiden_state>();
	UINT16 *buffered_spriteram16 = machine.generic.buffered_spriteram.u16;
	int offs,fx,fy,x,y,color,sprite;

	for (offs = 0x1000/2-4;offs >= 0;offs -= 4)
	{
		if (!(pri_mask&(buffered_spriteram16[offs+2]>>8))) continue;

		fx    = buffered_spriteram16[offs+0] & 0x2000;
		fy    = buffered_spriteram16[offs+0] & 0x4000;
		color = (buffered_spriteram16[offs+0] & 0x0f00) >> 8;
		y = buffered_spriteram16[offs+0] & 0x00ff;

		sprite = buffered_spriteram16[offs+1];
		sprite &= 0x0fff;

		x = buffered_spriteram16[offs+2] & 0xff;
		if (buffered_spriteram16[offs+2] & 0x100) x=0-(0x100-x);

		if (state->m_flipscreen) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
				sprite,
				color,fx,fy,x,y,15);
	}
}

SCREEN_UPDATE( raiden )
{
	raiden_state *state = screen->machine().driver_data<raiden_state>();
	/* Setup the tilemaps, alternate version has different scroll positions */
	if (!state->m_alternate) {
		tilemap_set_scrollx( state->m_bg_layer,0, state->m_scroll_ram[0]);
		tilemap_set_scrolly( state->m_bg_layer,0, state->m_scroll_ram[1]);
		tilemap_set_scrollx( state->m_fg_layer,0, state->m_scroll_ram[2]);
		tilemap_set_scrolly( state->m_fg_layer,0, state->m_scroll_ram[3]);
	}
	else {
		tilemap_set_scrolly( state->m_bg_layer,0, ((state->m_scroll_ram[0x01]&0x30)<<4)+((state->m_scroll_ram[0x02]&0x7f)<<1)+((state->m_scroll_ram[0x02]&0x80)>>7) );
		tilemap_set_scrollx( state->m_bg_layer,0, ((state->m_scroll_ram[0x09]&0x30)<<4)+((state->m_scroll_ram[0x0a]&0x7f)<<1)+((state->m_scroll_ram[0x0a]&0x80)>>7) );
		tilemap_set_scrolly( state->m_fg_layer,0, ((state->m_scroll_ram[0x11]&0x30)<<4)+((state->m_scroll_ram[0x12]&0x7f)<<1)+((state->m_scroll_ram[0x12]&0x80)>>7) );
		tilemap_set_scrollx( state->m_fg_layer,0, ((state->m_scroll_ram[0x19]&0x30)<<4)+((state->m_scroll_ram[0x1a]&0x7f)<<1)+((state->m_scroll_ram[0x1a]&0x80)>>7) );
	}

	tilemap_draw(bitmap,cliprect,state->m_bg_layer,0,0);

	/* Draw sprites underneath foreground */
	draw_sprites(screen->machine(),bitmap,cliprect,0x40);
	tilemap_draw(bitmap,cliprect,state->m_fg_layer,0,0);

	/* Rest of sprites */
	draw_sprites(screen->machine(),bitmap,cliprect,0x80);

	/* Text layer */
	tilemap_draw(bitmap,cliprect,state->m_tx_layer,0,0);
	return 0;
}
