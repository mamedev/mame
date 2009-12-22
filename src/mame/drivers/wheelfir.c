/* Wheels & Fire */

/* driver by
 David Haywood
 Tomasz Slanina
*/

/*

Produttore  TCH
N.revisione E133



CPU

2x MC68HC000FN16

2x TPC1020BFN-084C
1x BT478KPJ50-615-9132F
1x S9530AG-ADC0808-CCV
1x oscillator 32.000000MHz
1x ST93C46 EEPROM

ROMs

12x TMS27C040

1x GAL16V8QS
2x GAL22V10

Note

1x JAMMA connector
1x VGA connector - Used to link 2 PCBs together
2x trimmer (volume)


----

uses a blitter for the gfx, this is not fully understood...

level 5 interrupt = raster interrupt, used for road
level 3 interrupt = vblank interrupt
level 1 interrupt = blitter interrupt

something is missing, currently needs a hack to boot

*/


#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"

static int toggle_bit;

static READ16_HANDLER( wheelfir_rand1 )
{
	return input_port_read(space->machine, "IN0") ^ toggle_bit;	// mame_rand(space->machine);
}

static READ16_HANDLER( wheelfir_rand2 )
{
	return input_port_read(space->machine, "IN1");		// mame_rand(space->machine);
}


static READ16_HANDLER( wheelfir_rand4 )
{
	return mame_rand(space->machine);
}

static UINT16 *wheelfir_myram;
static UINT16 wheelfir_blitdata[16];
static bitmap_t *wheelfir_tmp_bitmap[3];

/*

A 30f7 - 3150 - 0059 - 0d5c - 0066 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - A
B
C 48f7 - 4971 - 007a - 0d28 - 0032 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - C
C 48f7 - 4945 - 004e - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - C
D 54f7 - 559d - 00a6 - 0d76 - 0080 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - D
E 1ff8 - 209d - 00a5 - 2728 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - E
F
G 60f7 - 6145 - 004e - 0d5c - 0066 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - G
H 6cf7 - 6d7c - 0085 - 0d28 - 0032 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - H
I 3efd - 3f5e - 0061 - 2d28 - 0032 - 0000 - 0000 - 3400 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - I
I 3efd - 3f5e - 0061 - 2d42 - 004c - 0000 - 0000 - 3400 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - I
I 3efd - 3fa0 - 00a3 - 2d42 - 004c - 0000 - 0000 - 3400 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - I
J
K
L 40f8 - 415b - 0063 - 2776 - 0080 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - L
M 3cf6 - 3d5a - 0064 - eac4 - 00ce - 0000 - 0000 - 3b00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - M
N 84f7 - 8566 - 006f - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - N
N 84f7 - 85b3 - 00bc - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - N
O 90f7 - 9150 - 0059 - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - O
O 90f7 - 917c - 0085 - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - O
O 90f7 - 91a8 - 00b1 - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - O
P f0f7 - f187 - 0090 - 0d42 - 004c - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - P
Q
R fcf7 - fd71 - 007a - 0d76 - 0080 - 0000 - 0000 - 3c00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - R
S 4bf8 - 4c45 - 004d - 2728 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - S
S 4bf8 - 4ca8 - 00b0 - 2728 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - S
S 4bf8 - 4cbe - 00c6 - 2742 - 004c - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - S
T 08f7 - 0966 - 006f - 0d28 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - T
T 08f7 - 0992 - 009b - 0d28 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - T
T 08f7 - 09b3 - 00bc - 0d28 - 0032 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - T
T 08f7 - 0992 - 009b - 0d42 - 004c - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - T
U 14f7 - 155b - 0064 - 0d90 - 009a - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - U
V 20f7 - 21d4 - 00dd - 0d76 - 0080 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - V
W 20f3 - 214e - 005b - b628 - 0032 - 0000 - 0000 - 3a00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - W
X 2cf7 - 2d50 - 0059 - 0dde - 00e8 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - X
Y 38f7 - 3921 - 002a - 0d76 - 0080 - 0000 - 0000 - 3d00 - 00d7 - c000 - 000c - 0000 - 0000 - 0000 - 00ff - ffff - ffff - Y
Z
    extra test mode reads..

    ????
    GAME OPTIONS
    COIN OPTIONS
    COLOR AND CONVERGENCY
    SOUND
    STATISTICS
    COMUNICATION
    EXIT

U 14f7 - 155b - 0064 - 0d90 - 009a - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - U
V 20f7 - 21d4 - 00dd - 0d76 - 0080 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - V
X 2cf7 - 2d50 - 0059 - 0dde - 00e8 - 0000 - 0000 - 3d00 - 0097 - c000 - 0008 - 0000 - 0000 - 0000 - 00ff - ffff - ffff - X
Y 38f7 - 3921 - 002a - 0d76 - 0080 - 0000 - 0000 - 3d00 - 00d7 - c000 - 000c - 0000 - 0000 - 0000 - 00ff - ffff - ffff - Y

by the grouping of the characters the test mode font should clearly be the one seen at the start of the last page. and
scattered over the previous page.  letters are 12x12, and U/V/X/Y are +0x0c00 from each other in this data.


by using 0xffff at 0xf as the draw command we seem to lose some data.. maybe its not right.

*/

