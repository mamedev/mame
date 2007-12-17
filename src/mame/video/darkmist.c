#include "driver.h"

/*defined in drivers/darkmist.c */

extern UINT8 * darkmist_scroll;
extern int darkmist_hw;

static int spritebank;

/* vis. flags */

#define DISPLAY_SPR		1
#define DISPLAY_FG		2 /* 2 or 8 */
#define DISPLAY_BG		4
#define DISPLAY_TXT		16

static tilemap *bgtilemap, *fgtilemap, *txtilemap;

static TILE_GET_INFO( get_bgtile_info )
{
	int code,attr,pal;

	code=memory_region(REGION_USER1)[tile_index]; /* TTTTTTTT */
	attr=memory_region(REGION_USER2)[tile_index]; /* -PPP--TT - FIXED BITS (0xxx00xx) */
	code+=(attr&3)<<8;
	pal=(attr>>4);

	SET_TILE_INFO(
		1,
        code,
        pal,
        0);
}

static TILE_GET_INFO( get_fgtile_info )
{
	int code,attr,pal;

	code=memory_region(REGION_USER3)[tile_index]; /* TTTTTTTT */
	attr=memory_region(REGION_USER4)[tile_index]; /* -PPP--TT - FIXED BITS (0xxx00xx) */
	pal=attr>>4;

	code+=(attr&3)<<8;

	code+=0x400;

	pal+=16;

	SET_TILE_INFO(
		1,
        code,
        pal,
        0);
}

static TILE_GET_INFO( get_txttile_info )
{
	int code,attr,pal;

	code=videoram[tile_index];
	attr=videoram[tile_index+0x400];
	pal=(attr>>1);

	code+=(attr&1)<<8;

	pal+=48;

	SET_TILE_INFO(
		0,
        code,
        pal,
        0);
}

PALETTE_INIT(darkmist)
{
	int i;

	/* black color */
	palette_set_color(machine, 0x100, MAKE_RGB(0,0,0));

	/* color lookup tables */

	for (i = 0;i < 256;i++)
	{
		if (*color_prom & 0x40)
			*(colortable++) = 0x100;
		else
			*(colortable++) = (*color_prom & 0x3f) + 0x80;
		color_prom++;
	}

	for (i = 0;i < 256;i++)
	{
		if (*color_prom & 0x40)
			*(colortable++) = 0x100;
		else
			*(colortable++) = (*color_prom & 0x3f) + 0x00;
		color_prom++;
	}

	for (i = 0;i < 256;i++)
	{
		if (*color_prom & 0x40)
			*(colortable++) = 0x100;
		else
			*(colortable++) = (*color_prom & 0x3f) + 0x40;
		color_prom++;
	}

	for (i = 0;i < 256;i++)
	{
		if (*color_prom & 0x40)
			*(colortable++) = 0x100;
		else
			*(colortable++) = (*color_prom & 0x3f) + 0xc0;
		color_prom++;
	}
}

WRITE8_HANDLER(darkmist_palette_w)
{
	paletteram[offset]=data;
	offset&=0xff;
	palette_set_color_rgb(Machine, offset, pal4bit(paletteram[offset+0x200]), pal4bit(paletteram[offset] >> 4), pal4bit(paletteram[offset]));
}

READ8_HANDLER(darkmist_palette_r)
{
	return paletteram[offset];
}

WRITE8_HANDLER(darkmist_spritebank_w)
{
	spritebank=data<<8;
}

VIDEO_START(darkmist)
{
	bgtilemap = tilemap_create( get_bgtile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,512,64 );
	fgtilemap = tilemap_create( get_fgtile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,256 );
	txtilemap = tilemap_create( get_txttile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );
	tilemap_set_transparent_pen(fgtilemap, 0);
	tilemap_set_transparent_pen(txtilemap, 0);
}

VIDEO_UPDATE( darkmist)
{

#define DM_GETSCROLL(n) (((darkmist_scroll[(n)]<<1)&0xff) + ((darkmist_scroll[(n)]&0x80)?1:0) +( ((darkmist_scroll[(n)-1]<<4) | (darkmist_scroll[(n)-1]<<12) )&0xff00))

	tilemap_set_scrollx(bgtilemap, 0, DM_GETSCROLL(0x2));
	tilemap_set_scrolly(bgtilemap, 0, DM_GETSCROLL(0x6));
	tilemap_set_scrollx(fgtilemap, 0, DM_GETSCROLL(0xa));
	tilemap_set_scrolly(fgtilemap, 0, DM_GETSCROLL(0xe));

	fillbitmap(bitmap, get_black_pen(machine), cliprect);

	if(darkmist_hw & DISPLAY_BG)
	{
		tilemap_draw(bitmap,cliprect,bgtilemap, 0,0);
	}


	if(darkmist_hw & DISPLAY_FG)
	{
		tilemap_draw(bitmap,cliprect,fgtilemap, 0,0);
	}

	if(darkmist_hw & DISPLAY_SPR)
	{
/*
    Sprites

    76543210
0 - TTTT TTTT - tile
1 - xyBP PPP? - palette (P), flips (x,y), B - use spritebank,
                ? - unknown, according to gamecode top bit of one of coords(y/x)
2 - YYYY YYYY - y coord
3 - XXXX XXXX - x coord

*/
	int i,fx,fy,tile,palette;
	for(i=0;i<spriteram_size;i+=32)
	{
		fy=spriteram[i+1]&0x40;
		fx=spriteram[i+1]&0x80;

		tile=spriteram[i+0];

		if(spriteram[i+1]&0x20)
		{
			tile+=spritebank;
		}

		palette=((spriteram[i+1])>>1)&0xf;

		if(spriteram[i+1]&0x1)
			palette=mame_rand(machine)&15;

		palette+=32;

		drawgfx(
               bitmap,machine->gfx[2],
               tile,
               palette,
               fx,fy,
               spriteram[i+3],spriteram[i+2],
               cliprect,
               TRANSPARENCY_PEN,0 );
		}

	}

	if(darkmist_hw & DISPLAY_TXT)
	{
		tilemap_mark_all_tiles_dirty(txtilemap);
		tilemap_draw(bitmap,cliprect,txtilemap, 0,0);
	}


	return 0;
}


