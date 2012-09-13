/***************************************************************************

 Lasso and similar hardware

    driver by Phil Stroffolino, Nicola Salmoria, Luca Elia


    Every game has 1 256 x 256 tilemap (non scrollable) made of 8 x 8
    tiles, and 16 x 16 sprites (some games use 32, some more).

    The graphics for tiles and sprites are held inside the same ROMs,
    but aren't shared between the two:

    the first $100 tiles are for the tilemap, the following $100 are
    for sprites. This constitutes the first graphics bank. There can
    be several.

    Lasso has an additional pixel layer (256 x 256 x 1) and a third
    CPU devoted to drawing into it (the lasso!)

    Wwjgtin has an additional $800 x $400 scrolling tilemap in ROM
    and $100 more 16 x 16 x 4 tiles for it.

    The colors are static ($40 colors, 2 PROMs) but the background
    color can be changed at runtime. Wwjgtin can change the last
    4 colors (= last palette) too.

***************************************************************************/

#include "emu.h"
#include "includes/lasso.h"

/***************************************************************************


                            Colors (BBGGGRRR)


***************************************************************************/

static rgb_t get_color( int data )
{
	int bit0, bit1, bit2;
	int r, g, b;

	/* red component */
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* green component */
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* blue component */
	bit0 = (data >> 6) & 0x01;
	bit1 = (data >> 7) & 0x01;
	b = 0x4f * bit0 + 0xa8 * bit1;

	return MAKE_RGB(r, g, b);
}


void lasso_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < 0x40; i++)
		palette_set_color(machine(), i, get_color(color_prom[i]));
}


PALETTE_INIT_MEMBER(lasso_state,wwjgtin)
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x40);

	for (i = 0; i < 0x40; i++)
		colortable_palette_set_color(machine().colortable, i, get_color(color_prom[i]));

	/* characters/sprites */
	for (i = 0; i < 0x40; i++)
		colortable_entry_set_value(machine().colortable, i, i);

	/* track */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry;

		if ((i - 0x40) & 0x03)
			ctabentry = ((((i - 0x40) & 0xf0) >> 2) + ((i - 0x40) & 0x0f)) & 0x3f;
		else
			ctabentry = 0;

		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}