/* each 512x512 page is
 0x40000 bytes long

therefore a 512x256 page is
 0x20000 bytes...

*/

/* it draws the background and road with direct writes to port 6... but where does this really go? an extra blit source page? */
static int wheelfir_six_pos = 0;
static bitmap_t* render_bitmap;

static WRITE16_HANDLER(wheelfir_blit_w)
{
	//wheelfir_blitdata[offset]=data;
	int width = video_screen_get_width(space->machine->primary_screen);
	int height = video_screen_get_height(space->machine->primary_screen);
	int vpage=0;
	COMBINE_DATA(&wheelfir_blitdata[offset]);

	/* a bit of a hack really .. */
	if(offset==0x6)
	{
		int sixdat = data&0xff;
		int x,y;

		x = wheelfir_six_pos % 512;
		y = wheelfir_six_pos / 512;

		x &= 511;
		y &= 511;

		wheelfir_six_pos++;

		*BITMAP_ADDR16(wheelfir_tmp_bitmap[2], y, x) = sixdat;
		return;
	}


	/* probably wrong, see above! */
	if(offset==0xf && data==0xffff)
	{


		int x,y;
		int xsize,ysize;
		UINT8 *rom = memory_region(space->machine, "gfx1");
		int dir=0;


		wheelfir_six_pos = 0;

		mame_printf_debug("XX %.4x - ",wheelfir_blitdata[0]-wheelfir_blitdata[1]);
		mame_printf_debug("YY %.4x - ",wheelfir_blitdata[2]-wheelfir_blitdata[3]);


		for(x=0;x<16;x++)
			mame_printf_debug("%x-",wheelfir_blitdata[x]);

		mame_printf_debug("\n");

		xsize = ((wheelfir_blitdata[1]&0xff)-(wheelfir_blitdata[0]&0xff))&0xff;
		ysize = ((wheelfir_blitdata[3]&0xff)-(wheelfir_blitdata[2]&0xff))&0x0ff;




		if(wheelfir_blitdata[7]&0x1)
			dir=1;
		else
		{
			dir=-1;
			xsize=0x100-xsize;
		}

		for(y=0;y<=ysize+1/*((wheelfir_blitdata[9]?wheelfir_blitdata[9]:16)*/;y++)
			for(x=0;x<=xsize+1/*(wheelfir_blitdata[9]?wheelfir_blitdata[9]:16)*/;x++)
			{
				int destx=wheelfir_blitdata[0]&0xff;
				int desty=wheelfir_blitdata[2]&0xff;

				int pagenumber = (wheelfir_blitdata[6]&0x3e00)>>9;
				int gfxbase = pagenumber * 0x20000;
				int xbit = (wheelfir_blitdata[6]&0x0100)>>8;


				int offs;
				int pix;
				int diffx,diffy;

				if (wheelfir_blitdata[7]&0x0040)destx +=0x100;
			//  if (wheelfir_blitdata[7]&0x0080) desty +=0x100;

//              if (wheelfir_blitdata[9]&0x0004) destx +=0x100;
//              if (wheelfir_blitdata[9]&0x0008) desty +=0x100;

				diffx=wheelfir_blitdata[0]>>8;
				diffy=wheelfir_blitdata[2]>>8;

			//  diffx-=0xf700;
			//  diffx&=511;


			//  diffy-=0xf400;
//              diffy&=1023;


				diffx &= 0xff;
				diffy &= 0xff;
				if (xbit) diffx+=0x100;

				diffx+=x;
				diffy+=y;


				destx+=x*dir;
				desty+=y;

				offs=gfxbase+diffy*512+diffx;

				offs&=0x3fffff;

				pix = rom[offs];


				if(pix && destx >0 && desty >0 && destx<width && desty <height)
					*BITMAP_ADDR16(wheelfir_tmp_bitmap[vpage], desty, destx) = pix;
			}
	}



}

