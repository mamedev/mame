/***************************************************************************

    Uses Data East custom chip 55 for backgrounds, with a special 8bpp mode
    2 times custom chips 52/71 for sprites.

***************************************************************************/

#include "driver.h"

UINT16 *sshangha_pf2_data,*sshangha_pf1_data;
UINT16 *sshangha_pf1_rowscroll,*sshangha_pf2_rowscroll;
static UINT16 sshangha_control_0[8];
static tilemap *pf1_8x8_tilemap,*pf1_16x16_tilemap,*pf2_tilemap;
static int sshangha_pf1_bank,sshangha_pf2_bank,sshangha_video_control;

/******************************************************************************/

WRITE16_HANDLER( sshangha_palette_24bit_w )
{
	int r,g,b;

	COMBINE_DATA(&paletteram16[offset]);
	if (offset&1) offset--;

	b = (paletteram16[offset] >> 0) & 0xff;
	g = (paletteram16[offset+1] >> 8) & 0xff;
	r = (paletteram16[offset+1] >> 0) & 0xff;

	palette_set_color(Machine,offset/2,MAKE_RGB(r,g,b));
}

static void sshangha_tilemap_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const mame_bitmap *bitmap0 = tilemap_get_pixmap(pf1_16x16_tilemap);
	const mame_bitmap *bitmap1 = tilemap_get_pixmap(pf2_tilemap);
	int x,y,p;

	for (y=0; y<240; y++) {
		for (x=0; x<320; x++) {
			p=*BITMAP_ADDR16(bitmap0, y, x)&0xf;
			p|=(*BITMAP_ADDR16(bitmap1, y, x)&0xf)<<4;

			*BITMAP_ADDR16(bitmap, y, x) = machine->pens[p|0x300];
		}
	}
}

WRITE16_HANDLER (sshangha_video_w)
{
	/* 0x4: Special video mode, other bits unknown */
	sshangha_video_control=data;
//  popmessage("%04x",data);
}

/******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT16 *spritesrc, UINT16 pmask, UINT16 pval)
{
	int offs;

	for (offs = 0;offs < 0x800;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spritesrc[offs+1] & 0x3fff;
		if (!sprite) continue;

		if ((spritesrc[offs+2]&pmask)!=pval)
			continue;

		y = spritesrc[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spritesrc[offs+2];
		colour = (x >>9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		sprite &= ~multi;

		if (flip_screen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=-16;
		}

		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		mult=+16;

		if (fx) fx=0; else fx=1;
		if (fy) fy=0; else fy=1;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

/******************************************************************************/

WRITE16_HANDLER( sshangha_pf2_data_w )
{
	COMBINE_DATA(&sshangha_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset);
}

WRITE16_HANDLER( sshangha_pf1_data_w )
{
	COMBINE_DATA(&sshangha_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_8x8_tilemap,offset);
	tilemap_mark_tile_dirty(pf1_16x16_tilemap,offset);
}

WRITE16_HANDLER( sshangha_control_0_w )
{
	COMBINE_DATA(&sshangha_control_0[offset]);
}

/******************************************************************************/

#if 0
static UINT32 sshangha_scan(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}
#endif

static TILE_GET_INFO( get_pf2_tile_info )
{
	UINT16 tile=sshangha_pf2_data[tile_index];
	SET_TILE_INFO(1,(tile&0xfff)|sshangha_pf2_bank,(tile>>12)|32,0);
}

static TILE_GET_INFO( get_pf1_16x16_tile_info )
{
	UINT16 tile=sshangha_pf1_data[tile_index];
	SET_TILE_INFO(1,(tile&0xfff)|sshangha_pf1_bank,tile>>12,0);
}

static TILE_GET_INFO( get_pf1_8x8_tile_info )
{
	UINT16 tile=sshangha_pf1_data[tile_index];
	SET_TILE_INFO(0,(tile&0xfff)|sshangha_pf1_bank,tile>>12,0);
}

