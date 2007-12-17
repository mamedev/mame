/*** Video *******************************************************************/
/* see drivers/pgm.c for notes on where improvements can be made */

#include "driver.h"

extern UINT16 *pgm_mainram, *pgm_bg_videoram, *pgm_tx_videoram, *pgm_videoregs, *pgm_rowscrollram;
static tilemap *pgm_tx_tilemap, *pgm_bg_tilemap;
static UINT16 *pgm_spritebufferram; // buffered spriteram
static UINT16 *sprite_temp_render;

extern UINT8 *pgm_sprite_a_region;   /* = memory_region       ( REGION_GFX4 ); */
extern size_t	pgm_sprite_a_region_allocate;

/* Sprites - These are a pain! */

/* this decodes one of the funky sprites to a bitmap so we can draw it more easily -- slow but easier to use*/
static void pgm_prepare_sprite(int wide, int high,int palt, int boffset)
{
	UINT8 *bdata    = memory_region       ( REGION_GFX4 );
	size_t  bdatasize = memory_region_length( REGION_GFX4 )-1;
	UINT8 *adata    = pgm_sprite_a_region;
	size_t  adatasize = pgm_sprite_a_region_allocate-1;
	int xcnt, ycnt;

	UINT32 aoffset;
	UINT16 msk;

	aoffset = (bdata[(boffset+3) & bdatasize] << 24) | (bdata[(boffset+2) & bdatasize] << 16) | (bdata[(boffset+1) & bdatasize] << 8) | (bdata[(boffset+0) & bdatasize] << 0);
	aoffset = aoffset >> 2; aoffset *= 3;

	boffset += 4; /* because the first dword is the a data offset */

	for (ycnt = 0 ; ycnt < high ; ycnt++) {
		for (xcnt = 0 ; xcnt < wide ; xcnt++) {
			int x;

			msk = (( bdata[(boffset+1) & bdatasize] << 8) |( bdata[(boffset+0) & bdatasize] << 0) );

			for (x=0;x<16;x++)
			{
				if (!(msk & 0x0001))
				{
					sprite_temp_render[(ycnt*(wide*16))+(xcnt*16+x)]  = adata[aoffset & adatasize]+ palt*32;
					aoffset++;
				}
				else
				{
					sprite_temp_render[(ycnt*(wide*16))+(xcnt*16+x)] = 0x8000;
				}
				msk >>=1;
			}

			boffset+=2;
		}
	}
}



static void draw_sprite_line(int wide, UINT16* dest, int xzoom, int xgrow, int yoffset, int flip, int xpos)
{
	int xcnt,xcntdraw;
	int xzoombit;
	int xoffset;
	int xdrawpos = 0;

	xcnt = 0;
	xcntdraw = 0;
	while (xcnt < wide*16)
	{
		UINT32 srcdat;
		if (!(flip&0x01)) xoffset = xcnt;
		else xoffset = (wide*16)-xcnt-1;

		srcdat = sprite_temp_render[yoffset+xoffset];
		xzoombit = (xzoom >> (xcnt&0x1f))&1;

		if (xzoombit == 1 && xgrow ==1)
		{ // double this column
			xdrawpos = xpos + xcntdraw;
			if (!(srcdat&0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))  dest[xdrawpos] = srcdat;
			}
			xcntdraw++;

			xdrawpos = xpos + xcntdraw;

			if (!(srcdat&0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))  dest[xdrawpos] = srcdat;
			}
			xcntdraw++;
		}
		else if (xzoombit ==1 && xgrow ==0)
		{
			/* skip this column */
		}
		else //normal column
		{
			xdrawpos = xpos + xcntdraw;
			if (!(srcdat&0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < 448))  dest[xdrawpos] = srcdat;
			}
			xcntdraw++;
		}

		xcnt++;

		if (xdrawpos == 448) xcnt = wide*16;
	}
}
/* this just loops over our decoded bitmap and puts it on the screen */
static void draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int boffset, int flip, mame_bitmap* bitmap, UINT32 xzoom, int xgrow, UINT32 yzoom, int ygrow )
{
	int ycnt;
	int ydrawpos;
	UINT16 *dest;
	int yoffset;
	int ycntdraw;
	int yzoombit;

	pgm_prepare_sprite( wide,high, palt, boffset );

	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;
	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt&0x1f))&1;

		if (yzoombit == 1 && ygrow == 1) // double this line
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR16(bitmap, ydrawpos, 0);
				draw_sprite_line(wide, dest, xzoom, xgrow, yoffset, flip, xpos);
			}
			ycntdraw++;

			ydrawpos = ypos + ycntdraw;
			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR16(bitmap, ydrawpos, 0);
				draw_sprite_line(wide, dest, xzoom, xgrow, yoffset, flip, xpos);
			}
			ycntdraw++;

			if (ydrawpos ==224) ycnt = high;
		}
		else if (yzoombit ==1 && ygrow == 0)
		{
			/* skip this line */
			/* we should process anyway if we don't do the pre-decode.. */
		}
		else /* normal line */
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = BITMAP_ADDR16(bitmap, ydrawpos, 0);
				draw_sprite_line(wide, dest, xzoom, xgrow, yoffset, flip, xpos);
			}
			ycntdraw++;

			if (ydrawpos ==224) ycnt = high;
		}

		ycnt++;
	}
}


