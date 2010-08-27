#include "emu.h"
#include "includes/snk.h"

/*******************************************************************************
 Shadow Handling Notes
********************************************************************************
 Shadows are handled by changing palette bank.

 Games Not Using Shadows

 those using gwar_vh_screenrefresh (gwar, bermudat, psychos, chopper1)
    (0-15 , 15 is transparent)

 Games Using Shadows

 those using tnk3_vh_screenrefresh (tnk3, athena, fitegolf) sgladiat is similar
    (0-7  , 6  is shadow, 7  is transparent) * these are using aso colour prom convert *
 those using ikari_vh_screenrefresh (ikari, victroad)
    (0-7  , 6  is shadow, 7  is transparent)
 those using tdfever_vh_screenrefresh (tdfever, fsoccer)
    (0-15 , 14 is shadow, 15 is transparent)

*******************************************************************************/


/**************************************************************************************/

PALETTE_INIT( tnk3 )
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

/**************************************************************************************/

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

static TILE_GET_INFO( marvins_get_tx_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->tx_videoram[tile_index];
	int color = code >> 5;

	SET_TILE_INFO(0,
			state->tx_tile_offset + code,
			color,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( ikari_get_tx_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->tx_videoram[tile_index];

	SET_TILE_INFO(0,
			state->tx_tile_offset + code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( gwar_get_tx_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->tx_videoram[tile_index];

	SET_TILE_INFO(0,
			state->tx_tile_offset + code,
			0,
			0);
}


static TILE_GET_INFO( marvins_get_fg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->fg_videoram[tile_index];

	SET_TILE_INFO(1,
			code,
			0,
			0);
}

static TILE_GET_INFO( marvins_get_bg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->bg_videoram[tile_index];

	SET_TILE_INFO(2,
			code,
			0,
			0);
}


static TILE_GET_INFO( aso_get_bg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int code = state->bg_videoram[tile_index];

	SET_TILE_INFO(1,
			state->bg_tile_offset + code,
			0,
			0);
}

static TILE_GET_INFO( tnk3_get_bg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int attr = state->bg_videoram[2*tile_index+1];
	int code = state->bg_videoram[2*tile_index] | ((attr & 0x30) << 4);
	int color = (attr & 0xf) ^ 8;

	SET_TILE_INFO(1,
			code,
			color,
			0);
}

static TILE_GET_INFO( ikari_get_bg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int attr = state->bg_videoram[2*tile_index+1];
	int code = state->bg_videoram[2*tile_index] | ((attr & 0x03) << 8);
	int color = (attr & 0x70) >> 4;

	SET_TILE_INFO(1,
			code,
			color,
			0);
}

static TILE_GET_INFO( gwar_get_bg_tile_info )
{
	snk_state *state = machine->driver_data<snk_state>();
	int attr = state->bg_videoram[2*tile_index+1];
	int code = state->bg_videoram[2*tile_index] | ((attr & 0x0f) << 8);
	int color = (attr & 0xf0) >> 4;

	if (state->is_psychos)	// psychos has a separate palette bank bit
		color &= 7;

	SET_TILE_INFO(1,
			code,
			color,
			0);

	// bermudat, tdfever use FFFF to blank the background.
	// (still call SET_TILE_INFO, otherwise problems might occur on boot when
	// the tile data hasn't been initialised)
	if (code >= machine->gfx[1]->total_elements)
		tileinfo->pen_data = state->empty_tile;
}


/**************************************************************************************/

static VIDEO_START( snk_3bpp_shadow )
{
	snk_state *state = machine->driver_data<snk_state>();
	int i;

	if(!(machine->config->m_video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 5; i++) state->drawmode_table[i] = DRAWMODE_SOURCE;
	state->drawmode_table[6] = (machine->config->m_video_attributes & VIDEO_HAS_SHADOWS) ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;
	state->drawmode_table[7] = DRAWMODE_NONE;

	for (i = 0x000;i < 0x400;i++)
		machine->shadow_table[i] = i | 0x200;
}

static VIDEO_START( snk_4bpp_shadow )
{
	snk_state *state = machine->driver_data<snk_state>();
	int i;

	if(!(machine->config->m_video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 13; i++) state->drawmode_table[i] = DRAWMODE_SOURCE;
	state->drawmode_table[14] = DRAWMODE_SHADOW;
	state->drawmode_table[15] = DRAWMODE_NONE;

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0x000;i < 0x400;i++)
		machine->shadow_table[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x200;i < 0x300;i++)
		machine->shadow_table[i] = i + 0x100;
}


VIDEO_START( marvins )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(snk_3bpp_shadow);

	state->tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	state->fg_tilemap = tilemap_create(machine, marvins_get_fg_tile_info, tilemap_scan_cols,    8, 8, 64, 32);
	state->bg_tilemap = tilemap_create(machine, marvins_get_bg_tile_info, tilemap_scan_cols,    8, 8, 64, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);
	tilemap_set_scrolldy(state->tx_tilemap, 8, 8);

	tilemap_set_transparent_pen(state->fg_tilemap, 15);
	tilemap_set_scrolldx(state->fg_tilemap, 15,  31);
	tilemap_set_scrolldy(state->fg_tilemap,  8, -32);

	tilemap_set_scrolldx(state->bg_tilemap, 15,  31);
	tilemap_set_scrolldy(state->bg_tilemap,  8, -32);

	state->tx_tile_offset = 0;
}

VIDEO_START( jcross )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(snk_3bpp_shadow);

	state->tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	state->bg_tilemap = tilemap_create(machine, aso_get_bg_tile_info,     tilemap_scan_cols,    8, 8, 64, 64);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);
	tilemap_set_scrolldy(state->tx_tilemap, 8, 8);

	tilemap_set_scrolldx(state->bg_tilemap, 15, 24);
	tilemap_set_scrolldy(state->bg_tilemap,  8, -32);

	state->num_sprites = 25;
	state->yscroll_mask = 0x1ff;
	state->bg_tile_offset = 0;
	state->tx_tile_offset = 0;
}

VIDEO_START( sgladiat )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(snk_3bpp_shadow);

	state->tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	state->bg_tilemap = tilemap_create(machine, aso_get_bg_tile_info,     tilemap_scan_cols,    8, 8, 64, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);
	tilemap_set_scrolldy(state->tx_tilemap, 8, 8);

	tilemap_set_scrolldx(state->bg_tilemap, 15, 24);
	tilemap_set_scrolldy(state->bg_tilemap,  8, -32);

	state->num_sprites = 25;
	state->yscroll_mask = 0x0ff;
	state->bg_tile_offset = 0;
	state->tx_tile_offset = 0;
}

VIDEO_START( hal21 )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(jcross);

	tilemap_set_scrolldy(state->bg_tilemap,  8, -32+256);

	state->num_sprites = 50;
	state->yscroll_mask = 0x1ff;
}