static VIDEO_START(wheelfir)
{

	wheelfir_tmp_bitmap[0] = video_screen_auto_bitmap_alloc(machine->primary_screen);
	wheelfir_tmp_bitmap[1] = video_screen_auto_bitmap_alloc(machine->primary_screen);
	wheelfir_tmp_bitmap[2] = video_screen_auto_bitmap_alloc(machine->primary_screen);

	render_bitmap =          video_screen_auto_bitmap_alloc(machine->primary_screen);

}

static UINT8 wheelfir_palette[8192];
static int wheelfir_palpos = 0;
/* Press R to show a page of gfx, Q / E to move between pages, and W to clear the framebuffer */
static VIDEO_UPDATE(wheelfir)
{
#if 0
    int x,y;
    static int base = 0;

    if ( input_code_pressed_once(screen->machine, KEYCODE_E) )
        base += 512*512;

    if ( input_code_pressed_once(screen->machine, KEYCODE_Q) )
        base -= 512*512;

    if (base<0x000000) base = 0x000000;
    if (base>0x3c0000) base = 0x3c0000;

    copybitmap(bitmap, wheelfir_tmp_bitmap[2], 0, 0, 0, 0, cliprect);
    //copybitmap_trans(bitmap, wheelfir_tmp_bitmap[1], 0, 0, 0, 0, cliprect, 0);
    copybitmap_trans(bitmap, wheelfir_tmp_bitmap[0], 0, 0, 0, 0, cliprect, 0);
    bitmap_fill(wheelfir_tmp_bitmap[0], video_screen_get_visible_area(screen),0);

    if ( input_code_pressed(screen->machine, KEYCODE_R) )
    {
        const UINT8 *gfx = memory_region(machine, "gfx1");
        for (y=0;y<128;y++)
        {
            for (x=0;x<512;x++)
            {
                int romoffs;
                UINT8 romdata;

                romoffs = base+y*512+x;

                romdata = gfx[romoffs];

                *BITMAP_ADDR16(bitmap, y, x) = romdata;
            }
        }
    }
#endif

	copybitmap(bitmap, render_bitmap, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, wheelfir_tmp_bitmap[0], 0, 0, 0, 0, cliprect, 0);

	{
		int x;
		for (x=0;x<8192;x+=3)
		{
			//int paldat = (wheelfir_palette[x]<<8) | wheelfir_palette[x+1];
			int b,g,r;

			r = wheelfir_palette[x];
			g = wheelfir_palette[x+1];
			b = wheelfir_palette[x+2];
			palette_set_color(screen->machine,x/3,MAKE_RGB(r,g,b));
		}
	}
#if 0
    {
        FILE* fp;
        fp=fopen("wheelfir_pal.dmp", "w+b");
        if (fp)
        {
            fwrite(wheelfir_palette, 1024, 1, fp);
            fclose(fp);
        }
    }
#endif

	return 0;
}

static WRITE16_HANDLER( pal_reset_pos_w )
{
	wheelfir_palpos = 0;
}

static WRITE16_HANDLER( pal_data_w )
{
	wheelfir_palette[wheelfir_palpos] = data & 0xff;
	wheelfir_palpos ++;

	wheelfir_palpos &=8191;
}



