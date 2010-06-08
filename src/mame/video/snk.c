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

UINT8 *snk_tx_videoram;
UINT8 *snk_fg_videoram;
UINT8 *snk_bg_videoram;

static tilemap_t *tx_tilemap;
static tilemap_t *fg_tilemap;
static tilemap_t *bg_tilemap;
static int fg_scrollx, fg_scrolly, bg_scrollx, bg_scrolly;
static int sp16_scrollx, sp16_scrolly, sp32_scrollx, sp32_scrolly;
static UINT8 sprite_split_point;
static int num_sprites, yscroll_mask;
static UINT32 bg_tile_offset;
static UINT32 tx_tile_offset;
static int is_psychos;

static UINT8 empty_tile[16*16];
static UINT8 drawmode_table[16];

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
	int code = snk_tx_videoram[tile_index];
	int color = code >> 5;
	SET_TILE_INFO(0,
			tx_tile_offset + code,
			color,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( ikari_get_tx_tile_info )
{
	int code = snk_tx_videoram[tile_index];
	SET_TILE_INFO(0,
			tx_tile_offset + code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( gwar_get_tx_tile_info )
{
	int code = snk_tx_videoram[tile_index];
	SET_TILE_INFO(0,
			tx_tile_offset + code,
			0,
			0);
}


static TILE_GET_INFO( marvins_get_fg_tile_info )
{
	int code = snk_fg_videoram[tile_index];
	SET_TILE_INFO(1,
			code,
			0,
			0);
}

static TILE_GET_INFO( marvins_get_bg_tile_info )
{
	int code = snk_bg_videoram[tile_index];
	SET_TILE_INFO(2,
			code,
			0,
			0);
}


static TILE_GET_INFO( aso_get_bg_tile_info )
{
	int code = snk_bg_videoram[tile_index];
	SET_TILE_INFO(1,
			bg_tile_offset + code,
			0,
			0);
}

static TILE_GET_INFO( tnk3_get_bg_tile_info )
{
	int attr = snk_bg_videoram[2*tile_index+1];
	int code = snk_bg_videoram[2*tile_index] | ((attr & 0x30) << 4);
	int color = (attr & 0xf) ^ 8;
	SET_TILE_INFO(1,
			code,
			color,
			0);
}

static TILE_GET_INFO( ikari_get_bg_tile_info )
{
	int attr = snk_bg_videoram[2*tile_index+1];
	int code = snk_bg_videoram[2*tile_index] | ((attr & 0x03) << 8);
	int color = (attr & 0x70) >> 4;
	SET_TILE_INFO(1,
			code,
			color,
			0);
}

static TILE_GET_INFO( gwar_get_bg_tile_info )
{
	int attr = snk_bg_videoram[2*tile_index+1];
	int code = snk_bg_videoram[2*tile_index] | ((attr & 0x0f) << 8);
	int color = (attr & 0xf0) >> 4;

	if (is_psychos)	// psychos has a separate palette bank bit
		color &= 7;

	SET_TILE_INFO(1,
			code,
			color,
			0);

	// bermudat, tdfever use FFFF to blank the background.
	// (still call SET_TILE_INFO, otherwise problems might occur on boot when
	// the tile data hasn't been initialised)
	if (code >= machine->gfx[1]->total_elements)
		tileinfo->pen_data = empty_tile;
}


/**************************************************************************************/

static VIDEO_START( snk_3bpp_shadow )
{
	int i;

	if(!(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 5; i++) drawmode_table[i] = DRAWMODE_SOURCE;
	drawmode_table[6] = (machine->config->video_attributes & VIDEO_HAS_SHADOWS) ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;
	drawmode_table[7] = DRAWMODE_NONE;

	for (i = 0x000;i < 0x400;i++)
		machine->shadow_table[i] = i | 0x200;
}

static VIDEO_START( snk_4bpp_shadow )
{
	int i;

	if(!(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 13; i++) drawmode_table[i] = DRAWMODE_SOURCE;
	drawmode_table[14] = DRAWMODE_SHADOW;
	drawmode_table[15] = DRAWMODE_NONE;

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0x000;i < 0x400;i++)
		machine->shadow_table[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x200;i < 0x300;i++)
		machine->shadow_table[i] = i + 0x100;
}


VIDEO_START( marvins )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	fg_tilemap = tilemap_create(machine, marvins_get_fg_tile_info, tilemap_scan_cols,    8, 8, 64, 32);
	bg_tilemap = tilemap_create(machine, marvins_get_bg_tile_info, tilemap_scan_cols,    8, 8, 64, 32);

	tilemap_set_transparent_pen(tx_tilemap,15);
	tilemap_set_scrolldy(tx_tilemap, 8, 8);

	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_scrolldx(fg_tilemap, 15,  31);
	tilemap_set_scrolldy(fg_tilemap,  8, -32);

	tilemap_set_scrolldx(bg_tilemap, 15,  31);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);

	tx_tile_offset = 0;
}

VIDEO_START( jcross )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	bg_tilemap = tilemap_create(machine, aso_get_bg_tile_info,     tilemap_scan_cols,    8, 8, 64, 64);

	tilemap_set_transparent_pen(tx_tilemap, 15);
	tilemap_set_scrolldy(tx_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);

	num_sprites = 25;
	yscroll_mask = 0x1ff;
	bg_tile_offset = 0;
	tx_tile_offset = 0;
}

VIDEO_START( sgladiat )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	bg_tilemap = tilemap_create(machine, aso_get_bg_tile_info,     tilemap_scan_cols,    8, 8, 64, 32);

	tilemap_set_transparent_pen(tx_tilemap, 15);
	tilemap_set_scrolldy(tx_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);

	num_sprites = 25;
	yscroll_mask = 0x0ff;
	bg_tile_offset = 0;
	tx_tile_offset = 0;
}

