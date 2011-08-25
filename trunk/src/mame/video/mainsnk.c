#include "emu.h"
#include "includes/mainsnk.h"


PALETTE_INIT( mainsnk )
{
	int i;
	int num_colors = 0x400;

	for( i=0; i<num_colors; i++ )
	{
		int bit0=0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i + 2*num_colors] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 2) & 0x01;
		bit1 = (color_prom[i + num_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 3) & 0x01;
		bit3 = (color_prom[i] >> 0) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*num_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 0) & 0x01;
		bit3 = (color_prom[i + num_colors] >> 1) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static TILEMAP_MAPPER( marvins_tx_scan_cols )
{
	// tilemap is 36x28, the central part is from the first RAM page and the
	// extra 4 columns are from the second page
	col -= 2;
	if (col & 0x20)
		return 0x400 + row + ((col & 0x1f) << 5);
	else
		return row + (col << 5);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	mainsnk_state *state = machine.driver_data<mainsnk_state>();
	int code = state->m_fgram[tile_index];

	SET_TILE_INFO(0,
			code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mainsnk_state *state = machine.driver_data<mainsnk_state>();
	int code = (state->m_bgram[tile_index]);

	SET_TILE_INFO(
			0,
			state->m_bg_tile_offset + code,
			0,
			0);
}


VIDEO_START(mainsnk)
{
	mainsnk_state *state = machine.driver_data<mainsnk_state>();

	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,    8, 8, 32, 32);

	tilemap_set_transparent_pen(state->m_tx_tilemap, 15);
	tilemap_set_scrolldy(state->m_tx_tilemap, 8, 8);

	tilemap_set_scrolldx(state->m_bg_tilemap, 16, 16);
	tilemap_set_scrolldy(state->m_bg_tilemap,  8,  8);
}


WRITE8_HANDLER(mainsnk_c600_w)
{
	mainsnk_state *state = space->machine().driver_data<mainsnk_state>();
	int bank;
	int total_elements = space->machine().gfx[0]->total_elements;

	flip_screen_set(space->machine(), ~data & 0x80);

	tilemap_set_palette_offset(state->m_bg_tilemap, (data & 0x07) << 4);
	tilemap_set_palette_offset(state->m_tx_tilemap, (data & 0x07) << 4);

	bank = 0;
	if (total_elements == 0x400)	// mainsnk
		bank = ((data & 0x30) >> 4);
	else if (total_elements == 0x800)	// canvas
		bank = ((data & 0x40) >> 6) | ((data & 0x30) >> 3);

	if (state->m_bg_tile_offset != (bank << 8))
	{
		state->m_bg_tile_offset = bank << 8;
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
	}
}

WRITE8_HANDLER( mainsnk_fgram_w )
{
	mainsnk_state *state = space->machine().driver_data<mainsnk_state>();

	state->m_fgram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap, offset);
}

WRITE8_HANDLER( mainsnk_bgram_w )
{
	mainsnk_state *state = space->machine().driver_data<mainsnk_state>();

	state->m_bgram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}



static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int scrollx, int scrolly )
{
	mainsnk_state *state = machine.driver_data<mainsnk_state>();
	const gfx_element *gfx = machine.gfx[1];
	const UINT8 *source, *finish;
	source =  state->m_spriteram;
	finish =  source + 25*4;

	while( source<finish )
	{
		int attributes = source[3];
		int tile_number = source[1];
		int sy = source[0];
		int sx = source[2];
		int color = attributes&0xf;
		int flipx = 0;
		int flipy = 0;
		if( sy>240 ) sy -= 256;

		tile_number |= attributes<<4 & 0x300;

		sx = 288-16 - sx;
		sy += 8;

		if (flip_screen_get(machine))
		{
			sx = 288-16 - sx;
			sy = 224-16 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen( bitmap,cliprect,gfx,
			tile_number,
			color,
			flipx,flipy,
			sx,sy,7);

		source+=4;
	}
}


SCREEN_UPDATE(mainsnk)
{
	mainsnk_state *state = screen->machine().driver_data<mainsnk_state>();

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	return 0;
}