VIDEO_START( aso )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(jcross);

	tilemap_set_scrolldx(state->bg_tilemap, 15+256, 24+256);

	state->num_sprites = 50;
	state->yscroll_mask = 0x1ff;
}


VIDEO_START( tnk3 )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(snk_3bpp_shadow);

	state->tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	state->bg_tilemap = tilemap_create(machine, tnk3_get_bg_tile_info,    tilemap_scan_cols,    8, 8, 64, 64);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);
	tilemap_set_scrolldy(state->tx_tilemap, 8, 8);

	tilemap_set_scrolldx(state->bg_tilemap, 15, 24);
	tilemap_set_scrolldy(state->bg_tilemap,  8, -32);

	state->num_sprites = 50;
	state->yscroll_mask = 0x1ff;
	state->tx_tile_offset = 0;
}

VIDEO_START( ikari )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(snk_3bpp_shadow);

	state->tx_tilemap = tilemap_create(machine, ikari_get_tx_tile_info, marvins_tx_scan_cols,  8,  8, 36, 28);
	state->bg_tilemap = tilemap_create(machine, ikari_get_bg_tile_info, tilemap_scan_cols,    16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);
	tilemap_set_scrolldy(state->tx_tilemap, 8, 8);

	tilemap_set_scrolldx(state->bg_tilemap, 15, 24);
	tilemap_set_scrolldy(state->bg_tilemap,  8, -32);

	state->tx_tile_offset = 0;
}