VIDEO_START( hal21 )
{
	VIDEO_START_CALL(jcross);

	tilemap_set_scrolldy(bg_tilemap,  8, -32+256);

	num_sprites = 50;
	yscroll_mask = 0x1ff;
}

VIDEO_START( aso )
{
	VIDEO_START_CALL(jcross);

	tilemap_set_scrolldx(bg_tilemap, 15+256, 24+256);

	num_sprites = 50;
	yscroll_mask = 0x1ff;
}


VIDEO_START( tnk3 )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	tx_tilemap = tilemap_create(machine, marvins_get_tx_tile_info, marvins_tx_scan_cols, 8, 8, 36, 28);
	bg_tilemap = tilemap_create(machine, tnk3_get_bg_tile_info,    tilemap_scan_cols,    8, 8, 64, 64);

	tilemap_set_transparent_pen(tx_tilemap, 15);
	tilemap_set_scrolldy(tx_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);

	num_sprites = 50;
	yscroll_mask = 0x1ff;
	tx_tile_offset = 0;
}

VIDEO_START( ikari )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	tx_tilemap = tilemap_create(machine, ikari_get_tx_tile_info, marvins_tx_scan_cols,  8,  8, 36, 28);
	bg_tilemap = tilemap_create(machine, ikari_get_bg_tile_info, tilemap_scan_cols,    16, 16, 32, 32);

	tilemap_set_transparent_pen(tx_tilemap, 15);
	tilemap_set_scrolldy(tx_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);

	tx_tile_offset = 0;
}

VIDEO_START( gwar )
{
	int i;

	/* prepare drawmode table */
	for(i = 0; i <= 14; i++) drawmode_table[i] = DRAWMODE_SOURCE;
	drawmode_table[15] = DRAWMODE_NONE;

	memset(empty_tile,0xf,sizeof(empty_tile));

	tx_tilemap = tilemap_create(machine, gwar_get_tx_tile_info, tilemap_scan_cols,  8,  8, 50, 32);
	bg_tilemap = tilemap_create(machine, gwar_get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(tx_tilemap, 15);

	tilemap_set_scrolldx(bg_tilemap, 16, 143);
	tilemap_set_scrolldy(bg_tilemap,  0, -32);

	tx_tile_offset = 0;

	is_psychos = 0;
}

VIDEO_START( psychos )
{
	VIDEO_START_CALL(gwar);
	is_psychos = 1;
}

VIDEO_START( tdfever )
{
	VIDEO_START_CALL(gwar);
	VIDEO_START_CALL(snk_4bpp_shadow);
}

/**************************************************************************************/

WRITE8_HANDLER( snk_tx_videoram_w )
{
	snk_tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset);
}

