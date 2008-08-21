#include "driver.h"
#include "snk.h"

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
 those using tdfever_vh_screenrefresh (tdfever)
    (0-15 , 14(13 for tdfeverj) is shadow, 15 is transparent)
 those using ftsoccer_vh_screenrefresh (ftsoccer)
    (0-15 , 14 is shadow/highlight, 15 is transparent)
 those using ikari_vh_screenrefresh (ikari, victroad)
    (0-7  , 6  is shadow, 7  is transparent)

*******************************************************************************/


#define MAX_VRAM_SIZE (64*64*2) /* 0x2000 */


UINT8 *snk_fg_videoram;
UINT8 *snk_bg_videoram;

static tilemap *fg_tilemap;
static tilemap *bg_tilemap;
static int fg_bank;
static int bg_scrollx, bg_scrolly, sp16_scrollx, sp16_scrolly, sp32_scrollx, sp32_scrolly;
static UINT8 unknown_reg;

/**************************************************************************************/

PALETTE_INIT( tnk3 )
{
	int i;
	int num_colors = 0x400;

	/*
        palette format is RRRG GGBB B??? the three unknown bits are used but
        I'm not sure how, I'm currently using them as least significant bit but
        that's most likely wrong.
    */
	for( i=0; i<num_colors; i++ )
	{
		int bit0=0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i + 2*num_colors] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 1) & 0x01;
		bit1 = (color_prom[i + num_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 3) & 0x01;
		bit3 = (color_prom[i] >> 0) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*num_colors] >> 3) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 0) & 0x01;
		bit3 = (color_prom[i + num_colors] >> 1) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

/**************************************************************************************/

static TILEMAP_MAPPER( tnk3_fg_scan_cols )
{
	// tilemap is 36x28, the central part is from the first RAM page and the
	// extra 4 columns are from the second page
	col -= 2;
	if (col & 0x20)
		return 0x400 + row + ((col & 0x1f) << 5);
	else
		return row + (col << 5);
}

