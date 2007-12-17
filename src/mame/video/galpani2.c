/***************************************************************************

                            -= Gal's Panic II =-

                    driver by   Luca Elia (l.elia@tin.it)


***************************************************************************/

#include "driver.h"
#include "kaneko16.h"

/*
304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprites regs)
304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
*/

/***************************************************************************


                        Palettized Background Layers


***************************************************************************/

UINT16 *galpani2_bg8_0,         *galpani2_bg8_1;
UINT16 *galpani2_palette_0,     *galpani2_palette_1;
UINT16 *galpani2_bg8_regs_0,    *galpani2_bg8_regs_1;
UINT16 *galpani2_bg8_0_scrollx, *galpani2_bg8_1_scrollx;
UINT16 *galpani2_bg8_0_scrolly, *galpani2_bg8_1_scrolly;

static mame_bitmap *galpani2_bg8_bitmap_0, *galpani2_bg8_bitmap_1;

#define galpani2_BG8_REGS_R( _n_ ) \
READ16_HANDLER( galpani2_bg8_regs_##_n_##_r ) \
{ \
	switch (offset * 2) \
	{ \
		case 0x16:	return mame_rand(Machine) & 1; \
		default: \
			logerror("CPU #0 PC %06X : Warning, bg8 #%d screen reg %04X read\n",activecpu_get_pc(),_n_,offset*2); \
	} \
	return galpani2_bg8_regs_##_n_[offset]; \
}

/*
    000-3ff     row? scroll
    400         ?
    800-bff     col? scroll
    c04         0003 flip, 0300 flip?
    c1c/e       01ff scroll, 3000 ?
*/
#define galpani2_BG8_REGS_W( _n_ ) \
WRITE16_HANDLER( galpani2_bg8_regs_##_n_##_w ) \
{ \
	COMBINE_DATA(&galpani2_bg8_regs_##_n_[offset]); \
}

#define galpani2_BG8_W( _n_ ) \
WRITE16_HANDLER( galpani2_bg8_##_n_##_w ) \
{ \
	int x,y,pen; \
	UINT16 newword = COMBINE_DATA(&galpani2_bg8_##_n_[offset]); \
	pen	=	newword & 0xff; \
	x	=	(offset % 512);	/* 512 x 256 */ \
	y	=	(offset / 512); \
	*BITMAP_ADDR16(galpani2_bg8_bitmap_##_n_, y, x) = Machine->pens[0x4000 + pen]; \
}

#define galpani2_BG8_PALETTE_W( _n_ ) \
WRITE16_HANDLER( galpani2_palette_##_n_##_w ) \
{ \
	UINT16 newword = COMBINE_DATA(&galpani2_palette_##_n_[offset]); \
	palette_set_color_rgb( Machine, offset + 0x4000 + _n_ * 0x100, pal5bit(newword >> 5), pal5bit(newword >> 10), pal5bit(newword >> 0) ); \
}

galpani2_BG8_REGS_R( 0 )
galpani2_BG8_REGS_R( 1 )

galpani2_BG8_REGS_W( 0 )
galpani2_BG8_REGS_W( 1 )

galpani2_BG8_W( 0 )
galpani2_BG8_W( 1 )

galpani2_BG8_PALETTE_W( 0 )
galpani2_BG8_PALETTE_W( 1 )


/***************************************************************************


                            xRGB  Background Layer


***************************************************************************/

UINT16 *galpani2_bg15;

static mame_bitmap *galpani2_bg15_bitmap;

/* 8 horizontal pages of 256x256 pixels? */
WRITE16_HANDLER( galpani2_bg15_w )
{
	UINT16 newword = COMBINE_DATA(&galpani2_bg15[offset]);

	int x = (offset % 256) + (offset / (256*256)) * 256 ;
	int y = (offset / 256) % 256;

	*BITMAP_ADDR16(galpani2_bg15_bitmap, y, x) = Machine->pens[0x4200 + (newword & 0x7fff)];
}


/***************************************************************************


                            Video Init Functions


***************************************************************************/

PALETTE_INIT( galpani2 )
{
	int i;
	/* first $4200 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 0x8000; i++)
		palette_set_color_rgb(machine,0x4200+i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

VIDEO_START( galpani2 )
{
	galpani2_bg15_bitmap  = auto_bitmap_alloc(256*8, 256, BITMAP_FORMAT_INDEXED16);
	galpani2_bg8_bitmap_0 = auto_bitmap_alloc(512, 256, BITMAP_FORMAT_INDEXED16);
	galpani2_bg8_bitmap_1 = auto_bitmap_alloc(512, 256, BITMAP_FORMAT_INDEXED16);

	video_start_kaneko16_sprites(machine);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( galpani2 )
{
	int layers_ctrl = -1;

	galpani2_mcu_run();

#ifdef MAME_DEBUG
if (input_code_pressed(KEYCODE_Z))
{	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (input_code_pressed(KEYCODE_E))	msk |= 4;
	if (input_code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;	}
#endif

	fillbitmap(bitmap,machine->pens[0],cliprect);
	fillbitmap(priority_bitmap,0,cliprect);

	if (layers_ctrl & 0x1)
	{
		int x = 0;
		int y = 0;
		copyscrollbitmap(	bitmap, galpani2_bg15_bitmap,
							1, &x, 1, &y,
							cliprect,TRANSPARENCY_PEN,machine->pens[0x4200 + 0]);
	}

/*  test mode:
    304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprite regs)
    304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
    16c0/40 = 5b        200/40 = 8
    scrollx = f5, on screen x should be 0 (f5+5b = 150) */

	if (layers_ctrl & 0x2)
	{
		int x = - ( *galpani2_bg8_0_scrollx + 0x200 - 0x0f5 );
		int y = - ( *galpani2_bg8_0_scrolly + 0x200 - 0x1be );
		copyscrollbitmap(	bitmap, galpani2_bg8_bitmap_0,
							1, &x, 1, &y,
							cliprect,TRANSPARENCY_PEN,machine->pens[0x4000 + 0]);
	}

	if (layers_ctrl & 0x4)
	{
		int x = - ( *galpani2_bg8_1_scrollx + 0x200 - 0x0f5 );
		int y = - ( *galpani2_bg8_1_scrolly + 0x200 - 0x1be );
		copyscrollbitmap(	bitmap, galpani2_bg8_bitmap_1,
							1, &x, 1, &y,
							cliprect,TRANSPARENCY_PEN,machine->pens[0x4000 + 0]);
	}

	if (layers_ctrl & 0x8)	kaneko16_draw_sprites(machine, bitmap, cliprect);
	return 0;
}
