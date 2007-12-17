#include "driver.h"


UINT8 *mnchmobl_vreg;
UINT8 *mnchmobl_status_vram;
UINT8 *mnchmobl_sprite_xpos;
UINT8 *mnchmobl_sprite_attr;
UINT8 *mnchmobl_sprite_tile;

static int mnchmobl_palette_bank;
static int flipscreen;

PALETTE_INIT( mnchmobl )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( mnchmobl_palette_bank_w )
{
	if( mnchmobl_palette_bank!=(data&0x3) )
	{
		memset( dirtybuffer, 1, 0x100 );
		mnchmobl_palette_bank = data&0x3;
	}
}

WRITE8_HANDLER( mnchmobl_flipscreen_w )
{
	if( flipscreen!=data )
	{
		memset( dirtybuffer, 1, 0x100 );
		flipscreen = data;
	}
}


READ8_HANDLER( mnchmobl_sprite_xpos_r ){ return mnchmobl_sprite_xpos[offset]; }
WRITE8_HANDLER( mnchmobl_sprite_xpos_w ){ mnchmobl_sprite_xpos[offset] = data; }

READ8_HANDLER( mnchmobl_sprite_attr_r ){ return mnchmobl_sprite_attr[offset]; }
WRITE8_HANDLER( mnchmobl_sprite_attr_w ){ mnchmobl_sprite_attr[offset] = data; }

READ8_HANDLER( mnchmobl_sprite_tile_r ){ return mnchmobl_sprite_tile[offset]; }
WRITE8_HANDLER( mnchmobl_sprite_tile_w ){ mnchmobl_sprite_tile[offset] = data; }

VIDEO_START( mnchmobl )
{
	dirtybuffer = auto_malloc(0x100);
	memset( dirtybuffer, 1, 0x100 );
	tmpbitmap = auto_bitmap_alloc(512,512,machine->screen[0].format);
}

READ8_HANDLER( mnchmobl_videoram_r )
{
	return videoram[offset];
}

WRITE8_HANDLER( mnchmobl_videoram_w )
{
	offset = offset&0xff; /* mirror the two banks? */
	if( videoram[offset]!=data )
	{
		videoram[offset] = data;
		dirtybuffer[offset] = 1;
	}
}

static void draw_status(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[0];
	int row;

	for( row=0; row<4; row++ )
	{
		int sy,sx = (row&1)*8;
		const UINT8 *source = mnchmobl_status_vram + (~row&1)*32;
		if( row<=1 )
		{
			source+=2*32;
			sx+=256+32+16;
		}
		for( sy=0; sy<256; sy+=8 )
		{
			drawgfx( bitmap, gfx,
				*source++,
				0, /* color */
				0,0, /* no flip */
				sx,sy,
				cliprect,
				TRANSPARENCY_NONE, 0 );
		}
	}
}

static void draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
/*
    ROM B1.2C contains 256 tilemaps defining 4x4 configurations of
    the tiles in ROM B2.2B
*/
	UINT8 *rom = memory_region(REGION_GFX2);
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	for( offs=0; offs<0x100; offs++ )
	{
		if( dirtybuffer[offs] )
		{
			int sy = (offs%16)*32;
			int sx = (offs/16)*32;
			int tile_number = videoram[offs];
			int row,col;
			dirtybuffer[offs] = 0;
			for( row=0; row<4; row++ )
			{
				for( col=0; col<4; col++ )
				{
					drawgfx( tmpbitmap,gfx,
						rom[col+tile_number*4+row*0x400],
						mnchmobl_palette_bank,
						0,0, /* flip */
						sx+col*8, sy+row*8,
						0, TRANSPARENCY_NONE, 0 );
				}
			}
		}
	}

	{
		int scrollx = -(mnchmobl_vreg[6]*2+(mnchmobl_vreg[7]>>7))-64-128-16;
		int scrolly = 0;

		copyscrollbitmap(bitmap,tmpbitmap,
			1,&scrollx,1,&scrolly,
			cliprect,TRANSPARENCY_NONE,0);
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int scroll = mnchmobl_vreg[6];
	int flags = mnchmobl_vreg[7];					/*   XB?????? */
	int xadjust = - 128-16 - ((flags&0x80)?1:0);
	int bank = (flags&0x40)?1:0;
	const gfx_element *gfx = machine->gfx[2+bank];
	int color_base = mnchmobl_palette_bank*4+3;
	int i;
	for( i=0; i<0x200; i++ )
	{
		int tile_number = mnchmobl_sprite_tile[i];	/*   ETTTTTTT */
		int attributes = mnchmobl_sprite_attr[i];	/*   XYYYYYCC */
		int sx = mnchmobl_sprite_xpos[i];			/*   XXXXXXX? */
		int sy = (i/0x40)*0x20;						/* Y YY------ */
		sy += (attributes>>2)&0x1f;
		if( tile_number != 0xff && (attributes&0x80) )
		{
			sx = (sx>>1) | (tile_number&0x80);
			sx = 2*((-32-scroll - sx)&0xff)+xadjust;
			drawgfx( bitmap, gfx,
				0x7f - (tile_number&0x7f),
				color_base-(attributes&0x03),
				0,0, /* no flip */
				sx,sy,
				cliprect, TRANSPARENCY_PEN, 7 );
		}
	}
}

VIDEO_UPDATE( mnchmobl )
{
	draw_background(machine, bitmap, cliprect);
	draw_sprites(machine, bitmap, cliprect);
	draw_status(machine, bitmap, cliprect);
	return 0;
}