static TILE_GET_INFO( tnk3_get_fg_tile_info )
{
	int code = snk_fg_videoram[tile_index];
	int color = code >> 5;
	SET_TILE_INFO(0,
			code | (fg_bank << 8),
			color,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( ikari_get_fg_tile_info )
{
	int code = snk_fg_videoram[tile_index];
	SET_TILE_INFO(0,
			code | (fg_bank << 8),
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( gwar_get_fg_tile_info )
{
	int code = snk_fg_videoram[tile_index];
	SET_TILE_INFO(0,
			code | (fg_bank << 8),
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
	int code = snk_bg_videoram[2*tile_index] | ((attr & 0x07) << 8);
	int color = (attr & 0xf0) >> 4;
	SET_TILE_INFO(1,
			code,
			color,
			0);
}


/**************************************************************************************/

VIDEO_START( snk )
{
	tmpbitmap = auto_bitmap_alloc(512, 512, video_screen_get_format(machine->primary_screen));
}


VIDEO_START( snk_3bpp_shadow )
{
	int i;

	VIDEO_START_CALL(snk);

	if(!(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 5; i++) gfx_drawmode_table[i] = DRAWMODE_SOURCE;
	gfx_drawmode_table[6] = DRAWMODE_SHADOW;
	gfx_drawmode_table[7] = DRAWMODE_NONE;

	for (i = 0x000;i < 0x400;i++)
		machine->shadow_table[i] = i | 0x200;
}

VIDEO_START( snk_4bpp_shadow )
{
	int i;

	VIDEO_START_CALL(snk);

	if(!(machine->config->video_attributes & VIDEO_HAS_SHADOWS))
		fatalerror("driver should use VIDEO_HAS_SHADOWS");

	/* prepare shadow draw table */
	for(i = 0; i <= 13; i++) gfx_drawmode_table[i] = DRAWMODE_SOURCE;
	gfx_drawmode_table[14] = DRAWMODE_SHADOW;
	gfx_drawmode_table[15] = DRAWMODE_NONE;

	/* all palette entries are not affected by shadow sprites... */
	for (i = 0x00;i < 0x400;i++)
		machine->shadow_table[i] = i;
	/* ... except for tilemap colors */
	for (i = 0x200;i < 0x300;i++)
		machine->shadow_table[i] = i + 0x100;
}

VIDEO_START( tnk3 )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	fg_tilemap = tilemap_create(tnk3_get_fg_tile_info, tnk3_fg_scan_cols, 8, 8, 36, 28);
	bg_tilemap = tilemap_create(tnk3_get_bg_tile_info, tilemap_scan_cols, 8, 8, 64, 64);

	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_scrolldy(fg_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);
}

VIDEO_START( ikari )
{
	VIDEO_START_CALL(snk_3bpp_shadow);

	fg_tilemap = tilemap_create(ikari_get_fg_tile_info, tnk3_fg_scan_cols,  8,  8, 36, 28);
	bg_tilemap = tilemap_create(ikari_get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);
	tilemap_set_scrolldy(fg_tilemap, 8, 8);

	tilemap_set_scrolldx(bg_tilemap, 15, 24);
	tilemap_set_scrolldy(bg_tilemap,  8, -32);
}

VIDEO_START( gwar )
{
	VIDEO_START_CALL(snk);

	fg_tilemap = tilemap_create(gwar_get_fg_tile_info, tilemap_scan_cols,  8,  8, 64, 32);
	bg_tilemap = tilemap_create(gwar_get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);

	tilemap_set_scrolldx(bg_tilemap, 16, 142);
	tilemap_set_scrolldy(bg_tilemap,  0, -32);
}

/**************************************************************************************/

WRITE8_HANDLER( snk_fg_videoram_w )
{
	snk_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( snk_bg_videoram_w )
{
	snk_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
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

	int bank = (data & 0x40) >> 6;

	if (fg_bank != bank)
	{
		tilemap_mark_all_tiles_dirty(fg_tilemap);
		fg_bank = bank;
	}

	flip_screen_set(data & 0x80);

	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x10) << 4);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x08) << 5);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x01) << 8);
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

	int bank = (data & 0x10) >> 4;

if (data != 0x20 &&	// normal
	data != 0x31 &&	// ikari test
	data != 0xaa)	// victroad spurious during boot
	popmessage("attrs %02x contact MAMEDEV", data);

	if (fg_bank != bank)
	{
		tilemap_mark_all_tiles_dirty(fg_tilemap);
		fg_bank = bank;
	}

	if (data & 1)
		tilemap_set_palette_offset(fg_tilemap, 16);
	else
		tilemap_set_palette_offset(fg_tilemap, 0);
}

WRITE8_HANDLER( gwar_fg_bank_w )
{
	int bank = (data & 0x30) >> 4;

	if (fg_bank != bank)
	{
		tilemap_mark_all_tiles_dirty(fg_tilemap);
		fg_bank = bank;
	}

	tilemap_set_palette_offset(fg_tilemap, (data & 0xf) << 4);
}

WRITE8_HANDLER( gwar_videoattrs_w )
{
	flip_screen_set(data & 0x04);

	sp32_scrollx = (sp32_scrollx & 0xff) | ((data & 0x80) << 1);
	sp16_scrollx = (sp16_scrollx & 0xff) | ((data & 0x40) << 2);
	sp32_scrolly = (sp32_scrolly & 0xff) | ((data & 0x20) << 3);
	sp16_scrolly = (sp16_scrolly & 0xff) | ((data & 0x10) << 4);
	bg_scrollx =   (bg_scrollx   & 0xff) | ((data & 0x02) << 7);
	bg_scrolly =   (bg_scrolly   & 0xff) | ((data & 0x01) << 8);
}

