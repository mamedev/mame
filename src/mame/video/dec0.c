/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    Each game uses the MXC-06 chip to produce sprites.

    Sprite data:  The unknown bits seem to be unused.

    Byte 0:
        Bit 0 : Y co-ord hi bit
        Bit 1,2 : Sprite width (1x, 2x, 4x, 8x) - NOT YET EMULATED (todo)
        Bit 3,4 : Sprite height (1x, 2x, 4x, 8x)
        Bit 5  - X flip
        Bit 6  - Y flip
        Bit 7  - Only display Sprite if set
    Byte 1: Y-coords
    Byte 2:
        Bit 0,1,2,3: Hi bits of sprite number
        Bit 4,5,6,7: (Probably unused MSB's of sprite)
    Byte 3: Low bits of sprite number
    Byte 4:
        Bit 0 : X co-ords hi bit
        Bit 1,2: ??
        Bit 3: Sprite flash (sprite is displayed every other frame)
        Bit 4,5,6,7:  - Colour
    Byte 5: X-coords

**********************************************************************

  Palette data

    0x000 - character palettes (Sprites on Midnight R)
    0x200 - sprite palettes (Characters on Midnight R)
    0x400 - tiles 1
    0x600 - tiles 2

    Bad Dudes, Robocop, Heavy Barrel, Hippodrome - 24 bit rgb
    Sly Spy, Midnight Resistance - 12 bit rgb

  Tile data

    4 bit palette select, 12 bit tile select

**********************************************************************

 All games contain three BAC06 background generator chips, usual (software)
configuration is 2 chips of 16*16 tiles, 1 of 8*8.

 Playfield control registers:
   bank 0:
   0:
        bit 0 (0x1) set = 8*8 tiles, else 16*16 tiles
        Bit 1 (0x2) unknown
        bit 2 (0x4) set enables rowscroll
        bit 3 (0x8) set enables colscroll
        bit 7 (0x80) set in playfield 1 is reverse screen (set via dip-switch)
        bit 7 (0x80) in other playfields unknown
   2: unknown (00 in bg, 03 in fg+text - maybe controls pf transparency?)
   4: unknown (always 00)
   6: playfield shape: 00 = 4x1, 01 = 2x2, 02 = 1x4 (low 4 bits only)

   bank 1:
   0: horizontal scroll
   2: vertical scroll
   4: colscroll shifter (low 4 bits, top 4 bits do nothing)
   6: rowscroll shifter (low 4 bits, top 4 bits do nothing)

   Row & column scroll can be applied simultaneously or by themselves.
   The shift register controls the granularity of the scroll offsets
   (more details given later).

Playfield priority (Bad Dudes, etc):
    In the bottommost playfield, pens 8-15 can have priority over the next playfield.
    In that next playfield, pens 8-15 can have priority over sprites.

Bit 0:  Playfield inversion
Bit 1:  Enable playfield mixing (for palettes 8-15 only)
Bit 2:  Enable playfield/sprite mixing (for palettes 8-15 only)

Priority word (Midres):
    Bit 0 set = Playfield 3 drawn over Playfield 2
            ~ = Playfield 2 drawn over Playfield 3
    Bit 1 set = Sprites are drawn inbetween playfields
            ~ = Sprites are on top of playfields
    Bit 2
    Bit 3 set = ...

Todo:
    Implement multi-width sprites (used by Birdtry).
    Implement sprite/tilemap orthogonality (not strictly needed as no
    games make deliberate use of it).

***************************************************************************/

#include "driver.h"
#include "includes/dec0.h"

static tilemap *pf1_tilemap_0,*pf1_tilemap_1,*pf1_tilemap_2;
static tilemap *pf2_tilemap_0,*pf2_tilemap_1,*pf2_tilemap_2;
static tilemap *pf3_tilemap_0,*pf3_tilemap_1,*pf3_tilemap_2;

UINT16 *dec0_pf1_data,*dec0_pf2_data,*dec0_pf3_data;
UINT16 *dec0_pf1_rowscroll,*dec0_pf2_rowscroll,*dec0_pf3_rowscroll;
UINT16 *dec0_pf1_colscroll,*dec0_pf2_colscroll,*dec0_pf3_colscroll;
static UINT16 dec0_pf1_control_0[4];
static UINT16 dec0_pf1_control_1[4];
static UINT16 dec0_pf2_control_0[4];
static UINT16 dec0_pf2_control_1[4];
static UINT16 dec0_pf3_control_0[4];
static UINT16 dec0_pf3_control_1[4];
static UINT16 *dec0_spriteram;
static UINT16 dec0_pri;

/******************************************************************************/

WRITE16_HANDLER( dec0_update_sprites_w )
{
	memcpy(dec0_spriteram,spriteram16,0x800);
}

/******************************************************************************/

static void update_24bitcol(int offset)
{
	int r,g,b;

	r = (paletteram16[offset] >> 0) & 0xff;
	g = (paletteram16[offset] >> 8) & 0xff;
	b = (paletteram16_2[offset] >> 0) & 0xff;

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( dec0_paletteram_rg_w )
{
	COMBINE_DATA(&paletteram16[offset]);
	update_24bitcol(offset);
}

WRITE16_HANDLER( dec0_paletteram_b_w )
{
	COMBINE_DATA(&paletteram16_2[offset]);
	update_24bitcol(offset);
}

/******************************************************************************/

static void draw_sprites(running_machine* machine, mame_bitmap *bitmap,const rectangle *cliprect,int pri_mask,int pri_val)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		y = dec0_spriteram[offs];
		if ((y&0x8000) == 0) continue;

		x = dec0_spriteram[offs+2];
		colour = x >> 12;
		if ((colour & pri_mask) != pri_val) continue;

		flash=x&0x800;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = dec0_spriteram[offs+1] & 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x>256) continue; /* Speedup */

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen) {
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
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

static void custom_tilemap_draw(running_machine* machine,
								mame_bitmap *bitmap,
								const rectangle *cliprect,
								tilemap *tilemap_ptr,
								const UINT16 *rowscroll_ptr,
								const UINT16 *colscroll_ptr,
								const UINT16 *control0,
								const UINT16 *control1,
								int flags)
{
	const mame_bitmap *src_bitmap = tilemap_get_pixmap(tilemap_ptr);
	int x, y, p;
	int column_offset=0, src_x=0, src_y=0;
	UINT32 scrollx=control1[0];
	UINT32 scrolly=control1[1];
	int width_mask = src_bitmap->width - 1;
	int height_mask = src_bitmap->height - 1;
	int row_scroll_enabled = (rowscroll_ptr && (control0[0]&0x4));
	int col_scroll_enabled = (colscroll_ptr && (control0[0]&0x8));

	if (!src_bitmap)
		return;

	/* Column scroll & row scroll may per applied per pixel, there are
    shift registers for each which control the granularity of the row/col
    offset (down to per line level for row, and per 8 lines for column).

    Nb:  The row & col selectors are _not_ affected by the shape of the
    playfield (ie, 256*1024, 512*512 or 1024*256).  So even if the tilemap
    width is only 256, 'src_x' should not wrap at 256 in the code below (to
    do so would mean the top half of row RAM would never be accessed which
    is incorrect).

    Nb2:  Real hardware exhibits a strange bug with column scroll on 'mode 2'
    (256*1024) - the first column has a strange additional offset, but
    curiously the first 'wrap' (at scroll offset 256) does not have this offset,
    it is displayed as expected.  The bug is confimed to only affect this mode,
    the other two modes work as expected.  This bug is not emulated, as it
    doesn't affect any games.
    */

	if (flip_screen)
		src_y = (src_bitmap->height - 256) - scrolly;
	else
		src_y = scrolly;

	for (y=0; y<=cliprect->max_y; y++) {
		if (row_scroll_enabled)
			src_x=scrollx + rowscroll_ptr[(src_y >> (control1[3]&0xf))&(0x1ff>>(control1[3]&0xf))];
		else
			src_x=scrollx;

		if (flip_screen)
			src_x=(src_bitmap->width - 256) - src_x;

		for (x=0; x<=cliprect->max_x; x++) {
			if (col_scroll_enabled)
				column_offset=colscroll_ptr[((src_x >> 3) >> (control1[2]&0xf))&(0x3f>>(control1[2]&0xf))];

			p = *BITMAP_ADDR16(src_bitmap, (src_y + column_offset)&height_mask, src_x&width_mask);

			src_x++;
			if ((flags&TILEMAP_DRAW_OPAQUE) || (p&0xf))
			{
				if( flags & TILEMAP_DRAW_LAYER0 )
				{
					/* Top 8 pens of top 8 palettes only */
					if ((p&0x88)==0x88)
						*BITMAP_ADDR16(bitmap, y, x) = machine->pens[p];
				}
				else
				{
					*BITMAP_ADDR16(bitmap, y, x) = machine->pens[p];
				}
			}
		}
		src_y++;
	}
}

/******************************************************************************/

static void dec0_pf1_draw(running_machine* machine,mame_bitmap *bitmap,const rectangle *cliprect,int flags)
{
	switch (dec0_pf1_control_0[3]&0x3) {
		case 0:	/* 4x1 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf1_tilemap_0,dec0_pf1_rowscroll,dec0_pf1_colscroll,dec0_pf1_control_0,dec0_pf1_control_1,flags);
			break;
		case 1:	/* 2x2 */
		default:
			custom_tilemap_draw(machine,bitmap,cliprect,pf1_tilemap_1,dec0_pf1_rowscroll,dec0_pf1_colscroll,dec0_pf1_control_0,dec0_pf1_control_1,flags);
			break;
		case 2:	/* 1x4 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf1_tilemap_2,dec0_pf1_rowscroll,dec0_pf1_colscroll,dec0_pf1_control_0,dec0_pf1_control_1,flags);
			break;
	};
}

static void dec0_pf2_draw(running_machine* machine,mame_bitmap *bitmap,const rectangle *cliprect,int flags)
{
	switch (dec0_pf2_control_0[3]&0x3) {
		case 0:	/* 4x1 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf2_tilemap_0,dec0_pf2_rowscroll,dec0_pf2_colscroll,dec0_pf2_control_0,dec0_pf2_control_1,flags);
			break;
		case 1:	/* 2x2 */
		default:
			custom_tilemap_draw(machine,bitmap,cliprect,pf2_tilemap_1,dec0_pf2_rowscroll,dec0_pf2_colscroll,dec0_pf2_control_0,dec0_pf2_control_1,flags);
			break;
		case 2:	/* 1x4 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf2_tilemap_2,dec0_pf2_rowscroll,dec0_pf2_colscroll,dec0_pf2_control_0,dec0_pf2_control_1,flags);
			break;
	};
}

static void dec0_pf3_draw(running_machine* machine,mame_bitmap *bitmap,const rectangle *cliprect,int flags)
{
	switch (dec0_pf3_control_0[3]&0x3) {
		case 0:	/* 4x1 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf3_tilemap_0,dec0_pf3_rowscroll,dec0_pf3_colscroll,dec0_pf3_control_0,dec0_pf3_control_1,flags);
			break;
		case 1:	/* 2x2 */
		default:
			custom_tilemap_draw(machine,bitmap,cliprect,pf3_tilemap_1,dec0_pf3_rowscroll,dec0_pf3_colscroll,dec0_pf3_control_0,dec0_pf3_control_1,flags);
			break;
		case 2:	/* 1x4 */
			custom_tilemap_draw(machine,bitmap,cliprect,pf3_tilemap_2,dec0_pf3_rowscroll,dec0_pf3_colscroll,dec0_pf3_control_0,dec0_pf3_control_1,flags);
			break;
	};
}

/******************************************************************************/

VIDEO_UPDATE( hbarrel )
{
	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
	draw_sprites(machine,bitmap,cliprect,0x08,0x08);
	dec0_pf2_draw(machine,bitmap,cliprect,0);

	/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	draw_sprites(machine,bitmap,cliprect,0x08,0x00);
	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( baddudes )
{
	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	/* WARNING: inverted wrt Midnight Resistance */
	if ((dec0_pri & 0x01) == 0)
	{
		dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		dec0_pf3_draw(machine,bitmap,cliprect,0);

		if (dec0_pri & 2)
			dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */

		draw_sprites(machine,bitmap,cliprect,0x00,0x00);

		if (dec0_pri & 4)
			dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */
	}
	else
	{
		dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		dec0_pf2_draw(machine,bitmap,cliprect,0);

		if (dec0_pri & 2)
			dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */

		draw_sprites(machine,bitmap,cliprect,0x00,0x00);

		if (dec0_pri & 4)
			dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0); /* Foreground pens only */
	}

	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( robocop )
{
	int trans;

	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	if (dec0_pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (dec0_pri & 0x01)
	{
		/* WARNING: inverted wrt Midnight Resistance */
		/* Robocop uses it only for the title screen, so this might be just */
		/* completely wrong. The top 8 bits of the register might mean */
		/* something (they are 0x80 in midres, 0x00 here) */
		dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER1|TILEMAP_DRAW_OPAQUE);

		if (dec0_pri & 0x02)
			draw_sprites(machine,bitmap,cliprect,0x08,trans);

		dec0_pf3_draw(machine,bitmap,cliprect,0);
	}
	else
	{
		dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (dec0_pri & 0x02)
			draw_sprites(machine,bitmap,cliprect,0x08,trans);

		dec0_pf2_draw(machine,bitmap,cliprect,0);
	}

	if (dec0_pri & 0x02)
		draw_sprites(machine,bitmap,cliprect,0x08,trans ^ 0x08);
	else
		draw_sprites(machine,bitmap,cliprect,0x00,0x00);

	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( birdtry )
{
	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	/* This game doesn't have the extra playfield chip on the game board, but
    the palette does show through. */
	fillbitmap(bitmap,machine->pens[768],cliprect);
	dec0_pf2_draw(machine,bitmap,cliprect,0);
	draw_sprites(machine,bitmap,cliprect,0x00,0x00);
	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( hippodrm )
{
	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	if (dec0_pri & 0x01)
	{
		dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		dec0_pf3_draw(machine,bitmap,cliprect,0);
	}
	else
	{
		dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
		dec0_pf2_draw(machine,bitmap,cliprect,0);
	}

	draw_sprites(machine,bitmap,cliprect,0x00,0x00);
	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( slyspy )
{
	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);
	dec0_pf2_draw(machine,bitmap,cliprect,0);

	draw_sprites(machine,bitmap,cliprect,0x00,0x00);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (dec0_pri&0x80)
		dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_LAYER0);

	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

VIDEO_UPDATE( midres )
{
	int trans;

	flip_screen_set(dec0_pf1_control_0[0]&0x80);

	if (dec0_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

	if (dec0_pri & 0x01)
	{
		dec0_pf2_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (dec0_pri & 0x02)
			draw_sprites(machine,bitmap,cliprect,0x08,trans);

		dec0_pf3_draw(machine,bitmap,cliprect,0);
	}
	else
	{
		dec0_pf3_draw(machine,bitmap,cliprect,TILEMAP_DRAW_OPAQUE);

		if (dec0_pri & 0x02)
			draw_sprites(machine,bitmap,cliprect,0x08,trans);

		dec0_pf2_draw(machine,bitmap,cliprect,0);
	}

	if (dec0_pri & 0x02)
		draw_sprites(machine,bitmap,cliprect,0x08,trans ^ 0x08);
	else
		draw_sprites(machine,bitmap,cliprect,0x00,0x00);

	dec0_pf1_draw(machine,bitmap,cliprect,0);
	return 0;
}

/******************************************************************************/

WRITE16_HANDLER( dec0_pf1_control_0_w )
{
	COMBINE_DATA(&dec0_pf1_control_0[offset]);
}

WRITE16_HANDLER( dec0_pf1_control_1_w )
{
	COMBINE_DATA(&dec0_pf1_control_1[offset]);
}

WRITE16_HANDLER( dec0_pf1_data_w )
{
	COMBINE_DATA(&dec0_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap_0,offset);
	tilemap_mark_tile_dirty(pf1_tilemap_1,offset);
	tilemap_mark_tile_dirty(pf1_tilemap_2,offset);
}

WRITE16_HANDLER( dec0_pf2_control_0_w )
{
	COMBINE_DATA(&dec0_pf2_control_0[offset]);
}

WRITE16_HANDLER( dec0_pf2_control_1_w )
{
	COMBINE_DATA(&dec0_pf2_control_1[offset]);
}

WRITE16_HANDLER( dec0_pf2_data_w )
{
	COMBINE_DATA(&dec0_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap_0,offset);
	tilemap_mark_tile_dirty(pf2_tilemap_1,offset);
	tilemap_mark_tile_dirty(pf2_tilemap_2,offset);
}

WRITE16_HANDLER( dec0_pf3_control_0_w )
{
	COMBINE_DATA(&dec0_pf3_control_0[offset]);
}

WRITE16_HANDLER( dec0_pf3_control_1_w )
{
	COMBINE_DATA(&dec0_pf3_control_1[offset]);
}

WRITE16_HANDLER( dec0_pf3_data_w )
{
	COMBINE_DATA(&dec0_pf3_data[offset]);
	tilemap_mark_tile_dirty(pf3_tilemap_0,offset);
	tilemap_mark_tile_dirty(pf3_tilemap_1,offset);
	tilemap_mark_tile_dirty(pf3_tilemap_2,offset);
}

WRITE16_HANDLER( dec0_priority_w )
{
  	COMBINE_DATA(&dec0_pri);
}

WRITE8_HANDLER( dec0_pf3_control_8bit_w )
{
	static int buffer[0x20];
	UINT16 myword;

	buffer[offset]=data;

	/* Rearrange little endian bytes from H6280 into big endian words for 68k */
	offset&=0xffe;
	myword=buffer[offset] + (buffer[offset+1]<<8);

	if (offset<0x10) dec0_pf3_control_0_w(offset/2,myword,0);
	else dec0_pf3_control_1_w((offset-0x10)/2,myword,0);
}

WRITE8_HANDLER( dec0_pf3_data_8bit_w )
{
	if (offset&1) { /* MSB has changed */
		UINT16 lsb=dec0_pf3_data[offset>>1];
		UINT16 newword=(lsb&0xff) | (data<<8);
		dec0_pf3_data[offset>>1]=newword;
	}
	else { /* LSB has changed */
		UINT16 msb=dec0_pf3_data[offset>>1];
		UINT16 newword=(msb&0xff00) | data;
		dec0_pf3_data[offset>>1]=newword;
	}
	tilemap_mark_tile_dirty(pf3_tilemap_0,offset>>1);
	tilemap_mark_tile_dirty(pf3_tilemap_1,offset>>1);
	tilemap_mark_tile_dirty(pf3_tilemap_2,offset>>1);
}

READ8_HANDLER( dec0_pf3_data_8bit_r )
{
	if (offset&1) /* MSB */
		return dec0_pf3_data[offset>>1]>>8;

	return dec0_pf3_data[offset>>1]&0xff;
}

/******************************************************************************/

static TILEMAP_MAPPER( tile_shape0_scan )
{
	return (col & 0xf) + ((row & 0xf) << 4) + ((col & 0x30) << 4);
}

static TILEMAP_MAPPER( tile_shape1_scan )
{
	return (col & 0xf) + ((row & 0xf) << 4) + ((row & 0x10) << 4) + ((col & 0x10) << 5);
}

static TILEMAP_MAPPER( tile_shape2_scan )
{
	return (col & 0xf) + ((row & 0x3f) << 4);
}

static TILEMAP_MAPPER( tile_shape0_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

static TILEMAP_MAPPER( tile_shape1_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

static TILEMAP_MAPPER( tile_shape2_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x7f) << 5);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	int tile=dec0_pf1_data[tile_index];
	SET_TILE_INFO(0,tile&0xfff,tile>>12,0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	int tile=dec0_pf2_data[tile_index];
	int pri=((tile>>12)>7);
	SET_TILE_INFO(1,tile&0xfff,tile>>12,0);
	tileinfo->group = pri;
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	int tile=dec0_pf3_data[tile_index];
	int pri=((tile>>12)>7);
	SET_TILE_INFO(2,tile&0xfff,tile>>12,0);
	tileinfo->group = pri;
}

VIDEO_START( dec0_nodma )
{
	pf1_tilemap_0 = tilemap_create(get_pf1_tile_info,tile_shape0_8x8_scan,TILEMAP_TYPE_PEN, 8, 8,128, 32);
	pf1_tilemap_1 = tilemap_create(get_pf1_tile_info,tile_shape1_8x8_scan,TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	pf1_tilemap_2 = tilemap_create(get_pf1_tile_info,tile_shape2_8x8_scan,TILEMAP_TYPE_PEN, 8, 8, 32,128);
	pf2_tilemap_0 = tilemap_create(get_pf2_tile_info,tile_shape0_scan,    TILEMAP_TYPE_PEN,16,16, 64, 16);
	pf2_tilemap_1 = tilemap_create(get_pf2_tile_info,tile_shape1_scan,    TILEMAP_TYPE_PEN,16,16, 32, 32);
	pf2_tilemap_2 = tilemap_create(get_pf2_tile_info,tile_shape2_scan,    TILEMAP_TYPE_PEN,16,16, 16, 64);
	pf3_tilemap_0 = tilemap_create(get_pf3_tile_info,tile_shape0_scan,    TILEMAP_TYPE_PEN,16,16, 64, 16);
	pf3_tilemap_1 = tilemap_create(get_pf3_tile_info,tile_shape1_scan,    TILEMAP_TYPE_PEN,16,16, 32, 32);
	pf3_tilemap_2 = tilemap_create(get_pf3_tile_info,tile_shape2_scan,    TILEMAP_TYPE_PEN,16,16, 16, 64);

	dec0_spriteram=spriteram16;
}

VIDEO_START( dec0 )
{
	video_start_dec0_nodma(machine);
	dec0_spriteram=auto_malloc(0x800);
}

/******************************************************************************/