static UINT16 *pgm_sprite_source;

static void draw_sprites(int priority, mame_bitmap* bitmap)
{
	/* ZZZZ Zxxx xxxx xxxx
       zzzz z-yy yyyy yyyy
       -ffp pppp Pvvv vvvv
       vvvv vvvv vvvv vvvv
       wwww wwwh hhhh hhhh
    */


	const UINT16 *finish = pgm_spritebufferram+(0xa00/2);

	while( pgm_sprite_source<finish )
	{
		int xpos = pgm_sprite_source[0] & 0x07ff;
		int ypos = pgm_sprite_source[1] & 0x03ff;
		int xzom = (pgm_sprite_source[0] & 0x7800) >> 11;
		int xgrow = (pgm_sprite_source[0] & 0x8000) >> 15;
		int yzom = (pgm_sprite_source[1] & 0x7800) >> 11;
		int ygrow = (pgm_sprite_source[1] & 0x8000) >> 15;
		int palt = (pgm_sprite_source[2] & 0x1f00) >> 8;
		int flip = (pgm_sprite_source[2] & 0x6000) >> 13;
		int boff = ((pgm_sprite_source[2] & 0x007f) << 16) | (pgm_sprite_source[3] & 0xffff);
		int wide = (pgm_sprite_source[4] & 0x7e00) >> 9;
		int high = pgm_sprite_source[4] & 0x01ff;
		int pri = (pgm_sprite_source[2] & 0x0080) >>  7;

		UINT32 xzoom, yzoom;

		UINT16* pgm_sprite_zoomtable = &pgm_videoregs[0x1000/2];

		if (xgrow)
		{
		//  xzom = 0xf-xzom; // would make more sense but everything gets zoomed slightly in dragon world 2 ?!
			xzom = 0x10-xzom; // this way it doesn't but there is a bad line when zooming after the level select?
		}

		if (ygrow)
		{
		//  yzom = 0xf-yzom; // see comment above
			yzom = 0x10-yzom;
		}

		xzoom = (pgm_sprite_zoomtable[xzom*2]<<16)|pgm_sprite_zoomtable[xzom*2+1];
		yzoom = (pgm_sprite_zoomtable[yzom*2]<<16)|pgm_sprite_zoomtable[yzom*2+1];

		boff *= 2;
		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		if (high == 0) break; /* is this right? */

		if ((priority == 1) && (pri == 0)) break;

		draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, boff, flip, bitmap, xzoom,xgrow, yzoom,ygrow);

		pgm_sprite_source += 5;
	}
}