WRITE8_HANDLER( gwara_videoattrs_w )
{
	flip_screen_set(data & 0x10);

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

WRITE8_HANDLER( gwar_unknown_video_w )
{
//popmessage("%02x",data);
	unknown_reg = data;
}

/**************************************************************************************/

static void tnk3_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xscroll, int yscroll)
{
	const gfx_element *gfx = machine->gfx[2];

	int tile_number, attributes, color, sx, sy;
	int xflip,yflip;
	int offs;

	/* tnk3 has 512 tiles, attribute bit 5 is y-flip */
	/* athena has 1024 tiles, attribute bit 5 is extra bank bit */
	int is_athena = (gfx->total_elements > 512);

	for(offs = 0; offs < 50*4; offs+=4)
	{
		tile_number = spriteram[offs+1];
		attributes  = spriteram[offs+3];
		tile_number |= (attributes & 0x40) << 2;
		color = attributes & 0xf;
		sx =  xscroll + 45 - 16 - spriteram[offs+2];
		if (!(attributes & 0x80)) sx += 256;
		sy = -yscroll + 7 - 16 + spriteram[offs];
		if (attributes & 0x10) sy += 256;
		xflip = 0;
		yflip = 0;

		if (is_athena)
		{
			tile_number |= (attributes & 0x20) << 4;
		}
		else	// tnk3
		{
			yflip = attributes & 0x20;
		}

		if (flip_screen_get())
		{
			sx = 73 - sx;	// this causes slight misalignment in tnk3 but is correct for athena and fitegolf
			sy = 246 - sy;
			xflip = !xflip;
			yflip = !yflip;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-16) sx -= 512;
		if (sy > 512-16) sy -= 512;

		drawgfx(bitmap,gfx,
				tile_number,
				color,
				xflip,yflip,
				sx,sy,
				cliprect,TRANSPARENCY_PEN_TABLE,7);
	}
}



static void ikari_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int start, int xscroll, int yscroll,
				UINT8 *source, int mode )
{
	gfx_element *gfx = machine->gfx[mode];
	int tile_number, attributes, color, sx, sy;
	int which, finish;
	int size = (mode == 2) ? 16 : 32;

	finish = (start+25)*4;

	for(which = start*4; which < finish; which+=4)
	{
		tile_number = source[which+1];
		attributes  = source[which+3];
		switch(mode)
		{
			case 2:
				tile_number |= (attributes & 0x60) << 3;
				break;
			case 3:
				tile_number |= (attributes & 0x40) << 2;
				break;
		}
		color = attributes & 0xf;
		sx =  xscroll + 44 - size - source[which+2];
		if (!(attributes & 0x80)) sx += 256;
		sy = -yscroll + 7 - size + source[which];
		if (attributes & 0x10) sy += 256;
		sx &= 0x1ff;
		sy &= 0x1ff;
		if (sx > 512-32) sx -= 512;
		if (sy > 512-32) sy -= 512;

		drawgfx(bitmap,gfx,
				tile_number,
				color,
				0,0,
				sx,sy,
				cliprect,TRANSPARENCY_PEN_TABLE,7);
	}
}



VIDEO_UPDATE( tnk3 )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tnk3_draw_sprites(screen->machine, bitmap, cliprect, sp16_scrollx, sp16_scrolly);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( ikari )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, sp16_scrollx, sp16_scrolly, spriteram + 0x800, 2 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect,  0, sp32_scrollx, sp32_scrolly, spriteram, 3 );
	ikari_draw_sprites(screen->machine, bitmap, cliprect, 25, sp16_scrollx, sp16_scrolly, spriteram + 0x800, 2 );

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}

/**************************************************************/

static void tdfever_draw_bg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xscroll, int yscroll )
{
	const UINT8 *source = snk_rambase + 0x000;
	const gfx_element *gfx = machine->gfx[1];

	int tile_number, attributes, color, sx, sy;
	int offs, x, y;

	for(x = 0; x < 32; x++) for(y = 0; y < 32; y++)
	{
		offs = (x<<6)+(y<<1);
		tile_number = source[offs];
		attributes  = source[offs+1];

		tile_number |= (attributes & 0xf) << 8;

		color = attributes >> 4;
		sx = x << 4;
		sy = y << 4;

		// intercept overflown tile indices
		if(tile_number >= gfx->total_elements)
			plot_box(tmpbitmap, sx, sy, gfx->width, gfx->height, get_black_pen(machine));
		else
			drawgfx(tmpbitmap,gfx,tile_number,color,0,0,sx,sy,0,TRANSPARENCY_NONE,0);
	}
	copyscrollbitmap(bitmap,tmpbitmap,1,&xscroll,1,&yscroll,cliprect);
}

