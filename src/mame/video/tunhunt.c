/*************************************************************************

    Atari Tunnel Hunt hardware

*************************************************************************/

#include "driver.h"

extern UINT8 tunhunt_control;
extern UINT8 *tunhunt_ram;

/****************************************************************************************/

/* Video Hardware Addresses */

/* Square Generator (13 bytes each) */
#define LINEV	0x1403	// LINES VERTICAL START
#define LINEVS	0x1483	// LINES VERT STOP
#define LINEH	0x1083	// LINES HORIZ START
#define LINEC	0x1283	// LINE COLOR, 4 BITS D0-D3
#define LINESH	0x1203	// LINES SLOPE 4 BITS D0-D3 (signed)
/* LINESH was used for rotation effects in an older version of the game */

/* Shell Object0 */
#define SHEL0H	0x1800	// SHELL H POSITON (NORMAL SCREEN)
#define SHL0V	0x1400	// SHELL V START(NORMAL SCREEN)
#define SHL0VS	0x1480	// SHELL V STOP (NORMAL SCREEN)
#define SHL0ST	0x1200	// SHELL VSTRETCH (LIKE MST OBJ STRECTH)
#define SHL0PC	0x1280	// SHELL PICTURE CODE (D3-D0)

/* Shell Object1 (see above) */
#define SHEL1H	0x1A00
#define SHL1V	0x1401
#define SHL1VS	0x1481
#define SHL1ST	0x1201
#define SHL1PC	0x1281

/* Motion Object RAM */
#define MOBJV	0x1C00	// V POSITION (SCREEN ON SIDE)
#define MOBVS	0x1482	// V STOP OF MOTION OBJECT (NORMAL SCREEN)
#define MOBJH	0x1402	// H POSITON (SCREEN ON SIDE) (VSTART - NORMAL SCREEN)
#define MOBST	0x1082	// STARTING LINE FOR RAM SCAN ON MOBJ
#define VSTRLO	0x1202	// VERT (SCREEN ON SIDE) STRETCH MOJ OBJ
#define MOTT	0x2C00	// MOTION OBJECT RAM (00-0F NOT USED, BYT CLEARED)
#define MOBSC0	0x1080	// SCAN ROM START FOR MOBJ (unused?)
#define MOBSC1	0x1081	// (unused?)

static tilemap *fg_tilemap;

/****************************************************************************************/

