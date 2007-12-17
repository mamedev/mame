/***************************************************************************

  video.c

***************************************************************************/

#include "driver.h"
#include <math.h>

UINT16 *nemesis_videoram1b;
UINT16 *nemesis_videoram2b;
UINT16 *nemesis_videoram1f;
UINT16 *nemesis_videoram2f;

UINT16 *nemesis_characterram;
size_t nemesis_characterram_size;
UINT16 *nemesis_xscroll1,*nemesis_xscroll2,*nemesis_yscroll;
UINT16 *nemesis_yscroll1,*nemesis_yscroll2;

static int spriteram_words;
static int tilemap_flip;
static int flipscreen;

static tilemap *background, *foreground;

static UINT8 *blank_characterdata; /* pseudo character */

/* gfxram dirty flags */

/* we should be able to draw sprite gfx directly without caching to GfxElements */

static UINT8 *sprite_dirty[8];

static const struct
{
	UINT8 width;
	UINT8 height;
	UINT8 char_type;
}
sprite_data[8] =
{
	{ 32, 32, 4 }, { 16, 32, 5 }, { 32, 16, 2 }, { 64, 64, 7 },
	{  8,  8, 0 }, { 16,  8, 6 }, {  8, 16, 3 }, { 16, 16, 1 }
};

static TILE_GET_INFO( get_bg_tile_info )
{
	int code,color,flags;
	code = nemesis_videoram1f[tile_index];
	color = nemesis_videoram2f[tile_index];
	flags = 0;
	if ( color & 0x80)  flags |= TILE_FLIPX;
	if ( code & 0x0800) flags |= TILE_FLIPY;
	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;
	if (code & 0xf800) {
		SET_TILE_INFO( 0, code&0x7ff, color&0x7f, flags );
	} else {
		SET_TILE_INFO( 0, 0x800, 0x00, 0 );
	}
	tileinfo->category = (code & 0x1000)>>12;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code,color,flags;
	code = nemesis_videoram1b[tile_index];
	color = nemesis_videoram2b[tile_index];
	flags = 0;
	if ( color & 0x80)  flags |= TILE_FLIPX;
	if ( code & 0x0800) flags |= TILE_FLIPY;
	if ((~code & 0x2000) || ((code & 0xc000) == 0x4000))
		 flags |= TILE_FORCE_LAYER0;
	if (code & 0xf800) {
		SET_TILE_INFO( 0, code&0x7ff, color&0x7f, flags );
	} else {
		SET_TILE_INFO( 0, 0x800, 0x00, 0 );
	}
	tileinfo->category = (code & 0x1000)>>12;
}

WRITE16_HANDLER( nemesis_gfx_flipx_w )
{
	if (ACCESSING_LSB)
	{
		flipscreen = data & 0x01;
		if (flipscreen)
			tilemap_flip |= TILEMAP_FLIPX;
		else
			tilemap_flip &= ~TILEMAP_FLIPX;

		tilemap_set_flip(ALL_TILEMAPS, tilemap_flip);
	}
	else
	{
		if (data & 0x100)
		{
			cpunum_set_input_line_and_vector(1, 0, HOLD_LINE, 0xff);
		}
	}
}

WRITE16_HANDLER( nemesis_gfx_flipy_w )
{
	if (ACCESSING_LSB)
	{
		if (data & 0x01)
			tilemap_flip |= TILEMAP_FLIPY;
		else
			tilemap_flip &= ~TILEMAP_FLIPY;

		tilemap_set_flip(ALL_TILEMAPS, tilemap_flip);
	}
}