VIDEO_START( sshangha )
{
	pf1_8x8_tilemap   = tilemap_create(get_pf1_8x8_tile_info,  tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pf1_16x16_tilemap = tilemap_create(get_pf1_16x16_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	pf2_tilemap = tilemap_create(get_pf2_tile_info,tilemap_scan_rows,    TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(pf1_8x8_tilemap,0);
	tilemap_set_transparent_pen(pf1_16x16_tilemap,0);
}

/******************************************************************************/

VIDEO_UPDATE( sshangha )
{
	static int last_pf1_bank,last_pf2_bank;
	int offs;

	flip_screen=sshangha_control_0[0]&0x80;
	tilemap_set_flip(ALL_TILEMAPS,flip_screen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	tilemap_set_enable( pf2_tilemap, sshangha_control_0[5]&0x8000);
	tilemap_set_enable( pf1_8x8_tilemap, sshangha_control_0[5]&0x0080);
	tilemap_set_enable( pf1_16x16_tilemap, sshangha_control_0[5]&0x0080);

	sshangha_pf1_bank=((sshangha_control_0[7]>> 4)&0xf)*0x1000;
	sshangha_pf2_bank=((sshangha_control_0[7]>>12)&0xf)*0x1000;

	if (sshangha_pf1_bank!=last_pf1_bank) tilemap_mark_all_tiles_dirty(pf1_8x8_tilemap);
	if (sshangha_pf1_bank!=last_pf1_bank) tilemap_mark_all_tiles_dirty(pf1_16x16_tilemap);
	if (sshangha_pf2_bank!=last_pf2_bank) tilemap_mark_all_tiles_dirty(pf2_tilemap);
	last_pf1_bank=sshangha_pf1_bank;
	last_pf2_bank=sshangha_pf2_bank;

	/* Rowscroll - todo, this might actually be col scroll, or both row & col combined.  Check.. */
	if (sshangha_control_0[6]&0x40) {
		tilemap_set_scroll_rows(pf1_8x8_tilemap,256);
		tilemap_set_scroll_rows(pf1_16x16_tilemap,256);
		for (offs=0; offs<256; offs++) {
			tilemap_set_scrollx( pf1_8x8_tilemap,0, sshangha_control_0[1] + sshangha_pf1_rowscroll[offs+0x200] );
			tilemap_set_scrollx( pf1_16x16_tilemap,0, sshangha_control_0[1] + sshangha_pf1_rowscroll[offs+0x200] );
		}
	} else {
		tilemap_set_scroll_rows(pf1_16x16_tilemap,1);
		tilemap_set_scroll_rows(pf1_8x8_tilemap,1);
		tilemap_set_scrollx( pf1_8x8_tilemap,0, sshangha_control_0[1] );
		tilemap_set_scrollx( pf1_16x16_tilemap,0, sshangha_control_0[1] );
	}

	if (sshangha_control_0[6]&0x4000) {
		tilemap_set_scroll_rows(pf2_tilemap,256);
		for (offs=0; offs<256; offs++) {
			tilemap_set_scrollx( pf2_tilemap,0, sshangha_control_0[3] - 3 + sshangha_pf2_rowscroll[offs+0x200] );
		}
	} else {
		tilemap_set_scroll_rows(pf2_tilemap,1);
		tilemap_set_scrollx( pf2_tilemap,0, sshangha_control_0[3] - 3 );
	}

	tilemap_set_scrolly( pf2_tilemap,0, sshangha_control_0[4] );
	tilemap_set_scrolly( pf1_8x8_tilemap,0, sshangha_control_0[2] );
	tilemap_set_scrolly( pf1_16x16_tilemap,0, sshangha_control_0[2] );

	if ((sshangha_control_0[5]&0x8000)==0)
		fillbitmap(bitmap,get_black_pen(machine),cliprect);

	/* Super Shanghai has a mode where the two tilemaps are combined to
    produce a 6bpp tilemap.  We can't precompute this as any tiles can be
    used in any tilemap, so we plot it on the fly */
	if ((sshangha_video_control&4)==0) {
		sshangha_tilemap_draw(machine, bitmap, cliprect);
		draw_sprites(machine, bitmap, cliprect, spriteram16,0x4000,0x4000);
	}
	else {
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
		draw_sprites(machine, bitmap, cliprect, spriteram16,0x4000,0x4000);

		if (sshangha_control_0[6]&0x80)
			tilemap_draw(bitmap,cliprect,pf1_8x8_tilemap,0,0);
		else
			tilemap_draw(bitmap,cliprect,pf1_16x16_tilemap,0,0);
	}

	draw_sprites(machine, bitmap, cliprect, spriteram16_2,0x0000,0x0000);
	draw_sprites(machine, bitmap, cliprect, spriteram16,0x4000,0x0000);
	return 0;
}
