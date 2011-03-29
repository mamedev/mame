/***************************************************************************

                            -= Gal's Panic II =-

                    driver by   Luca Elia (l.elia@tin.it)


***************************************************************************/

#include "emu.h"
#include "includes/kaneko16.h"
#include "includes/galpani2.h"

/*
304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprites regs)
304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
*/

/***************************************************************************


                        Palettized Background Layers


***************************************************************************/


#ifdef UNUSED_DEFINITION
#define galpani2_BG8_REGS_R( _n_ ) \
READ16_HANDLER( galpani2_bg8_regs_##_n_##_r ) \
{ \
	galpani2_state *state = space->machine().driver_data<galpani2_state>(); \
	switch (offset * 2) \
	{ \
		case 0x16:	return space->machine().rand() & 1; \
		default: \
			logerror("CPU #0 PC %06X : Warning, bg8 #%d screen reg %04X read\n",cpu_get_pc(&space->device()),_n_,offset*2); \
	} \
	return state->bg8_regs_##_n_[offset]; \
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
	galpani2_state *state = space->machine().driver_data<galpani2_state>(); \
	COMBINE_DATA(&state->bg8_regs_##_n_[offset]); \
}
#endif

#define galpani2_BG8_W( _n_ ) \
WRITE16_HANDLER( galpani2_bg8_##_n_##_w ) \
{ \
	galpani2_state *state = space->machine().driver_data<galpani2_state>(); \
	int x,y,pen; \
	UINT16 newword = COMBINE_DATA(&state->bg8_##_n_[offset]); \
	pen	=	newword & 0xff; \
	x	=	(offset % 512);	/* 512 x 256 */ \
	y	=	(offset / 512); \
	*BITMAP_ADDR16(state->bg8_bitmap_##_n_, y, x) = 0x4000 + pen; \
}

#define galpani2_BG8_PALETTE_W( _n_ ) \
WRITE16_HANDLER( galpani2_palette_##_n_##_w ) \
{ \
	galpani2_state *state = space->machine().driver_data<galpani2_state>(); \
	UINT16 newword = COMBINE_DATA(&state->palette_##_n_[offset]); \
	palette_set_color_rgb( space->machine(), offset + 0x4000 + _n_ * 0x100, pal5bit(newword >> 5), pal5bit(newword >> 10), pal5bit(newword >> 0) ); \
}

#ifdef UNUSED_FUNCTION
galpani2_BG8_REGS_R( 0 )
galpani2_BG8_REGS_R( 1 )

galpani2_BG8_REGS_W( 0 )
galpani2_BG8_REGS_W( 1 )
#endif

galpani2_BG8_W( 0 )
galpani2_BG8_W( 1 )

galpani2_BG8_PALETTE_W( 0 )
galpani2_BG8_PALETTE_W( 1 )


/***************************************************************************


                            xRGB  Background Layer


***************************************************************************/

/* 8 horizontal pages of 256x256 pixels? */
WRITE16_HANDLER( galpani2_bg15_w )
{
	galpani2_state *state = space->machine().driver_data<galpani2_state>();
	UINT16 newword = COMBINE_DATA(&state->bg15[offset]);

	int x = (offset % 256) + (offset / (256*256)) * 256 ;
	int y = (offset / 256) % 256;

	*BITMAP_ADDR16(state->bg15_bitmap, y, x) = 0x4200 + (newword & 0x7fff);
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
	galpani2_state *state = machine.driver_data<galpani2_state>();
	state->bg15_bitmap  = auto_bitmap_alloc(machine, 256*8, 256, BITMAP_FORMAT_INDEXED16);
	state->bg8_bitmap_0 = auto_bitmap_alloc(machine, 512, 256, BITMAP_FORMAT_INDEXED16);
	state->bg8_bitmap_1 = auto_bitmap_alloc(machine, 512, 256, BITMAP_FORMAT_INDEXED16);

	VIDEO_START_CALL(kaneko16_sprites);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE( galpani2 )
{
	galpani2_state *state = screen->machine().driver_data<galpani2_state>();
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine(), KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine(), KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine(), KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine(), KEYCODE_E))	msk |= 4;
	if (input_code_pressed(screen->machine(), KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(screen->machine().priority_bitmap,cliprect,0);

	if (layers_ctrl & 0x1)
	{
		int x = 0;
		int y = 0;
		copyscrollbitmap_trans(bitmap, state->bg15_bitmap,
							   1, &x, 1, &y,
							   cliprect,0x4200 + 0);
	}

/*  test mode:
    304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprite regs)
    304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
    16c0/40 = 5b        200/40 = 8
    scrollx = f5, on screen x should be 0 (f5+5b = 150) */

	if (layers_ctrl & 0x2)
	{
		int x = - ( *state->bg8_0_scrollx + 0x200 - 0x0f5 );
		int y = - ( *state->bg8_0_scrolly + 0x200 - 0x1be );
		copyscrollbitmap_trans(bitmap, state->bg8_bitmap_0,
							   1, &x, 1, &y,
							   cliprect,0x4000 + 0);
	}

	if (layers_ctrl & 0x4)
	{
		int x = - ( *state->bg8_1_scrollx + 0x200 - 0x0f5 );
		int y = - ( *state->bg8_1_scrolly + 0x200 - 0x1be );
		copyscrollbitmap_trans(bitmap, state->bg8_bitmap_1,
							   1, &x, 1, &y,
							   cliprect,0x4000 + 0);
	}

	if (layers_ctrl & 0x8)	kaneko16_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