static void wwjgtin_set_last_four_colors( running_machine &machine, colortable_t *colortable )
{
	lasso_state *state = machine.driver_data<lasso_state>();
	int i;

	/* the last palette entries can be changed */
	for(i = 0; i < 3; i++)
		colortable_palette_set_color(colortable, 0x3d + i, get_color(state->m_last_colors[i]));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(lasso_state::lasso_get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	SET_TILE_INFO_MEMBER(0,
				  code + ((UINT16)m_gfxbank << 8),
				  color & 0x0f,
				  0);
}

TILE_GET_INFO_MEMBER(lasso_state::wwjgtin_get_track_tile_info)
{
	UINT8 *ROM = machine().root_device().memregion("user1")->base();
	int code = ROM[tile_index];
	int color = ROM[tile_index + 0x2000];

	SET_TILE_INFO_MEMBER(2,
				  code,
				  color & 0x0f,
				  0);
}

TILE_GET_INFO_MEMBER(lasso_state::pinbo_get_bg_tile_info)
{
	int code  = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	SET_TILE_INFO_MEMBER(0,
				  code + ((color & 0x30) << 4),
				  color & 0x0f,
				  0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

void lasso_state::video_start()
{

	/* create tilemap */
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(lasso_state::lasso_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(lasso_state,wwjgtin)
{

	/* create tilemaps */
	m_bg_tilemap =    &machine().tilemap().create(tilemap_get_info_delegate(FUNC(lasso_state::lasso_get_bg_tile_info),this),      TILEMAP_SCAN_ROWS,  8,  8,  32, 32);
	m_track_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(lasso_state::wwjgtin_get_track_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 128, 64);

	m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(lasso_state,pinbo)
{

	/* create tilemap */
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(lasso_state::pinbo_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(lasso_state::lasso_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(lasso_state::lasso_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(lasso_state::lasso_flip_screen_w)
{
	/* don't know which is which, but they are always set together */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	machine().tilemap().set_flip_all((flip_screen_x() ? TILEMAP_FLIPX : 0) | (flip_screen_y() ? TILEMAP_FLIPY : 0));
}


WRITE8_MEMBER(lasso_state::lasso_video_control_w)
{
	int bank = (data & 0x04) >> 2;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	lasso_flip_screen_w(space, offset, data);
}

WRITE8_MEMBER(lasso_state::wwjgtin_video_control_w)
{
	int bank = ((data & 0x04) ? 0 : 1) + ((data & 0x10) ? 2 : 0);
	m_track_enable = data & 0x08;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	lasso_flip_screen_w(space, offset, data);
}

WRITE8_MEMBER(lasso_state::pinbo_video_control_w)
{

	/* no need to dirty the tilemap -- only the sprites use the global bank */
	m_gfxbank = (data & 0x0c) >> 2;

	lasso_flip_screen_w(space, offset, data);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int reverse )
{
	lasso_state *state = machine.driver_data<lasso_state>();
	const UINT8 *finish, *source;
	int inc;

	if (reverse)
	{
		source = state->m_spriteram;
		finish = state->m_spriteram + state->m_spriteram.bytes();
		inc = 4;
	}
	else
	{
		source = state->m_spriteram + state->m_spriteram.bytes() - 4;
		finish = state->m_spriteram - 4;
		inc = -4;
	}

	while (source != finish)
	{
		int sx, sy, flipx, flipy;
		int code, color;

		sx = source[3];
		sy = source[0];
		flipx = source[1] & 0x40;
		flipy = source[1] & 0x80;

		if (state->flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (state->flip_screen_y())
			flipy = !flipy;
		else
			sy = 240 - sy;

		code = source[1] & 0x3f;
		color = source[2] & 0x0f;

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				code | ((UINT16)state->m_gfxbank << 6),
				color,
				flipx, flipy,
				sx,sy,0);

		source += inc;
    }
}


static void draw_lasso( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	lasso_state *state = machine.driver_data<lasso_state>();
	offs_t offs;
	pen_t pen = 0x3f;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int bit;
		UINT8 data;
		UINT8 x;
		UINT8 y = offs >> 5;

		if (state->flip_screen_y())
			y = ~y;

		if ((y < cliprect.min_y) || (y > cliprect.max_y))
			continue;

		x = (offs & 0x1f) << 3;
		data = state->m_bitmap_ram[offs];

		if (state->flip_screen_x())
			x = ~x;

		for (bit = 0; bit < 8; bit++)
		{
			if ((data & 0x80) && (x >= cliprect.min_x) && (x <= cliprect.max_x))
				bitmap.pix16(y, x) = pen;

			if (state->flip_screen_x())
				x = x - 1;
			else
				x = x + 1;

			data = data << 1;
		}
	}
}


SCREEN_UPDATE_IND16( lasso )
{
	lasso_state *state = screen.machine().driver_data<lasso_state>();
	palette_set_color(screen.machine(), 0, get_color(*state->m_back_color));
	bitmap.fill(0, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_lasso(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0);

	return 0;
}

SCREEN_UPDATE_IND16( chameleo )
{
	lasso_state *state = screen.machine().driver_data<lasso_state>();
	palette_set_color(screen.machine(), 0, get_color(*state->m_back_color));
	bitmap.fill(0, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 0);

	return 0;
}


SCREEN_UPDATE_IND16( wwjgtin )
{
	lasso_state *state = screen.machine().driver_data<lasso_state>();
	colortable_palette_set_color(screen.machine().colortable, 0, get_color(*state->m_back_color));
	wwjgtin_set_last_four_colors(screen.machine(), screen.machine().colortable);

	state->m_track_tilemap->set_scrollx(0, state->m_track_scroll[0] + state->m_track_scroll[1] * 256);
	state->m_track_tilemap->set_scrolly(0, state->m_track_scroll[2] + state->m_track_scroll[3] * 256);

	if (state->m_track_enable)
		state->m_track_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		bitmap.fill(get_black_pen(screen.machine()), cliprect);

	draw_sprites(screen.machine(), bitmap, cliprect, 1);	// reverse order
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