WRITE16_HANDLER( nemesis_palette_word_w )
{
	int r,g,b,bit1,bit2,bit3,bit4,bit5;

	COMBINE_DATA(paletteram16 + offset);
	data = paletteram16[offset];

	/* Mish, 30/11/99 - Schematics show the resistor values are:
        300 Ohms
        620 Ohms
        1200 Ohms
        2400 Ohms
        4700 Ohms

        So the correct weights per bit are 8, 17, 33, 67, 130
    */

	#define MULTIPLIER 8 * bit1 + 17 * bit2 + 33 * bit3 + 67 * bit4 + 130 * bit5

	bit1=(data >>  0)&1;
	bit2=(data >>  1)&1;
	bit3=(data >>  2)&1;
	bit4=(data >>  3)&1;
	bit5=(data >>  4)&1;
	r = MULTIPLIER;
	bit1=(data >>  5)&1;
	bit2=(data >>  6)&1;
	bit3=(data >>  7)&1;
	bit4=(data >>  8)&1;
	bit5=(data >>  9)&1;
	g = MULTIPLIER;
	bit1=(data >>  10)&1;
	bit2=(data >>  11)&1;
	bit3=(data >>  12)&1;
	bit4=(data >>  13)&1;
	bit5=(data >>  14)&1;
	b = MULTIPLIER;

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( salamander_palette_word_w )
{
	COMBINE_DATA(paletteram16 + offset);
	offset &= ~1;

	data = ((paletteram16[offset] << 8) & 0xff00) | (paletteram16[offset+1] & 0xff);
	palette_set_color_rgb(Machine,offset / 2,pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
}

WRITE16_HANDLER( nemesis_videoram1b_word_w )
{
	COMBINE_DATA(nemesis_videoram1b + offset);
	tilemap_mark_tile_dirty( foreground,offset );
}
WRITE16_HANDLER( nemesis_videoram1f_word_w )
{
	COMBINE_DATA(nemesis_videoram1f + offset);
	tilemap_mark_tile_dirty( background,offset );
}

WRITE16_HANDLER( nemesis_videoram2b_word_w )
{
	COMBINE_DATA(nemesis_videoram2b + offset);
	tilemap_mark_tile_dirty( foreground,offset );
}
WRITE16_HANDLER( nemesis_videoram2f_word_w )
{
	COMBINE_DATA(nemesis_videoram2f + offset);
	tilemap_mark_tile_dirty( background,offset );
}


/* we have to straighten out the 16-bit word into bytes for gfxdecode() to work */
WRITE16_HANDLER( nemesis_characterram_word_w )
{
	UINT16 oldword = nemesis_characterram[offset];
	COMBINE_DATA(nemesis_characterram + offset);
	data = nemesis_characterram[offset];

	if (oldword != data)
	{
		int i;
		for (i=0; i<8; i++)
		{
			int w = sprite_data[i].width;
			int h = sprite_data[i].height;
			sprite_dirty[i][offset * 4 / (w * h)] = 1;
		}
	}
}


/* claim a palette dirty array */
VIDEO_START( nemesis )
{
	int i;
	spriteram_words = spriteram_size / 2;

	background = tilemap_create(
		get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32 );

	foreground = tilemap_create(
		get_fg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32 );

	tilemap_set_transparent_pen( background, 0 );
	tilemap_set_transparent_pen( foreground, 0 );
	tilemap_set_scroll_rows( background, 256 );
	tilemap_set_scroll_rows( foreground, 256 );

	for (i=0; i<8; i++)
	{
		int w = sprite_data[i].width;
		int h = sprite_data[i].height;
		int size = 0x20000 / (w * h);
		sprite_dirty[i] = auto_malloc(size);
		memset(sprite_dirty[i], 1, size);
	}

	memset(nemesis_characterram,0,nemesis_characterram_size);


	blank_characterdata = auto_malloc(32*8/8*(2048+1));
	memset(blank_characterdata,0x00,32*8/8*(2048+1));
	decodechar(machine->gfx[0],0x800,(UINT8 *)blank_characterdata,
					machine->drv->gfxdecodeinfo[0].gfxlayout);

	flipscreen = 0;
	tilemap_flip = 0;
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	/*
     *  16 bytes per sprite, in memory from 56000-56fff
     *
     *  byte    0 : relative priority.
     *  byte    2 : size (?) value #E0 means not used., bit 0x01 is flipx
                    0xc0 is upper 2 bits of zoom.
                    0x38 is size.
     *  byte    4 : zoom = 0xff
     *  byte    6 : low bits sprite code.
     *  byte    8 : color + hi bits sprite code., bit 0x20 is flipy bit. bit 0x01 is high bit of X pos.
     *  byte    A : X position.
     *  byte    C : Y position.
     *  byte    E : not used.
     */

	int adress;	/* start of sprite in spriteram */
	int sx;	/* sprite X-pos */
	int sy;	/* sprite Y-pos */
	int code;	/* start of sprite in obj RAM */
	int color;	/* color of the sprite */
	int flipx,flipy;
	int zoom;
	int char_type;
	int priority;
	int size;
	int w,h;
	int idx;

	for (priority=256-1; priority>=0; priority--)
	{
		for (adress = spriteram_words-8; adress >= 0; adress -= 8)
		{
			if((spriteram16[adress] & 0xff)!=priority) continue;

			zoom = spriteram16[adress+2] & 0xff;
			if (!(spriteram16[adress+2] & 0xff00) && ((spriteram16[adress+3] & 0xff00) != 0xff00))
				code = spriteram16[adress+3] + ((spriteram16[adress+4] & 0xc0) << 2);
			else
				code = (spriteram16[adress+3] & 0xff) + ((spriteram16[adress+4] & 0xc0) << 2);

			if (zoom != 0xFF || code!=0)
			{
				size=spriteram16[adress+1];
				zoom+=(size&0xc0)<<2;

				sx = spriteram16[adress+5]&0xff;
				sy = spriteram16[adress+6]&0xff;
				if(spriteram16[adress+4]&1)
					sx-=0x100;	/* fixes left side clip */
				color = (spriteram16[adress+4] & 0x1e) >> 1;
				flipx = spriteram16[adress+1] & 0x01;
				flipy = spriteram16[adress+4] & 0x20;

				idx = (size >> 3) & 7;
				w = sprite_data[idx].width;
				h = sprite_data[idx].height;
				code = code * 8 * 16 / (w * h);
				char_type = sprite_data[idx].char_type;

				if( zoom )
				{
					zoom = ((1<<16)*0x80/zoom) + 0x02ab;
					if (flipscreen)
					{
						sx = 256 - ((zoom * w) >> 16) - sx;
						sy = 256 - ((zoom * h) >> 16) - sy;
						flipx = !flipx;
						flipy = !flipy;
					}
					pdrawgfxzoom(bitmap,machine->gfx[char_type],
						code,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,TRANSPARENCY_PEN,0,
						zoom,zoom,
						0xfff0 );
				}
			} /* if sprite */
		} /* for loop */
	} /* priority */
}

/******************************************************************************/

static void update_gfx(running_machine *machine)
{
	int offs,code;
	int bAnyDirty;

	bAnyDirty = 0;
	for (offs = spriteram_words - 1;offs >= 0;offs --)
	{
		if (sprite_dirty[4][offs])
		{
			decodechar(machine->gfx[0],offs,(UINT8 *)nemesis_characterram,
					machine->drv->gfxdecodeinfo[0].gfxlayout);
			bAnyDirty = 1;
			sprite_dirty[4][offs] = 0;
		}
	}
	if( bAnyDirty )
	{
		tilemap_mark_all_tiles_dirty( background );
		tilemap_mark_all_tiles_dirty( foreground );
	}

	for (offs = 0;offs < spriteram_words;offs += 8)
	{
		int char_type;
		int zoom;

		zoom = spriteram16[offs+2] & 0xff;
		if (!(spriteram16[offs+2] & 0xff00) && ((spriteram16[offs+3] & 0xff00) != 0xff00))
			code = spriteram16[offs+3] + ((spriteram16[offs+4] & 0xc0) << 2);
		else
			code = (spriteram16[offs+3] & 0xff) + ((spriteram16[offs+4] & 0xc0) << 2);

		if (zoom != 0xFF || code!=0)
		{
			int size = spriteram16[offs+1];
			int idx = (size >> 3) & 7;
			int w, h;

			w = sprite_data[idx].width;
			h = sprite_data[idx].height;
			code = code * 8 * 16 / (w * h);
			char_type = sprite_data[idx].char_type;
			if (sprite_dirty[idx][code] == 1)
			{
				decodechar(machine->gfx[char_type],code,(UINT8 *)nemesis_characterram,
					machine->drv->gfxdecodeinfo[char_type].gfxlayout);
				sprite_dirty[idx][code] = 0;
			}
		}
	}
}

/******************************************************************************/

VIDEO_UPDATE( nemesis )
{
	int offs;

	update_gfx(machine);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);

	tilemap_set_scrolly( background, 0, (nemesis_yscroll[0x180] & 0xff) );

	for (offs = 0;offs < 256;offs++)
	{
		tilemap_set_scrollx( background, offs,
			(nemesis_xscroll2[offs] & 0xff) + ((nemesis_xscroll2[0x100 + offs] & 0x01) << 8) - (flipscreen ? 0x107 : 0) );
		tilemap_set_scrollx( foreground, offs,
			(nemesis_xscroll1[offs] & 0xff) + ((nemesis_xscroll1[0x100 + offs] & 0x01) << 8) - (flipscreen ? 0x107 : 0) );
	}

	tilemap_draw(bitmap,cliprect,background,0,1);
	tilemap_draw(bitmap,cliprect,foreground,0,2);
	tilemap_draw(bitmap,cliprect,background,1,4);
	tilemap_draw(bitmap,cliprect,foreground,1,8);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( salamand )
{
	int offs;
	rectangle clip;

	update_gfx(machine);

	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[0],cliprect);

	clip.min_x = 0;
	clip.max_x = 255;

	tilemap_set_scroll_cols( background, 64 );
	tilemap_set_scroll_cols( foreground, 64 );
	tilemap_set_scroll_rows( background, 1 );
	tilemap_set_scroll_rows( foreground, 1 );

	for (offs = 0; offs < 64; offs++)
	{
		tilemap_set_scrolly( background, offs, nemesis_yscroll1[offs] );
		tilemap_set_scrolly( foreground, offs, nemesis_yscroll2[offs] );
	}

	for (offs = cliprect->min_y; offs <= cliprect->max_y; offs++)
	{
		clip.min_y = offs;
		clip.max_y = offs;

		tilemap_set_scrollx( background, 0,
			((nemesis_xscroll2[offs] & 0xff) + ((nemesis_xscroll2[0x100 + offs] & 1) << 8)) );
		tilemap_set_scrollx( foreground, 0,
			((nemesis_xscroll1[offs] & 0xff) + ((nemesis_xscroll1[0x100 + offs] & 1) << 8)) );

		tilemap_draw(bitmap,&clip,foreground,0,1);
		tilemap_draw(bitmap,&clip,background,0,2);
		tilemap_draw(bitmap,&clip,foreground,1,4);
		tilemap_draw(bitmap,&clip,background,1,8);
	}

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
