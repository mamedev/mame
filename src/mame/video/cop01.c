/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *cop01_bgvideoram,*cop01_fgvideoram;

static UINT8 mightguy_vreg[4];
static tilemap *bg_tilemap,*fg_tilemap;



PALETTE_INIT( cop01 )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup tables */

	/* characters use colors 0-15 (or 0-127, but the eight rows are identical) */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* background tiles use colors 192-255 */
	/* I don't know how much of the lookup table PROM is hooked up, */
	/* I'm only using the first 32 bytes because the rest is empty. */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = 0xc0 + (i & 0x30) + (color_prom[((i & 0x40) >> 2) + (i & 0x0f)] & 0x0f);
	color_prom += 256;

	/* sprites use colors 128-143 (or 128-191, but the four rows are identical) */
	for (i = 0;i < TOTAL_COLORS(2);i++)
		COLOR(2,i) = 0x80 + (*(color_prom++) & 0x0f);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile = cop01_bgvideoram[tile_index];
	int attr = cop01_bgvideoram[tile_index+0x800];
	int pri  = (attr & 0x80) >> 7;

	/* kludge: priority is not actually pen based, but color based. Since the
     * game uses a lookup table, the two are not the same thing.
     * Palette entries with bits 2&3 set have priority over sprites.
     * tilemap.c can't handle that yet, so I'm cheating, because I know that
     * color codes using the second row of the lookup table don't use palette
     * entries 12-15.
     * The only place where this has any effect is the beach at the bottom of
     * the screen right at the beginning of mightguy. cop01 doesn't seem to
     * use priority at all.
     */
	if (attr & 0x10) pri = 0;

	SET_TILE_INFO(1,tile + ((attr & 0x03) << 8),(attr & 0x1c) >> 2,0);
	tileinfo->group = pri;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int tile = cop01_fgvideoram[tile_index];
	SET_TILE_INFO(0,tile,0,0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( cop01 )
{
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,      8,8,64,32);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,15);

	/* priority doesn't exactly work this way, see above */
	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x0fff,0xf000); /* split type 1 has pens 0-11 transparent in front half */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( cop01_background_w )
{
	cop01_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( cop01_foreground_w )
{
	cop01_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE8_HANDLER( cop01_vreg_w )
{
	/*  0x40: --xx---- sprite bank, coin counters, flip screen
     *        -----x-- flip screen
     *        ------xx coin counters
     *  0x41: xxxxxxxx xscroll
     *  0x42: ---xx--- ? matches the bg tile color most of the time, but not
     *                 during level transitions. Maybe sprite palette bank?
     *                 (the four banks in the PROM are identical)
     *        ------x- unused (xscroll overflow)
     *        -------x msb xscroll
     *  0x43: xxxxxxxx yscroll
     */
	mightguy_vreg[offset] = data;

	if (offset == 0)
	{
		coin_counter_w(0,data & 1);
		coin_counter_w(1,data & 2);
		flip_screen_set(data & 4);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs,code,attr,sx,sy,flipx,flipy,color;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		code = spriteram[offs+1];
		attr = spriteram[offs+2];
		/* xxxx---- color
         * ----xx-- flipy,flipx
         * -------x msbx
         */
		color = attr>>4;
		flipx = attr & 0x04;
		flipy = attr & 0x08;

		sx = (spriteram[offs+3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - spriteram[offs];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (code&0x80)
			code += (mightguy_vreg[0]&0x30)<<3;

		drawgfx(bitmap,machine->gfx[2],
			code,
			color,
			flipx,flipy,
			sx,sy,
			cliprect,TRANSPARENCY_PEN,0 );
	}
}


VIDEO_UPDATE( cop01 )
{
	tilemap_set_scrollx(bg_tilemap,0,mightguy_vreg[1] + 256 * (mightguy_vreg[2] & 1));
	tilemap_set_scrolly(bg_tilemap,0,mightguy_vreg[3]);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0 );
	return 0;
}