static ADDRESS_MAP_START( wheelfir_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE (&wheelfir_myram)

	AM_RANGE(0x7c0000, 0x7c0001) AM_READ(wheelfir_rand1)
	AM_RANGE(0x780000, 0x780001) AM_READ(wheelfir_rand2)
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ_PORT("P1")
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")


	AM_RANGE(0x700000, 0x70001f) AM_WRITE(wheelfir_blit_w) // blitter stuff
	AM_RANGE(0x720000, 0x720001) AM_WRITE(pal_reset_pos_w) // always 0?
	AM_RANGE(0x720002, 0x720003) AM_WRITE(pal_data_w) // lots of different values.. also blitter? palette?
//  AM_RANGE(0x720004, 0x720005) AM_WRITENOP // always ffff?

//  AM_RANGE(0x740000, 0x740001) AM_WRITENOP
//  AM_RANGE(0x760000, 0x760001) AM_WRITENOP
//  AM_RANGE(0x740000, 0x740001) AM_WRITENOP
//  AM_RANGE(0x7a0000, 0x7a0001) AM_WRITENOP
//  AM_RANGE(0x7c0000, 0x7c0001) AM_WRITENOP

  ADDRESS_MAP_END


/* sub is sound cpu? the program roms contain lots of samples */
static ADDRESS_MAP_START( wheelfir_sub, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x780000, 0x780001) AM_READ(wheelfir_rand4)
	AM_RANGE(0x700000, 0x700001) AM_WRITENOP
	AM_RANGE(0x740000, 0x740001) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START( wheelfir )
	PORT_START("IN0")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, "0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0001, "1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x1000, 0x1000, "Test / Game?"  )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const device_config* frame_timer;
static const device_config* scanline_timer;

static TIMER_DEVICE_CALLBACK( frame_timer_callback )
{
	/* callback */
}

static int scanline_counter = 0;
static int total_scanlines = 262;

static void render_background_to_render_buffer(int scanline)
{
	int x;
	UINT16 yscroll = (wheelfir_blitdata[0xb]&0x00ff) | (wheelfir_blitdata[0x8]&0x0080) << 1;
	UINT16 xscroll = (wheelfir_blitdata[0xa]&0x00ff) | (wheelfir_blitdata[0x8]&0x0040) << 2;

	UINT16 *source = BITMAP_ADDR16(wheelfir_tmp_bitmap[2], (scanline+yscroll)&511, 0);
	UINT16 *dest = BITMAP_ADDR16(render_bitmap, scanline, 0);

//  if (scanline==100)
//      printf("%04x %04x %04x %04x\n", wheelfir_blitdata[0x8],wheelfir_blitdata[0x9],wheelfir_blitdata[0xa],wheelfir_blitdata[0xb]);

	for (x=0;x<336;x++)
	{
		dest[x] = source[ (x+xscroll) &511];
	}

}

static TIMER_DEVICE_CALLBACK( scanline_timer_callback )
{
	timer_call_after_resynch(timer->machine, NULL, 0, 0);

	if (scanline_counter != (total_scanlines - 1))
	{
		scanline_counter++;
		cputag_set_input_line(timer->machine, "maincpu", 5, HOLD_LINE); // raster IRQ, changes scroll values for road
		timer_device_adjust_oneshot(timer, attotime_div(ATTOTIME_IN_HZ(60), total_scanlines), 0);

		if (scanline_counter < 256)
		{
			render_background_to_render_buffer(scanline_counter);
		}

		if (scanline_counter == 256)
		{
			cputag_set_input_line(timer->machine, "maincpu", 3, HOLD_LINE); // vblank IRQ?
			toggle_bit = 0x8000; // must toggle..
		}

		if (scanline_counter == 0)
		{
			toggle_bit = 0x0000; // must toggle..
		}

	//  printf("scanline %d\n",scanline_counter);
	}
	else /* pretend we're still on the same scanline to compensate for rounding errors */
	{
		scanline_counter = total_scanlines-1;
	}
}

static VIDEO_EOF( wheelfir )
{
	scanline_counter = -1;
	bitmap_fill(wheelfir_tmp_bitmap[0], video_screen_get_visible_area(machine->primary_screen),0);

	timer_device_adjust_oneshot(frame_timer,  attotime_zero, 0);
	timer_device_adjust_oneshot(scanline_timer,  attotime_zero, 0);
}

