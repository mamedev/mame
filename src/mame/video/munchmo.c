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

	for (i = 0;i < machine->config->total_colors;i++)
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
	mnchmobl_palette_bank = data&0x3;
}

WRITE8_HANDLER( mnchmobl_flipscreen_w )
{
	flipscreen = data;
}


VIDEO_START( mnchmobl )
{
	machine->generic.tmpbitmap = auto_bitmap_alloc(machine,512,512,video_screen_get_format(machine->primary_screen));
}

static void draw_status(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
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
			drawgfx_opaque( bitmap, cliprect,
				gfx,
				*source++,
				0, /* color */
				0,0, /* no flip */
				sx,sy );
		}
	}
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
/*
    ROM B1.2C contains 256 tilemaps defining 4x4 configurations of
    the tiles in ROM B2.2B
*/
	UINT8 *rom = memory_region(machine, "gfx2");
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	for( offs=0; offs<0x100; offs++ )
	{
		int sy = (offs%16)*32;
		int sx = (offs/16)*32;
		int tile_number = machine->generic.videoram.u8[offs];
		int row,col;

		for( row=0; row<4; row++ )
		{
			for( col=0; col<4; col++ )
			{
				drawgfx_opaque( machine->generic.tmpbitmap,0, gfx,
					rom[col+tile_number*4+row*0x400],
					mnchmobl_palette_bank,
					0,0, /* flip */
					sx+col*8, sy+row*8 );
			}
		}
	}

	{
		int scrollx = -(mnchmobl_vreg[6]*2+(mnchmobl_vreg[7]>>7))-64-128-16;
		int scrolly = 0;

		copyscrollbitmap(bitmap,machine->generic.tmpbitmap, 1,&scrollx,1,&scrolly, cliprect);
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int scroll = mnchmobl_vreg[6];
	int flags = mnchmobl_vreg[7];							/*   XB?????? */
	int xadjust = - 128 - 16 - ((flags & 0x80) ? 1 : 0);
	int bank = (flags & 0x40) ? 1 : 0;
	const gfx_element *gfx = machine->gfx[2 + bank];
	int color_base = mnchmobl_palette_bank * 4 + 3;
	int i, j;
	int firstsprite = mnchmobl_vreg[4] & 0x3f;
	for( i = firstsprite; i < firstsprite + 0x40; i++ )
	{
		for( j = 0; j < 8; j++ )
		{
			int offs = (j << 6) | ( i & 0x3f );
			int tile_number = mnchmobl_sprite_tile[offs];		/*   ETTTTTTT */
			int attributes = mnchmobl_sprite_attr[offs];		/*   XYYYYYCC */
			int sx = mnchmobl_sprite_xpos[offs];			/*   XXXXXXX? */
			int sy = (offs >> 6) << 5;					/* Y YY------ */
			sy += (attributes >> 2) & 0x1f;
			if( attributes & 0x80 )
			{
				sx = (sx >> 1) | (tile_number & 0x80);
				sx = 2 * ((- 32 - scroll - sx) & 0xff) + xadjust;
				drawgfx_transpen( bitmap, cliprect, gfx,
					0x7f - (tile_number & 0x7f),
					color_base - (attributes & 0x03),
					0,0,							/* no flip */
					sx,sy, 7 );
			}
		}
	}
}

VIDEO_UPDATE( mnchmobl )
{
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	draw_status(screen->machine, bitmap, cliprect);
	return 0;
}