WRITE8_HANDLER( marvins_fg_videoram_w )
{
	snk_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( marvins_bg_videoram_w )
{
	snk_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( snk_bg_videoram_w )
{
	snk_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
}


WRITE8_HANDLER( snk_fg_scrollx_w )
{
	fg_scrollx = (fg_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_fg_scrolly_w )
{
	fg_scrolly = (fg_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_bg_scrollx_w )
{
	bg_scrollx = (bg_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_bg_scrolly_w )
{
	bg_scrolly = (bg_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp16_scrollx_w )
{
	sp16_scrollx = (sp16_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp16_scrolly_w )
{
	sp16_scrolly = (sp16_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp32_scrollx_w )
{
	sp32_scrollx = (sp32_scrollx & ~0xff) | data;
}

WRITE8_HANDLER( snk_sp32_scrolly_w )
{
	sp32_scrolly = (sp32_scrolly & ~0xff) | data;
}

WRITE8_HANDLER( snk_sprite_split_point_w )
{
	sprite_split_point = data;
}


WRITE8_HANDLER( marvins_palette_bank_w )
{
	tilemap_set_palette_offset(bg_tilemap, data & 0x70);
	tilemap_set_palette_offset(fg_tilemap, (data & 0x07) << 4);
}

WRITE8_HANDLER( marvins_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x80);

	// other bits unknown
}

WRITE8_HANDLER( sgladiat_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x80);

	tilemap_set_palette_offset(bg_tilemap, ((data & 0xf) ^ 8) << 4);

	// other bits unknown
}

WRITE8_HANDLER( hal21_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x80);

	tilemap_set_palette_offset(bg_tilemap, ((data & 0xf) ^ 8) << 4);
	if (bg_tile_offset != ((data & 0x20) << 3))
	{
		bg_tile_offset = (data & 0x20) << 3;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	// other bits unknown
}

WRITE8_HANDLER( marvins_scroll_msb_w )
{
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x04) << 6);
	fg_scrollx =   (fg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( jcross_scroll_msb_w )
{
	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( sgladiat_scroll_msb_w )
{
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
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

	flip_screen_set(space->machine, data & 0x20);

	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
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

	flip_screen_set(space->machine, data & 0x80);

	if (tx_tile_offset != ((data & 0x40) << 2))
	{
		tx_tile_offset = (data & 0x40) << 2;
		tilemap_mark_all_tiles_dirty(tx_tilemap);
	}

	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( aso_bg_bank_w )
{
	tilemap_set_palette_offset(bg_tilemap, ((data & 0xf) ^ 8) << 4);
	if (bg_tile_offset != ((data & 0x30) << 4))
	{
		bg_tile_offset = (data & 0x30) << 4;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( ikari_bg_scroll_msb_w )
{
	bg_scrollx = (bg_scrollx & 0xff) | ((data & 0x02) << 7);
	bg_scrolly = (bg_scrolly & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( ikari_sp_scroll_msb_w )
{
	sp32_scrollx = (sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	sp32_scrolly = (sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_HANDLER( ikari_unknown_video_w )
{
	/* meaning of 0xc980 uncertain.
       Normally 0x20, ikaria/ikarijp sets it to 0x31 during test mode.
       Changing char bank is necessary to fix the display during the
       hard flags test and the test grid.
       Changing palette bank is necessary to fix colors in test mode. */

if (data != 0x20 &&	// normal
	data != 0x31 &&	// ikari test
	data != 0xaa)	// victroad spurious during boot
	popmessage("attrs %02x contact MAMEDEV", data);

	tilemap_set_palette_offset(tx_tilemap, (data & 0x01) << 4);
	if (tx_tile_offset != ((data & 0x10) << 4))
	{
		tx_tile_offset = (data & 0x10) << 4;
		tilemap_mark_all_tiles_dirty(tx_tilemap);
	}
}

WRITE8_HANDLER( gwar_tx_bank_w )
{
	tilemap_set_palette_offset(tx_tilemap, (data & 0xf) << 4);
	if (tx_tile_offset != ((data & 0x30) << 4))
	{
		tx_tile_offset = (data & 0x30) << 4;
		tilemap_mark_all_tiles_dirty(tx_tilemap);
	}

	if (is_psychos)
		tilemap_set_palette_offset(bg_tilemap, (data & 0x80));
}

WRITE8_HANDLER( gwar_videoattrs_w )
{
	flip_screen_set(space->machine, data & 0x04);

	sp32_scrollx = (sp32_scrollx & 0xff) | ((data & 0x80) << 1);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x40) << 2);
	sp32_scrolly = (sp32_scrolly & 0xff) | ((data & 0x20) << 3);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x10) << 4);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( gwara_videoattrs_w )
{
	flip_screen_set(space->machine, data & 0x10);

	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( gwara_sp_scroll_msb_w )
{
	sp32_scrollx = (sp32_scrollx & 0xff) | ((data & 0x20) << 3);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x10) << 4);
	sp32_scrolly = (sp32_scrolly & 0xff) | ((data & 0x08) << 5);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x04) << 6);
}

WRITE8_HANDLER( tdfever_sp_scroll_msb_w )
{
	sp32_scrolly = (sp32_scrolly & 0xff) | ((data & 0x80) << 1);
	sp32_scrollx = (sp32_scrollx & 0xff) | ((data & 0x40) << 2);
}

WRITE8_HANDLER( tdfever_spriteram_w )
{
	/*  partial updates avoid flickers in the fsoccer radar. */
	if (offset < 0x80 && space->machine->generic.spriteram.u8[offset] != data)
	{
		int vpos = space->machine->primary_screen->vpos();

		if (vpos > 0)
			space->machine->primary_screen->update_partial(vpos - 1);
	}

	space->machine->generic.spriteram.u8[offset] = data;
}

/**************************************************************************************/

static void marvins_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
		const int scrollx, const int scrolly, const int from, const int to)
{
	const gfx_element *gfx = machine->gfx[3];
	const UINT8 *source, *finish;

	source = machine->generic.spriteram.u8 + from*4;
	finish = machine->generic.spriteram.u8 + to*4;

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
			drawmode_table, machine->shadow_table);

		source+=4;
	}
}


static void tnk3_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const int xscroll, const int yscroll)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	const gfx_element *gfx = machine->gfx[2];
	const int size = gfx->width;
	int tile_number, attributes, color, sx, sy;
	int xflip,yflip;
	int offs;

	/* jcross and sgladiat have only 25 sprites, the others 50 */

	/* jcross has 256 tiles, attribute bit 6 is unused and bit 5 is y-flip */
	/* sgladiat and tnk3 have 512 tiles, bit 6 is bank and bit 5 is y-flip */
	/* athena has 1024 tiles, bit 6 and bit 5 are bank */

	for (offs = 0; offs < num_sprites*4; offs += 4)
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
		sy &= yscroll_mask;	// sgladiat apparently has only 256 pixels of vertical scrolling range
		if (sx > 512-size) sx -= 512;
		if (sy > (yscroll_mask+1)-size) sy -= (yscroll_mask+1);

		drawgfx_transtable(bitmap,cliprect,gfx,
				tile_number,
				color,
				xflip,yflip,
				sx,sy,
				drawmode_table, machine->shadow_table);
	}
}


static void ikari_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
		const int start, const int xscroll, const int yscroll, const UINT8 *source, const int gfxnum )
{
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
				drawmode_table, machine->shadow_table);
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
				drawmode_table, machine->shadow_table);
	}
}