VIDEO_START( gwar )
{
	snk_state *state = machine->driver_data<snk_state>();
	int i;

	/* prepare drawmode table */
	for(i = 0; i <= 14; i++) state->drawmode_table[i] = DRAWMODE_SOURCE;
	state->drawmode_table[15] = DRAWMODE_NONE;

	memset(state->empty_tile, 0xf, sizeof(state->empty_tile));

	state->tx_tilemap = tilemap_create(machine, gwar_get_tx_tile_info, tilemap_scan_cols,  8,  8, 50, 32);
	state->bg_tilemap = tilemap_create(machine, gwar_get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 15);

	tilemap_set_scrolldx(state->bg_tilemap, 16, 143);
	tilemap_set_scrolldy(state->bg_tilemap,  0, -32);

	state->tx_tile_offset = 0;

	state->is_psychos = 0;
}

VIDEO_START( psychos )
{
	snk_state *state = machine->driver_data<snk_state>();

	VIDEO_START_CALL(gwar);
	state->is_psychos = 1;
}

VIDEO_START( tdfever )
{
	VIDEO_START_CALL(gwar);
	VIDEO_START_CALL(snk_4bpp_shadow);
}

/**************************************************************************************/

WRITE8_HANDLER( snk_tx_videoram_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE8_HANDLER( marvins_fg_videoram_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( marvins_bg_videoram_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( snk_bg_videoram_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset >> 1);
}


WRITE8_HANDLER( snk_fg_scrollx_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->fg_scrollx = (state->fg_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_fg_scrolly_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->fg_scrolly = (state->fg_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_bg_scrollx_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrollx = (state->bg_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_bg_scrolly_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrolly = (state->bg_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp16_scrollx_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp16_scrollx = (state->sp16_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp16_scrolly_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp16_scrolly = (state->sp16_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp32_scrollx_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp32_scrollx = (state->sp32_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp32_scrolly_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp32_scrolly = (state->sp32_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sprite_split_point_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sprite_split_point = data;
}


WRITE8_HANDLER( marvins_palette_bank_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	tilemap_set_palette_offset(state->bg_tilemap, data & 0x70);
	tilemap_set_palette_offset(state->fg_tilemap, (data & 0x07) << 4);
}

WRITE8_HANDLER( marvins_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x80);

	// other bits unknown
}

WRITE8_HANDLER( sgladiat_flipscreen_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x80);

	tilemap_set_palette_offset(state->bg_tilemap, ((data & 0xf) ^ 8) << 4);

	// other bits unknown
}

WRITE8_HANDLER( hal21_flipscreen_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x80);

	tilemap_set_palette_offset(state->bg_tilemap, ((data & 0xf) ^ 8) << 4);
	if (state->bg_tile_offset != ((data & 0x20) << 3))
	{
		state->bg_tile_offset = (data & 0x20) << 3;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	// other bits unknown
}

WRITE8_HANDLER( marvins_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x04) << 6);
	state->fg_scrollx =   (state->fg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( jcross_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrolly =   (state->bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( sgladiat_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( aso_videoattrs_w )
{
	/*
        video attributes:
        X-------
        -X------
        --X-----    flip screen
        ---X----    scrolly MSB (background)
        ----X---    scrolly MSB (sprites)
        -----X--
        ------X-    scrollx MSB (background)
        -------X    scrollx MSB (sprites)
    */

	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x20);

	state->bg_scrolly =   (state->bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( tnk3_videoattrs_w )
{
	/*
        video attributes:
        X-------    flip screen
        -X------    character bank (for text layer)
        --X-----
        ---X----    scrolly MSB (background)
        ----X---    scrolly MSB (sprites)
        -----X--
        ------X-    scrollx MSB (background)
        -------X    scrollx MSB (sprites)
    */

	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x80);

	if (state->tx_tile_offset != ((data & 0x40) << 2))
	{
		state->tx_tile_offset = (data & 0x40) << 2;
		tilemap_mark_all_tiles_dirty(state->tx_tilemap);
	}

	state->bg_scrolly =   (state->bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( aso_bg_bank_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	tilemap_set_palette_offset(state->bg_tilemap, ((data & 0xf) ^ 8) << 4);
	if (state->bg_tile_offset != ((data & 0x30) << 4))
	{
		state->bg_tile_offset = (data & 0x30) << 4;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( ikari_bg_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->bg_scrollx = (state->bg_scrollx & 0xff) | ((data & 0x02) << 7);
	state->bg_scrolly = (state->bg_scrolly & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( ikari_sp_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp32_scrollx = (state->sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	state->sp32_scrolly = (state->sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_HANDLER( ikari_unknown_video_w )
{
	/* meaning of 0xc980 uncertain.
       Normally 0x20, ikaria/ikarijp sets it to 0x31 during test mode.
       Changing char bank is necessary to fix the display during the
       hard flags test and the test grid.
       Changing palette bank is necessary to fix colors in test mode. */

	snk_state *state = space->machine->driver_data<snk_state>();

if (data != 0x20 &&	// normal
	data != 0x31 &&	// ikari test
	data != 0xaa)	// victroad spurious during boot
	popmessage("attrs %02x contact MAMEDEV", data);

	tilemap_set_palette_offset(state->tx_tilemap, (data & 0x01) << 4);
	if (state->tx_tile_offset != ((data & 0x10) << 4))
	{
		state->tx_tile_offset = (data & 0x10) << 4;
		tilemap_mark_all_tiles_dirty(state->tx_tilemap);
	}
}

WRITE8_HANDLER( gwar_tx_bank_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	tilemap_set_palette_offset(state->tx_tilemap, (data & 0xf) << 4);
	if (state->tx_tile_offset != ((data & 0x30) << 4))
	{
		state->tx_tile_offset = (data & 0x30) << 4;
		tilemap_mark_all_tiles_dirty(state->tx_tilemap);
	}

	if (state->is_psychos)
		tilemap_set_palette_offset(state->bg_tilemap, (data & 0x80));
}

WRITE8_HANDLER( gwar_videoattrs_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x04);

	state->sp32_scrollx = (state->sp32_scrollx & 0xff) | ((data & 0x80) << 1);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x40) << 2);
	state->sp32_scrolly = (state->sp32_scrolly & 0xff) | ((data & 0x20) << 3);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x10) << 4);
	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->bg_scrolly =   (state->bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( gwara_videoattrs_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	flip_screen_set(space->machine, data & 0x10);

	state->bg_scrollx =   (state->bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	state->bg_scrolly =   (state->bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( gwara_sp_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp32_scrollx = (state->sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	state->sp16_scrollx = (state->sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	state->sp32_scrolly = (state->sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	state->sp16_scrolly = (state->sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_HANDLER( tdfever_sp_scroll_msb_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	state->sp32_scrolly = (state->sp32_scrolly & 0xff) | ((data & 0x80) << 1);
	state->sp32_scrollx = (state->sp32_scrollx & 0xff) | ((data & 0x40) << 2);
}

WRITE8_HANDLER( tdfever_spriteram_w )
{
	snk_state *state = space->machine->driver_data<snk_state>();

	/*  partial updates avoid flickers in the fsoccer radar. */
	if (offset < 0x80 && state->spriteram[offset] != data)
	{
		int vpos = space->machine->primary_screen->vpos();

		if (vpos > 0)
			space->machine->primary_screen->update_partial(vpos - 1);
	}

	state->spriteram[offset] = data;
}

/**************************************************************************************/

static void marvins_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
		const int scrollx, const int scrolly, const int from, const int to)
{
	snk_state *state = machine->driver_data<snk_state>();
	const gfx_element *gfx = machine->gfx[3];
	const UINT8 *source, *finish;

	source = state->spriteram + from*4;
	finish = state->spriteram + to*4;

	while( source<finish )
	{
		int attributes = source[3]; /* Y?F? CCCC */
		int tile_number = source[1];
		int sx =  scrollx + 301 - 15 - source[2] + ((attributes&0x80)?256:0);
		int sy = -scrolly - 8 + source[0];
		int color = attributes&0xf;
		int flipy = (attributes&0x20);
		int flipx = 0;

		if (flip_screen_get(machine))
		{
			sx = 89 - 16 - sx;
			sy = 262 - 16 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx &= 0x1ff;
		sy &= 0xff;
		if (sx > 512-16) sx -= 512;
		if (sy > 256-16) sy -= 256;

		drawgfx_transtable(bitmap,cliprect,gfx,
			tile_number,
			color,
			flipx, flipy,
			sx, sy,
			state->drawmode_table, machine->shadow_table);

		source+=4;
	}
}


static void tnk3_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const int xscroll, const int yscroll)
{
	snk_state *state = machine->driver_data<snk_state>();
	UINT8 *spriteram = state->spriteram;
	const gfx_element *gfx = machine->gfx[2];
	const int size = gfx->width;
	int tile_number, attributes, color, sx, sy;
	int xflip,yflip;
	int offs;

	/* jcross and sgladiat have only 25 sprites, the others 50 */

	/* jcross has 256 tiles, attribute bit 6 is unused and bit 5 is y-flip */
	/* sgladiat and tnk3 have 512 tiles, bit 6 is bank and bit 5 is y-flip */
	/* athena has 1024 tiles, bit 6 and bit 5 are bank */

	for (offs = 0; offs < state->num_sprites*4; offs += 4)
	{
		tile_number = spriteram[offs+1];
		attributes  = spriteram[offs+3];
		color = attributes & 0xf;
		sx =  xscroll + 301 - size - spriteram[offs+2];
		sy = -yscroll + 7 - size + spriteram[offs];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;
		xflip = 0;
		yflip = 0;

		if (gfx->total_elements > 256)	// all except jcross
		{
			tile_number |= (attributes & 0x40) << 2;
		}

		if (gfx->total_elements > 512)	// athena
		{
			tile_number |= (attributes & 0x20) << 4;
		}
		else	// all others
		{
			yflip = attributes & 0x20;
		}

		if (flip_screen_get(machine))
		{
			sx = 89 - size - sx;	// this causes slight misalignment in tnk3 but is correct for athena and fitegolf
			sy = 262 - size - sy;
			xflip = !xflip;
			yflip = !yflip;
		}

		sx &= 0x1ff;
		sy &= state->yscroll_mask;	// sgladiat apparently has only 256 pixels of vertical scrolling range
		if (sx > 512-size) sx -= 512;
		if (sy > (state->yscroll_mask+1)-size) sy -= (state->yscroll_mask+1);

		drawgfx_transtable(bitmap,cliprect,gfx,
				tile_number,
				color,
				xflip,yflip,
				sx,sy,
				state->drawmode_table, machine->shadow_table);
	}
}


static void ikari_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
		const int start, const int xscroll, const int yscroll, const UINT8 *source, const int gfxnum )
{
	snk_state *state = machine->driver_data<snk_state>();
	const gfx_element *gfx = machine->gfx[gfxnum];
	const int size = gfx->width;
	int tile_number, attributes, color, sx, sy;
	int which, finish;

	finish = (start+25)*4;

	for (which = start*4; which < finish; which += 4)
	{
		tile_number = source[which+1];
		attributes  = source[which+3];
		color = attributes & 0xf;
		sx =  xscroll + 300 - size - source[which+2];
		sy = -yscroll + 7 - size + source[which];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;

		switch (size)
		{
			case 16:
				tile_number |= (attributes & 0x60) << 3;
				break;

			case 32:
				tile_number |= (attributes & 0x40) << 2;
				break;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-size) sx -= 512;
		if (sy > 512-size) sy -= 512;

		drawgfx_transtable(bitmap,cliprect,gfx,
				tile_number,
				color,
				0,0,
				sx,sy,
				state->drawmode_table, machine->shadow_table);
	}
}

/**************************************************************/

/*
Sprite Format
-------------
byte0: y offset
byte1: tile number
byte2: x offset
byte3: attributes

    32x32 attributes:

    76543210
    ----xxxx (color)
    ---x---- (y offset bit8)
    -xx----- (bank number)
    x------- (x offset bit8)

    16x16 attributes:

    76543210
    -----xxx (color)
    ---x---- (y offset bit8)
    -xx-x--- (bank number)
    x------- (x offset bit8)
*/
static void tdfever_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
		const int xscroll, const int yscroll, const UINT8 *source, const int gfxnum, const int hw_xflip, const int from, const int to )
{
	snk_state *state = machine->driver_data<snk_state>();
	const gfx_element *gfx = machine->gfx[gfxnum];
	const int size = gfx->width;
	int tile_number, attributes, sx, sy, color;
	int which;
	int flipx, flipy;

	for(which = from*4; which < to*4; which+=4)
	{
		tile_number = source[which+1];
		attributes  = source[which+3];
		color = attributes & 0x0f;
		sx = -xscroll - 9 + source[which+2];
		sy = -yscroll + 1 - size + source[which];
		sx += (attributes & 0x80) << 1;
		sy += (attributes & 0x10) << 4;

		switch (size)
		{
			case 16:
				tile_number |= ((attributes & 0x08) << 5) | ((attributes & 0x60) << 4);
				color &= 7;	// attribute bit 3 is used for bank select
				if (from == 0)
					color |= 8;	// low priority sprites use the other palette bank
				break;

			case 32:
				tile_number |= (attributes & 0x60) << 3;
				break;
		}

		flipx = hw_xflip;
		flipy = 0;

		if (hw_xflip)
			sx = 495 - size - sx;

		if (flip_screen_get(machine))
		{
			sx = 495 - size - sx;
			sy = 258 - size - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-size) sx -= 512;
		if (sy > 512-size) sy -= 512;

		drawgfx_transtable(bitmap,cliprect,gfx,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,
				state->drawmode_table, machine->shadow_table);
	}
}

/**************************************************************/

VIDEO_UPDATE( marvins )
{
	snk_state *state = screen->machine->driver_data<snk_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);
	tilemap_set_scrollx(state->fg_tilemap, 0, state->fg_scrollx);
	tilemap_set_scrolly(state->fg_tilemap, 0, state->fg_scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	marvins_draw_sprites(screen->machine, bitmap, cliprect, state->sp16_scrollx, state->sp16_scrolly, 0, state->sprite_split_point>>2);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	marvins_draw_sprites(screen->machine, bitmap, cliprect, state->sp16_scrollx, state->sp16_scrolly, state->sprite_split_point>>2, 25);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( tnk3 )
{
	snk_state *state = screen->machine->driver_data<snk_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tnk3_draw_sprites(screen->machine, bitmap, cliprect, state->sp16_scrollx, state->sp16_scrolly);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( ikari )
{
	snk_state *state = screen->machine->driver_data<snk_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, state->sp16_scrollx, state->sp16_scrolly, state->spriteram + 0x800, 2 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, state->sp32_scrollx, state->sp32_scrolly, state->spriteram,         3 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect, 25, state->sp16_scrollx, state->sp16_scrolly, state->spriteram + 0x800, 2 );

	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( gwar )
{
	snk_state *state = screen->machine->driver_data<snk_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	tdfever_draw_sprites(screen->machine, bitmap, cliprect, state->sp16_scrollx, state->sp16_scrolly, state->spriteram + 0x800, 2, 0, 0, state->sprite_split_point );
	tdfever_draw_sprites(screen->machine, bitmap, cliprect, state->sp32_scrollx, state->sp32_scrolly, state->spriteram,         3, 0, 0, 32 );
	tdfever_draw_sprites(screen->machine, bitmap, cliprect, state->sp16_scrollx, state->sp16_scrolly, state->spriteram + 0x800, 2, 0, state->sprite_split_point, 64 );

	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( tdfever )
{
	snk_state *state = screen->machine->driver_data<snk_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	tdfever_draw_sprites(screen->machine, bitmap, cliprect, state->sp32_scrollx, state->sp32_scrolly, state->spriteram, 2, 1, 0, 32 );

	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}