WRITE8_HANDLER( tunhunt_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tunhunt_mott_w )
{
	if( spriteram[offset]!=data )
	{
		spriteram[offset] = data;
		dirtybuffer[offset>>4] = 1;
	}
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = videoram[tile_index];
	int code = attr & 0x3f;
	int color = attr >> 6;
	int flags = color ? TILE_FORCE_LAYER0 : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( tunhunt )
{
	/*
    Motion Object RAM contains 64 lines of run-length encoded data.
    We keep track of dirty lines and cache the expanded bitmap.
    With max RLE expansion, bitmap size is 256x64.
    */
	dirtybuffer = auto_malloc(64);

	memset( dirtybuffer, 1, 64 );
	tmpbitmap = auto_bitmap_alloc( 256, 64, machine->screen[0].format );

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_cols,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scrollx(fg_tilemap, 0, 64);
}

PALETTE_INIT( tunhunt )
{
	/* Tunnel Hunt uses a combination of color proms and palette RAM to specify a 16 color
     * palette.  Here, we manage only the mappings for alphanumeric characters and SHELL
     * graphics, which are unpacked ahead of time and drawn using MAME's drawgfx primitives.
     */

	/* AlphaNumerics (1bpp)
     *  2 bits of hilite select from 4 different background colors
     *  Foreground color is always pen#4
     *  Background color is mapped as follows:
     */

	/* alpha hilite#0 */
	colortable[0] = 0x0; /* background color#0 (transparent) */
	colortable[1] = 0x4; /* foreground color */

	/* alpha hilite#1 */
	colortable[2] = 0x5; /* background color#1 */
	colortable[3] = 0x4; /* foreground color */

	/* alpha hilite#2 */
	colortable[4] = 0x6; /* background color#2 */
	colortable[5] = 0x4; /* foreground color */

	/* alpha hilite#3 */
	colortable[6] = 0xf; /* background color#3 */
	colortable[7] = 0x4; /* foreground color */

	/* shell graphics; these are either 1bpp (2 banks) or 2bpp.  It isn't clear which.
     * In any event, the following pens are associated with the shell graphics:
     */
	colortable[0x8] = 0;
	colortable[0x9] = 4;//1;
	colortable[0xa] = 2;
	colortable[0xb] = 4;
}

/*
Color Array Ram Assignments:
    Location
        0               Blanking, border
        1               Mot Obj (10) (D), Shell (01)
        2               Mot Obj (01) (G), Shell (10)
        3               Mot Obj (00) (W)
        4               Alpha & Shell (11) - shields
        5               Hilight 1
        6               Hilight 2
        8-E             Lines (as normal) background
        F               Hilight 3
*/
static void update_palette(running_machine *machine)
{
//  const UINT8 *color_prom = memory_region( REGION_PROMS );
/*
    The actual contents of the color proms (unused by this driver)
    are as follows:

    D11 "blue/green"
    0000:   00 00 8b 0b fb 0f ff 0b
            00 00 0f 0f fb f0 f0 ff

    C11 "red"
    0020:   00 f0 f0 f0 b0 b0 00 f0
            00 f0 f0 00 b0 00 f0 f0
*/
	int color;
	int shade;
	int i;
	int red,green,blue;

	for( i=0; i<16; i++ )
	{
		color = paletteram[i];
		shade = 0xf^(color>>4);

		color &= 0xf; /* hue select */
		switch( color )
		{
		default:
		case 0x0: red = 0xff; green = 0xff; blue = 0xff; break; /* white */
		case 0x1: red = 0xff; green = 0x00; blue = 0xff; break; /* purple */
		case 0x2: red = 0x00; green = 0x00; blue = 0xff; break; /* blue */
		case 0x3: red = 0x00; green = 0xff; blue = 0xff; break; /* cyan */
		case 0x4: red = 0x00; green = 0xff; blue = 0x00; break; /* green */
		case 0x5: red = 0xff; green = 0xff; blue = 0x00; break; /* yellow */
		case 0x6: red = 0xff; green = 0x00; blue = 0x00; break; /* red */
		case 0x7: red = 0x00; green = 0x00; blue = 0x00; break; /* black? */

		case 0x8: red = 0xff; green = 0x7f; blue = 0x00; break; /* orange */
		case 0x9: red = 0x7f; green = 0xff; blue = 0x00; break; /* ? */
		case 0xa: red = 0x00; green = 0xff; blue = 0x7f; break; /* ? */
		case 0xb: red = 0x00; green = 0x7f; blue = 0xff; break; /* ? */
		case 0xc: red = 0xff; green = 0x00; blue = 0x7f; break; /* ? */
		case 0xd: red = 0x7f; green = 0x00; blue = 0xff; break; /* ? */
		case 0xe: red = 0xff; green = 0xaa; blue = 0xaa; break; /* ? */
		case 0xf: red = 0xaa; green = 0xaa; blue = 0xff; break; /* ? */
		}

	/* combine color components with shade value (0..0xf) */
		#define APPLY_SHADE( C,S ) ((C*S)/0xf)
		red		= APPLY_SHADE(red,shade);
		green	= APPLY_SHADE(green,shade);
		blue	= APPLY_SHADE(blue,shade);

		palette_set_color( machine,i,MAKE_RGB(red,green,blue) );
	}
}

static void draw_motion_object(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
/*
 *      VSTRLO  0x1202
 *          normally 0x02 (gameplay, attract1)
 *          in attract2 (with "Tunnel Hunt" graphic), decrements from 0x2f down to 0x01
 *          goes to 0x01 for some enemy shots
 *
 *      MOBSC0  0x1080
 *      MOBSC1  0x1081
 *          always 0x00?
 */
//  int skip = tunhunt_ram[MOBST];
	int x0 = 255-tunhunt_ram[MOBJV];
	int y0 = 255-tunhunt_ram[MOBJH];
	int scalex,scaley;
	int line,span;
	int x,span_data;
	int color;
	int count;
	const UINT8 *source;

	for( line=0; line<64; line++ )
	{
		if( dirtybuffer[line] )
		{
			dirtybuffer[line] = 0;
			x = 0;
			source = &spriteram[line*0x10];
			for( span=0; span<0x10; span++ )
			{
				span_data = source[span];
				if( span_data == 0xff ) break;
				color = ((span_data>>6)&0x3)^0x3;
				count = (span_data&0x1f)+1;
				while( count-- && x < 256 )
				{
					*BITMAP_ADDR16(tmpbitmap, line, x++) = machine->pens[color];
				}
			}
			while( x<256 )
			{
				*BITMAP_ADDR16(tmpbitmap, line, x++) = machine->pens[0];
			}
		} /* dirty line */
	} /* next line */

	switch( tunhunt_ram[VSTRLO] )
	{
	case 0x01:
		scaley = (1<<16)*0.33; /* seems correct */
		break;

	case 0x02:
		scaley = (1<<16)*0.50; /* seems correct */
		break;

	default:
		scaley = (1<<16)*tunhunt_ram[VSTRLO]/4; /* ??? */
		break;
	}
	scalex = (1<<16);

	copyrozbitmap(
		bitmap,tmpbitmap,
		-x0*scalex,/* startx */
		-y0*scaley,/* starty */
		scalex,/* incxx */
		0,0,/* incxy,incyx */
		scaley,/* incyy */
		0, /* no wraparound */
		cliprect,
		TRANSPARENCY_PEN,machine->pens[0],
		0 /* priority */
	);
}

static void draw_box(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
/*
    This is unnecessarily slow, but the box priorities aren't completely understood,
    yet.  Once understood, this function should be converted to use fillbitmap with
    rectangular chunks instead of BITMAP_ADDR.

    Tunnels:
        1080: 00 00 00      01  e7 18   ae 51   94 6b   88 77   83 7c   80 7f   x0
        1480: 00 f0 17      00  22 22   5b 5b   75 75   81 81   86 86   89 89   y0
        1400: 00 00 97      ff  f1 f1   b8 b8   9e 9e   92 92   8d 8d   8a 8a   y1
        1280: 07 03 00      07  07 0c   0c 0d   0d 0e   0e 08   08 09   09 0a   palette select

    Color Bars:
        1080: 00 00 00      01  00 20 40 60 80 a0 c0 e0     01 2a   50 7a       x0
        1480: 00 f0 00      00  40 40 40 40 40 40 40 40     00 00   00 00       y0
        1400: 00 00 00      ff  ff ff ff ff ff ff ff ff     40 40   40 40       y1
        1280: 07 03 00      01  07 06 04 05 02 07 03 00     09 0a   0b 0c       palette select
        ->hue 06 02 ff      60  06 05 03 04 01 06 02 ff     d2 00   c2 ff
*/
	int span,x,y;
	int color;
//  rectangle bbox;
	int z;
	int x0,y0,y1;

	for( y=0; y<256; y++ )
	{
		if (0xff-y >= cliprect->min_y && 0xff-y <= cliprect->max_y)
			for( x=0; x<256; x++ )
			{
				color = 0;
				z = 0;
				for( span=3; span<16; span++ )
				{
					x0 = tunhunt_ram[span+0x1080];
					y0 = tunhunt_ram[span+0x1480];
					y1 = tunhunt_ram[span+0x1400];

					if( y>=y0 && y<=y1 && x>=x0 && x0>=z )
					{
						color = tunhunt_ram[span+0x1280]&0xf;
						z = x0; /* give priority to rightmost spans */
					}
				}
				if (x >= cliprect->min_x && x <= cliprect->max_x)
					*BITMAP_ADDR16(bitmap, 0xff-y, x) = machine->pens[color];
			}
	}
}

/* "shell" graphics are 16x16 pixel tiles used for player shots and targeting cursor */
static void draw_shell(running_machine *machine,
		mame_bitmap *bitmap,
		const rectangle *cliprect,
		int picture_code,
		int hposition,
		int vstart,
		int vstop,
		int vstretch,
		int hstretch )
{
	if( hstretch )
	{
		int sx,sy;
		for( sx=0; sx<256; sx+=16 )
		{
			for( sy=0; sy<256; sy+=16 )
			{
				drawgfx( bitmap, machine->gfx[1],
					picture_code,
					0, /* color */
					0,0, /* flip */
					sx,sy,
					cliprect,
					TRANSPARENCY_PEN,0 );
			}
		}
	}
	else
	/*
        vstretch is normally 0x01

        targeting cursor:
            hposition   = 0x78
            vstart      = 0x90
            vstop       = 0x80

        during grid test:
            vstretch    = 0xff
            hposition   = 0xff
            vstart      = 0xff
            vstop       = 0x00

    */
	drawgfx( bitmap, machine->gfx[1],
			picture_code,
			0, /* color */
			0,0, /* flip */
			255-hposition-16,vstart-32,
			cliprect,
			TRANSPARENCY_PEN,0 );
}

VIDEO_UPDATE( tunhunt )
{
	update_palette(machine);

	draw_box(machine, bitmap, cliprect);

	draw_motion_object(machine, bitmap, cliprect);

	draw_shell(machine, bitmap, cliprect,
		tunhunt_ram[SHL0PC],	/* picture code */
		tunhunt_ram[SHEL0H],	/* hposition */
		tunhunt_ram[SHL0V],	/* vstart */
		tunhunt_ram[SHL0VS],	/* vstop */
		tunhunt_ram[SHL0ST],	/* vstretch */
		tunhunt_control&0x08 ); /* hstretch */

	draw_shell(machine, bitmap, cliprect,
		tunhunt_ram[SHL1PC],	/* picture code */
		tunhunt_ram[SHEL1H],	/* hposition */
		tunhunt_ram[SHL1V],	/* vstart */
		tunhunt_ram[SHL1VS],	/* vstop */
		tunhunt_ram[SHL1ST],	/* vstretch */
		tunhunt_control&0x10 ); /* hstretch */

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