/**************************************************************/

VIDEO_UPDATE( marvins )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);
	tilemap_set_scrollx(fg_tilemap, 0, fg_scrollx);
	tilemap_set_scrolly(fg_tilemap, 0, fg_scrolly);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0 ,0);
	marvins_draw_sprites(screen->machine,bitmap,cliprect, sp16_scrollx, sp16_scrolly, 0, sprite_split_point>>2 );
	tilemap_draw(bitmap,cliprect,fg_tilemap,0 ,0);
	marvins_draw_sprites(screen->machine,bitmap,cliprect, sp16_scrollx, sp16_scrolly, sprite_split_point>>2, 25 );
	tilemap_draw(bitmap,cliprect,tx_tilemap,0 ,0);

	return 0;
}


VIDEO_UPDATE( tnk3 )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tnk3_draw_sprites(screen->machine, bitmap, cliprect, sp16_scrollx, sp16_scrolly);
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( ikari )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, sp16_scrollx, sp16_scrolly, screen->machine->generic.spriteram.u8 + 0x800, 2 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, sp32_scrollx, sp32_scrolly, screen->machine->generic.spriteram.u8,         3 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect, 25, sp16_scrollx, sp16_scrolly, screen->machine->generic.spriteram.u8 + 0x800, 2 );

	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( gwar )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	tdfever_draw_sprites(screen->machine, bitmap, cliprect, sp16_scrollx, sp16_scrolly, screen->machine->generic.spriteram.u8 + 0x800, 2, 0, 0, sprite_split_point );
	tdfever_draw_sprites(screen->machine, bitmap, cliprect, sp32_scrollx, sp32_scrolly, screen->machine->generic.spriteram.u8,         3, 0, 0, 32 );
	tdfever_draw_sprites(screen->machine, bitmap, cliprect, sp16_scrollx, sp16_scrolly, screen->machine->generic.spriteram.u8 + 0x800, 2, 0, sprite_split_point, 64 );

	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( tdfever )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	tdfever_draw_sprites(screen->machine, bitmap, cliprect, sp32_scrollx, sp32_scrolly, screen->machine->generic.spriteram.u8, 2, 1, 0, 32 );

	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);

	return 0;
}