/*
Sprite Format
-------------
byte0: y offset
byte1: tile number
byte2: x offset
byte3: attributes

    mode 0/1 attributes:

    76543210
    ----xxxx (color)
    ---x---- (y offset bit8)
    -xx----- (bank number)
    x------- (x offset bit8)

    mode 2 attributes:

    76543210
    -----xxx (color)
    ---x---- (y offset bit8)
    -xx-x--- (bank number)
    x------- (x offset bit8)
*/
static void tdfever_draw_sp(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xscroll, int yscroll, int mode, int flip )
{
	const UINT8 *source = spriteram + ((mode==2)?0x800:0x000);
	const gfx_element *gfx = machine->gfx[(mode==1)?3:2];
	int tile_number, attributes, sx, sy, color, pen_mode;
	int which, finish, sp_size;
	int flipx, flipy;

	if(mode < 0 || mode > 2) return;

	pen_mode = (snk_gamegroup & 1) ? TRANSPARENCY_PEN_TABLE : TRANSPARENCY_PEN;

	if(mode == 2)
	{
		finish  = 64 * 4;
		sp_size = 16;
	}
	else
	{
		finish  = 32 * 4;
		sp_size = 32;
	}

	for(which = 0; which < finish; which+=4)
	{
		if(*(UINT32*)(source+which) == 0 || *(UINT32*)(source+which) == -1) continue;

		tile_number = source[which+1];
		attributes  = source[which+3];

		sx = xscroll + source[which+2]; if(mode==0) sx = 256-sx;
		sy = yscroll + source[which];
		sx += attributes<<1 & 0x100;
		sy += attributes<<4 & 0x100;

		flipx = flip;
		flipy = 0;

		if (flip_screen_get())
		{
			sx = 495 - sp_size - sx;
			sy = 258 - sp_size - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx &= 0x1ff;
		sy &= 0x1ff;
		if(sx > 512-sp_size) sx -= 512;
		if(sy > 512-sp_size) sy -= 512;

		switch(mode)
		{
			case 2:
				tile_number |= (attributes<<4 & 0x600) | (attributes<<5 & 0x100);
				color = attributes & 0x07;
			break;

			default:
				tile_number |= attributes<<3 & 0x300;
				color = attributes & 0x0f;
		}

		drawgfx(bitmap,gfx,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,pen_mode,15);
	}
}

static void tdfever_draw_tx(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int attributes, int dx, int dy, int base )
{
	const UINT8 *source = snk_rambase - 0xd000 + base;
	const gfx_element *gfx = machine->gfx[0];

	int tile_high = (attributes & 0xf0) << 4;
	int color = attributes & 0xf;
	int tile_number, sx, sy;
	int x, y;

	for(x = 0; x < 64; x++) for(y = 0; y < 32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(tile_number == 0x20) continue;

		sx = dx + x*8;
		sy = dy + y*8;

		drawgfx(bitmap,gfx,tile_high|tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN,15);
	}
}

/**************************************************************/

VIDEO_UPDATE( tdfever )
{
	const UINT8 *ram = snk_rambase - 0xd000;

	UINT8 bg_attributes = ram[0xc880];
	UINT8 sp_attributes = ram[0xc900];
	UINT8 tx_attributes = ram[0xc8c0];
	int bg_scroll_x = -ram[0xc840] + ((bg_attributes & 0x02) ? 256:0);
	int bg_scroll_y = -ram[0xc800] + ((bg_attributes & 0x01) ? 256:0);
	int sp16_scroll_x = -ram[0xc9c0] + ((sp_attributes & 0x40) ? 0:256);
	int sp16_scroll_y = -ram[0xc980] + ((sp_attributes & 0x80) ? 256:0);

	// TODO bg_attribute & 0x10 is screen flip

	if(snk_gamegroup == 3 || snk_gamegroup == 5) // tdfever, tdfeverj
	{
			bg_scroll_x += 143;
			bg_scroll_y += -32;
			sp16_scroll_x += 135;
			sp16_scroll_y += -65;
	}
	else if(snk_gamegroup == 7) // ftsoccer
	{
			bg_scroll_x += 16;
			bg_scroll_y += 0;
			sp16_scroll_x += 40;
			sp16_scroll_y += -31;
	}
	tdfever_draw_bg(screen->machine, bitmap, cliprect, bg_scroll_x, bg_scroll_y );

	spriteram = snk_rambase + 0x1000;
	tdfever_draw_sp(screen->machine, bitmap, cliprect, sp16_scroll_x, sp16_scroll_y, 0, 1 );

	tdfever_draw_tx(screen->machine, bitmap, cliprect, tx_attributes, 0, 0, 0xf800 );
	return 0;
}

VIDEO_UPDATE( gwar )
{
	tilemap_set_scrollx(bg_tilemap, 0, bg_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	if(unknown_reg & 0xf8) // improves priority
	{
		tdfever_draw_sp(screen->machine, bitmap, cliprect, -sp16_scrollx - 9, -sp16_scrolly - 15, 2, 0 );
		tdfever_draw_sp(screen->machine, bitmap, cliprect, -sp32_scrollx - 9, -sp32_scrolly - 31, 1, 0 );
	}
	else
	{
		tdfever_draw_sp(screen->machine, bitmap, cliprect, -sp32_scrollx - 9, -sp32_scrolly - 31, 1, 0 );
		tdfever_draw_sp(screen->machine, bitmap, cliprect, -sp16_scrollx - 9, -sp16_scrolly - 15, 2, 0 );
	}

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}


VIDEO_UPDATE( old_gwar )
{
	const UINT8 *ram = snk_rambase - 0xd000;
	int gwar_sp_baseaddr, gwar_tx_baseaddr;
	UINT8 bg_attribute;

	if(snk_gamegroup == 4) // gwara
	{
		gwar_sp_baseaddr = 0xf000;
		gwar_tx_baseaddr = 0xc800;
	}
	else
	{
		gwar_sp_baseaddr = 0xc000;
		gwar_tx_baseaddr = 0xf800;
	}

	bg_attribute = ram[gwar_sp_baseaddr+0x880];

	// TODO bg_attribute & 0x04 is screen flip

	{
		int bg_scroll_y, bg_scroll_x;

		bg_scroll_x = -ram[gwar_sp_baseaddr+0x840] + 16;
		bg_scroll_y = -ram[gwar_sp_baseaddr+0x800];

		bg_scroll_x += (bg_attribute & 2) ? 256:0;
 		bg_scroll_y += (bg_attribute & 1) ? 256:0;

		tdfever_draw_bg(screen->machine, bitmap, cliprect, bg_scroll_x, bg_scroll_y );
	}

	{
		UINT8 sp_attribute = ram[gwar_sp_baseaddr+0xac0];
		int sp16_x = -ram[gwar_sp_baseaddr+0x940] - 9;
		int sp16_y = -ram[gwar_sp_baseaddr+0x900] - 15;
		int sp32_x = -ram[gwar_sp_baseaddr+0x9c0] - 9;
		int sp32_y = -ram[gwar_sp_baseaddr+0x980] - 31;

		if(snk_gamegroup == 2) // gwar, gwarj, gwarb, choppera
		{
			sp16_y += (bg_attribute & 0x10) ? 256:0;
			sp16_x += (bg_attribute & 0x40) ? 256:0;
			sp32_y += (bg_attribute & 0x20) ? 256:0;
			sp32_x += (bg_attribute & 0x80) ? 256:0;
		}
		else
		{
			UINT8 spp_attribute = ram[gwar_sp_baseaddr+0xa80];
			sp16_x += (spp_attribute & 0x10) ? 256:0;
			sp16_y += (spp_attribute & 0x04) ? 256:0;
			sp32_x += (spp_attribute & 0x20) ? 256:0;
			sp32_y += (spp_attribute & 0x08) ? 256:0;
		}

		spriteram = snk_rambase + 0x1000;

		if(sp_attribute & 0xf8) // improves priority
		{
			tdfever_draw_sp(screen->machine, bitmap, cliprect, sp16_x, sp16_y, 2, 0 );
			tdfever_draw_sp(screen->machine, bitmap, cliprect, sp32_x, sp32_y, 1, 0 );
		}
		else
		{
			tdfever_draw_sp(screen->machine, bitmap, cliprect, sp32_x, sp32_y, 1, 0 );
			tdfever_draw_sp(screen->machine, bitmap, cliprect, sp16_x, sp16_y, 2, 0 );
		}
	}

	{
		UINT8 text_attribute = ram[gwar_sp_baseaddr+0x8c0];
		tdfever_draw_tx(screen->machine, bitmap, cliprect, text_attribute, 0, 0, gwar_tx_baseaddr );
	}
	return 0;
}