static MACHINE_RESET(wheelfir)
{
	frame_timer = devtag_get_device(machine, "frame_timer");
	scanline_timer = devtag_get_device(machine, "scan_timer");
	timer_device_adjust_oneshot(frame_timer, attotime_zero, 0);
	timer_device_adjust_oneshot(scanline_timer,  attotime_zero, 0);
	scanline_counter = -1;
}

static INTERRUPT_GEN( wheelfir_irq )
{
	// we seem to need this interrupt at least once for every object drawn on the screen, otherwise things flicker + slowdown
	cpu_set_input_line(device, 1, HOLD_LINE); // blitter IRQ?
}


static MACHINE_DRIVER_START( wheelfir )
	MDRV_CPU_ADD("maincpu", M68000, 32000000)
	MDRV_CPU_PROGRAM_MAP(wheelfir_main)
	MDRV_CPU_VBLANK_INT_HACK(wheelfir_irq,256)  // 1,3,5 valid

	MDRV_CPU_ADD("sub", M68000, 32000000/2)
	MDRV_CPU_PROGRAM_MAP(wheelfir_sub)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold) // 1 valid

	MDRV_MACHINE_RESET (wheelfir)

	MDRV_TIMER_ADD("frame_timer", frame_timer_callback)
	MDRV_TIMER_ADD("scan_timer", scanline_timer_callback)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(wheelfir)
	MDRV_VIDEO_UPDATE(wheelfir)
	MDRV_VIDEO_EOF(wheelfir)
MACHINE_DRIVER_END


ROM_START( wheelfir )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "tch1.u19", 0x00001, 0x80000, CRC(33bbbc67) SHA1(c2ecc0ab522ee442076ea7b9536aee6e1fad0540) )
	ROM_LOAD16_BYTE( "tch2.u21", 0x00000, 0x80000, CRC(ed6b9e8a) SHA1(214c5aaf55963a219db33dd5d530492e09ad5e07) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code + sound samples */
	ROM_LOAD16_BYTE( "tch3.u83",  0x00001, 0x80000, CRC(43c014a6) SHA1(6c01a08dda204f36e8768795dd5d405576a49140) )
	ROM_LOAD16_BYTE( "tch11.u65", 0x00000, 0x80000, CRC(fc894b2e) SHA1(ebe6d1adf889731fb6f53b4ce5f09c60e2aefb97) )

	ROM_REGION( 0x400000, "gfx1", 0 ) // 512x512 gfx pages
	ROM_LOAD( "tch4.u52", 0x000000, 0x80000, CRC(fe4bc2c7) SHA1(33a2ef79cb13f9e7e7d513915c6e13c4e7fe0188) )
	ROM_LOAD( "tch5.u53", 0x080000, 0x80000, CRC(a38b9ca5) SHA1(083c9f700b9df1039fb553e918e205c6d32057ad) )
	ROM_LOAD( "tch6.u54", 0x100000, 0x80000, CRC(2733ae6b) SHA1(ebd91e123b670159f79be19a552d1ae0c8a0faff) )
	ROM_LOAD( "tch7.u55", 0x180000, 0x80000, CRC(6d98f27f) SHA1(d39f7f184abce645b9165b64e89e3b5354187eea) )
	ROM_LOAD( "tch8.u56", 0x200000, 0x80000, CRC(22b661fe) SHA1(b6edf8e1e8b479ee8813502157615f54627dc7c1) )
	ROM_LOAD( "tch9.u57", 0x280000, 0x80000, CRC(83c66de3) SHA1(50deaf3338d590340b928f891548c47ba8f3ca38) )
	ROM_LOAD( "tch10.u58",0x300000, 0x80000, CRC(2036ed80) SHA1(910381e2ccdbc2d06f873021d8af02795d22f595) )
	ROM_LOAD( "tch12.u59",0x380000, 0x80000, CRC(cce2e675) SHA1(f3d8916077b2e057169d0f254005cd959789a3b3) ) // font is in here
ROM_END

static DRIVER_INIT(wheelfir)
{
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");
	RAM[0xdd3da/2] = 0x4e71; // hack!
}

GAME( 199?, wheelfir,    0, wheelfir,    wheelfir,    wheelfir, ROT0,  "TCH", "Wheels & Fire", GAME_NOT_WORKING|GAME_NO_SOUND )