/* TX Layer */

WRITE16_HANDLER( pgm_tx_videoram_w )
{
	pgm_tx_videoram[offset] = data;
	tilemap_mark_tile_dirty(pgm_tx_tilemap,offset/2);
}

static TILE_GET_INFO( get_pgm_tx_tilemap_tile_info )
{

/* 0x904000 - 0x90ffff is the Text Overlay Ram (pgm_tx_videoram)
    each tile uses 4 bytes, the tilemap is 64x128?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA017FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	int tileno,colour,flipyx; //,game;

	tileno = pgm_tx_videoram[tile_index *2] & 0xffff;
	colour = (pgm_tx_videoram[tile_index*2+1] & 0x3e) >> 1;
	flipyx = (pgm_tx_videoram[tile_index*2+1] & 0xc0) >> 6;

	if (tileno > 0xbfff) { tileno -= 0xc000 ; tileno += 0x20000; } /* not sure about this */

	SET_TILE_INFO(0,tileno,colour,TILE_FLIPYX(flipyx));
}

/* BG Layer */

WRITE16_HANDLER( pgm_bg_videoram_w )
{
	pgm_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(pgm_bg_tilemap,offset/2);
}

static TILE_GET_INFO( get_pgm_bg_tilemap_tile_info )
{
	/* pretty much the same as tx layer */

	int tileno,colour,flipyx;

	tileno = pgm_bg_videoram[tile_index *2] & 0xffff;
	if (tileno > 0x7ff) tileno+=0x1000; /* Tiles 0x800+ come from the GAME Roms */
	colour = (pgm_bg_videoram[tile_index*2+1] & 0x3e) >> 1;
	flipyx = (pgm_bg_videoram[tile_index*2+1] & 0xc0) >> 6;

	SET_TILE_INFO(1,tileno,colour,TILE_FLIPYX(flipyx));
}



/*** Video - Start / Update ****************************************************/

VIDEO_START( pgm )
{
	pgm_tx_tilemap= tilemap_create(get_pgm_tx_tilemap_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	tilemap_set_transparent_pen(pgm_tx_tilemap,15);

	pgm_bg_tilemap = tilemap_create(get_pgm_bg_tilemap_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 32, 32,64,64);
	tilemap_set_transparent_pen(pgm_bg_tilemap,31);
	tilemap_set_scroll_rows(pgm_bg_tilemap,64*32);

	pgm_spritebufferram = auto_malloc (0xa00);

	/* we render each sprite to a bitmap then copy the bitmap to screen bitmap with zooming */
	/* easier this way because of the funky sprite format */
	sprite_temp_render = auto_malloc(0x400*0x200*2);
}

VIDEO_UPDATE( pgm )
{
	int y;

	fillbitmap(bitmap,get_black_pen(machine),&machine->screen[0].visarea);

	pgm_sprite_source = pgm_spritebufferram;
	draw_sprites(1, bitmap);

	tilemap_set_scrolly(pgm_bg_tilemap,0, pgm_videoregs[0x2000/2]);

	for (y = 0; y < 224; y++)
		tilemap_set_scrollx(pgm_bg_tilemap,(y+pgm_videoregs[0x2000/2])&0x7ff, pgm_videoregs[0x3000/2]+pgm_rowscrollram[y]);

	tilemap_draw(bitmap,cliprect,pgm_bg_tilemap,0,0);

	draw_sprites(0, bitmap);

	tilemap_set_scrolly(pgm_tx_tilemap,0, pgm_videoregs[0x5000/2]);
	tilemap_set_scrollx(pgm_tx_tilemap,0, pgm_videoregs[0x6000/2]); // Check
	tilemap_draw(bitmap,cliprect,pgm_tx_tilemap,0,0);
	return 0;
}

VIDEO_EOF( pgm )
{
	/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
	memcpy(pgm_spritebufferram,pgm_mainram,0xa00);
}
