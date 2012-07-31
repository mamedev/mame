/***************************************************************************

                            -= Kaneko 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)

    [ 1 High Color Layer ]

        In ROM  (Optional)

    [ 1024 Sprites ]

        Sprites are 16 x 16 x 4 in the older games, 16 x 16 x 8 in
        gtmr & gtmr2.
        Sprites types 0 and 2 can also have a simple effect keeping
        sprites on the screen


**************************************************************************/

#include "emu.h"
#include "includes/kaneko16.h"
#include "kan_pand.h"


struct tempsprite
{
	int code,color;
	int x,y;
	int xoffs,yoffs;
	int flipx,flipy;
	int priority;
};


WRITE16_MEMBER(kaneko16_state::kaneko16_display_enable)
{
	COMBINE_DATA(&m_disp_enable);
}

VIDEO_START( kaneko16_sprites )
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	state->m_disp_enable = 1;	// default enabled for games not using it
	state->m_keep_sprites = 0;	// default disabled for games not using it

	/* 0x400 sprites max */
	state->m_first_sprite = auto_alloc_array(machine, struct tempsprite, 0x400);
}

VIDEO_START( kaneko16_1xVIEW2_tilemaps )
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	state->m_disp_enable = 1; // default enabled for games not using it
	state->m_keep_sprites = 0;	// default disabled for games not using it
	machine.primary_screen->register_screen_bitmap(state->m_sprites_bitmap);
}


VIDEO_START( kaneko16_1xVIEW2 )
{
	VIDEO_START_CALL(kaneko16_sprites);
	VIDEO_START_CALL(kaneko16_1xVIEW2_tilemaps);
}

VIDEO_START( kaneko16_2xVIEW2 )
{
	VIDEO_START_CALL(kaneko16_1xVIEW2);
}

VIDEO_START( sandscrp_1xVIEW2 )
{
	VIDEO_START_CALL(kaneko16_1xVIEW2);
}



/* Berlwall and Gals Panic have an additional hi-color layers */

PALETTE_INIT( berlwall )
{
	int i;

	/* first 2048 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine,2048 + i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

VIDEO_START( berlwall )
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	int sx, x,y;
	UINT8 *RAM	=	state->memregion("gfx3")->base();

	/* Render the hi-color static backgrounds held in the ROMs */

	state->m_bg15_bitmap.allocate(256 * 32, 256 * 1);

/*
    8aba is used as background color
    8aba/2 = 455d = 10001 01010 11101 = $11 $0a $1d
*/

	for (sx = 0 ; sx < 32 ; sx++)	// horizontal screens
	 for (x = 0 ; x < 256 ; x++)	// horizontal pixels
	  for (y = 0 ; y < 256 ; y++)	// vertical pixels
	  {
			int addr  = sx * (256 * 256) + x + y * 256;
			int data = RAM[addr * 2 + 0] * 256 + RAM[addr * 2 + 1];
			int r,g,b;

			r = (data & 0x07c0) >>  6;
			g = (data & 0xf800) >> 11;
			b = (data & 0x003e) >>  1;

			/* apply a simple decryption */
			r ^= 0x09;

			if (~g & 0x08) g ^= 0x10;
			g = (g - 1) & 0x1f;		/* decrease with wraparound */

			b ^= 0x03;
			if (~b & 0x08) b ^= 0x10;
			b = (b + 2) & 0x1f;		/* increase with wraparound */

			/* kludge to fix the rollercoaster picture */
			if ((r & 0x10) && (b & 0x10))
				g = (g - 1) & 0x1f;		/* decrease with wraparound */

			state->m_bg15_bitmap.pix16(y, sx * 256 + x) = 2048 + ((g << 10) | (r << 5) | b);
	  }

	VIDEO_START_CALL(kaneko16_1xVIEW2);
}

VIDEO_START( galsnew )
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	VIDEO_START_CALL(kaneko16_sprites);

	machine.primary_screen->register_screen_bitmap(state->m_sprites_bitmap);

}

/***************************************************************************

                                Sprites Drawing

    Sprite data is layed out in RAM in different ways for different games
    (type 0,1,2,etc.). This basically involves the bits in the attribute
    word to be shuffled around and/or the words being in different order.

    Each sprite is always stuffed in 4 words. There may be some extra
    padding words though (e.g. type 2 sprites are like type 0 but the
    data is held in the last 8 bytes of every 16). Examples are:

    Type 0: shogwarr, blazeon, bakubrkr.
    Type 1: gtmr.
    Type 2: berlwall

Offset:         Format:                     Value:

0000.w          Attribute (type 0 & 2)

                    f--- ---- ---- ----     Multisprite: Use Latched Code + 1
                    -e-- ---- ---- ----     Multisprite: Use Latched Color (And Flip?)
                    --d- ---- ---- ----     Multisprite: Use Latched X,Y As Offsets
                    ---c ba-- ---- ----
                    ---- --9- ---- ----     High Priority (vs FG Tiles Of High Priority)
                    ---- ---8 ---- ----     High Priority (vs BG Tiles Of High Priority)
                    ---- ---- 7654 32--     Color
                    ---- ---- ---- --1-     X Flip
                    ---- ---- ---- ---0     Y Flip

                Attribute (type 1)

                    f--- ---- ---- ----     Multisprite: Use Latched Code + 1
                    -e-- ---- ---- ----     Multisprite: Use Latched Color (And Flip?)
                    --d- ---- ---- ----     Multisprite: Use Latched X,Y As Offsets
                    ---c ba-- ---- ----
                    ---- --9- ---- ----     X Flip
                    ---- ---8 ---- ----     Y Flip
                    ---- ---- 7--- ----     High Priority (vs FG Tiles Of High Priority)
                    ---- ---- -6-- ----     High Priority (vs BG Tiles Of High Priority)
                    ---- ---- --54 3210     Color

0002.w                                      Code
0004.w                                      X Position << 6
0006.w                                      Y Position << 6

***************************************************************************/

#define USE_LATCHED_XY		1
#define USE_LATCHED_CODE	2
#define USE_LATCHED_COLOR	4

static int kaneko16_parse_sprite_type012(running_machine &machine, int i, struct tempsprite *s)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int attr, xoffs, offs;

	if (state->m_sprite_type == 2)	offs = i * 16/2 + 0x8/2;
	else							offs = i * 8/2;

	if (offs >= (state->m_spriteram.bytes()/2))	return -1;

	attr			=		spriteram16[offs + 0];
	s->code			=		spriteram16[offs + 1];
	s->x			=		spriteram16[offs + 2];
	s->y			=		spriteram16[offs + 3];

	if (state->m_sprite_type == 1)
	{
	s->color		=		(attr & 0x003f);
	s->priority		=		(attr & 0x00c0) >> 6;
	s->flipy		=		(attr & 0x0100);
	s->flipx		=		(attr & 0x0200);
	s->code			+=		(s->y & 1) << 16;	// bloodwar
	}
	else
	{
	s->flipy		=		(attr & 0x0001);
	s->flipx		=		(attr & 0x0002);
	s->color		=		(attr & 0x00fc) >> 2;
	s->priority		=		(attr & 0x0300) >> 8;
	}
	xoffs			=		(attr & 0x1800) >> 11;
	s->yoffs		=		state->m_sprites_regs[0x10/2 + xoffs*2 + 1];
	s->xoffs		=		state->m_sprites_regs[0x10/2 + xoffs*2 + 0];

if (state->m_sprite_flipy)
{
	s->yoffs		-=		state->m_sprites_regs[0x2/2];
	s->yoffs		-=		machine.primary_screen->visible_area().min_y<<6;
}
else
{
	s->yoffs		-=		state->m_sprites_regs[0x2/2];
	s->yoffs		+=		machine.primary_screen->visible_area().min_y<<6;
}

	return					( (attr & 0x2000) ? USE_LATCHED_XY    : 0 ) |
							( (attr & 0x4000) ? USE_LATCHED_COLOR : 0 ) |
							( (attr & 0x8000) ? USE_LATCHED_CODE  : 0 ) ;
}

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities
static void kaneko16_draw_sprites_custom(bitmap_ind16 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int priority)
{
	pen_t pen_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	bitmap_ind8 &priority_bitmap = gfx->machine().priority_bitmap;
	int sprite_screen_height = ((1<<16)*gfx->height+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width+0x8000)>>16;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width<<16)/sprite_screen_width;
		int dy = (gfx->height<<16)/sprite_screen_height;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < clip.min_y )
		{ /* clip top */
			int pixels = clip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > clip.max_x+1 )
		{ /* clip right */
			int pixels = ex-clip.max_x-1;
			ex -= pixels;
		}
		if( ey > clip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-clip.max_y-1;
			ey -= pixels;
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &priority_bitmap.pix8(y);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c != 0 )
					{
						if (pri[x] < priority)
							dest[x] = pen_base + c;
						pri[x] = 0xff; // mark it "already drawn"
					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}

/* Build a list of sprites to display & draw them */

void kaneko16_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	/* Sprites *must* be parsed from the first in RAM to the last,
       because of the multisprite feature. But they *must* be drawn
       from the last in RAM (frontmost) to the first in order to
       cope with priorities using pdrawgfx.

       Hence we parse them from first to last and put the result
       in a temp buffer, then draw the buffer's contents from last
       to first. */

	int max	=	(machine.primary_screen->width() > 0x100) ? (0x200<<6) : (0x100<<6);

	int i = 0;
	struct tempsprite *s = state->m_first_sprite;

	/* These values are latched from the last sprite. */
	int x			=	0;
	int y			=	0;
	int code		=	0;
	int color		=	0;
	int priority	=	0;
	int xoffs		=	0;
	int yoffs		=	0;
	int flipx		=	0;
	int flipy		=	0;

	while (1)
	{
		int flags;

		switch( state->m_sprite_type )
		{
			case 0:
			case 1:
			case 2:		flags = kaneko16_parse_sprite_type012(machine, i,s);	break;
			default:	flags = -1;
		}

		if (flags == -1)	// End of Sprites
			break;

		if (flags & USE_LATCHED_CODE)
			s->code = ++code;	// Use the latched code + 1 ..
		else
			code = s->code;		// .. or latch this value


		if (flags & USE_LATCHED_COLOR)
		{
			s->color		=	color;
			s->priority		=	priority;
			s->xoffs		=	xoffs;
			s->yoffs		=	yoffs;

			if (state->m_sprite_fliptype==0)
			{
				s->flipx		=	flipx;
				s->flipy		=	flipy;
			}
		}
		else
		{
			color		=	s->color;
			priority	=	s->priority;
			xoffs		=	s->xoffs;
			yoffs		=	s->yoffs;

			if (state->m_sprite_fliptype==0)
			{
				flipx = s->flipx;
				flipy = s->flipy;
			}
		}

		// brap boys explicitly doesn't want the flip to be latched, maybe there is a different bit to enable that behavior?
		if (state->m_sprite_fliptype==1)
		{
			flipx		=	s->flipx;
			flipy		=	s->flipy;
		}

		if (flags & USE_LATCHED_XY)
		{
			s->x += x;
			s->y += y;
		}
		// Always latch the latest result
		x	=	s->x;
		y	=	s->y;

		/* We can now buffer this sprite */

		s->x	=	s->xoffs + s->x;
		s->y	=	s->yoffs + s->y;

		s->x	+=	state->m_sprite_xoffs;
		s->y	+=	state->m_sprite_yoffs;

		if (state->m_sprite_flipx)	{ s->x = max - s->x - (16<<6);	s->flipx = !s->flipx;	}
		if (state->m_sprite_flipy)	{ s->y = max - s->y - (16<<6);	s->flipy = !s->flipy;	}

		s->x		=		( (s->x & 0x7fc0) - (s->x & 0x8000) ) / 0x40;
		s->y		=		( (s->y & 0x7fc0) - (s->y & 0x8000) ) / 0x40;

		i++;
		s++;
	}


	/* Let's finally draw the sprites we buffered, in reverse order
       (for pdrawgfx) */

	for (s--; s >= state->m_first_sprite; s--)
	{
		int curr_pri = s->priority;

		UINT32 primask = state->m_priority.sprite[curr_pri];

		kaneko16_draw_sprites_custom(
										bitmap,cliprect,machine.gfx[0],
										s->code,
										s->color,
										s->flipx, s->flipy,
										s->x, s->y,
										primask );
	}
}



/***************************************************************************


                            Sprites Registers

    Offset:         Format:                     Value:

    0000.w          f--- ---- ---- ----         Sprites Disable?? (see blazeon)
                    -edc ba98 7654 3---
                    ---- ---- ---- -2--         Keep sprites on screen (only sprites types 0 and 2)
                    ---- ---- ---- --1-         Flip X
                    ---- ---- ---- ---0         Flip Y

    0002.w                                      Y Offset << 6 (Global)


    0004..000e.w                                ?


    0010.w                                      X Offset << 6 #0
    0012.w                                      Y Offset << 6 #0

    0014.w                                      X Offset << 6 #1
    0016.w                                      Y Offset << 6 #1

    0018.w                                      X Offset << 6 #2
    001a.w                                      Y Offset << 6 #2

    001c.w                                      X Offset << 6 #3
    001e.w                                      Y Offset << 6 #3

***************************************************************************/

/*
[gtmr]

Initial self test:
600000: 4BC0 94C0 4C40 94C0-0404 0002 0000 0000     (Layers 1 regs)
680000: 4BC0 94C0 4C40 94C0-1C1C 0002 0000 0000     (Layers 2 regs)
Race start:
600000: DC00 7D00 DC80 7D00-0404 0002 0000 0000     (Layers 1 regs)
680000: DC00 7D00 DC80 7D00-1C1C 0002 0000 0000     (Layers 2 regs)

[gtmr]
700000: 0040 0000 0001 0180-0000 0000 0000 0000     (Sprites  regs)
700010: 0040 0000 0040 0000-0040 0000 2840 1E00     ; 1,0 .. a1,78
                                                    ; a0*2=screenx/2
                                                    ; 78*2=screeny/2
FLIP ON:
700000: 0043 FFC0 0001 0180-0000 0000 0000 0000     (Sprites  regs)
700010: 2FC0 4400 2FC0 4400-2FC0 4400 57C0 6200     ; bf,110 .. 15f,188
                                                    ; 15f-bf=a0! 188-110=78!

[berlwall]
600000: 48CC 03C0 0001 0100-0000 0000 0000 0000     (Sprites  regs)
600010: 0000 0000 0000 0000-0000 0000 0000 0000
FLIP ON:
600000: 48CF FC00 0001 0100-0000 0000 0000 0000     (Sprites  regs)
600010: 0000 0000 0000 0000-0000 0000 0000 0000

[mgcrystl]
900000: 4FCC 0000 0040 00C0-xxxx 0001 0001 0001     (Sprites  regs)
900010: 0000 FC40 A000 9C40-1E00 1A40 0000 FC40
FLIP ON:
900000: 4FCF 0000 0040 00C0-xxxx 0001 0001 0001     (Sprites  regs)
900010: 0000 0400 A000 A400-1E00 2200 0000 0400     ; +1f<<6 on y
*/

READ16_MEMBER(kaneko16_state::kaneko16_sprites_regs_r)
{
	return m_sprites_regs[offset];
}

WRITE16_MEMBER(kaneko16_state::kaneko16_sprites_regs_w)
{
	UINT16 new_data;

	COMBINE_DATA(&m_sprites_regs[offset]);
	new_data  = m_sprites_regs[offset];

	switch (offset)
	{
		case 0:
			if (ACCESSING_BITS_0_7)
			{
				m_sprite_flipx = new_data & 2;
				m_sprite_flipy = new_data & 1;

				if(m_sprite_type == 0 || m_sprite_type == 2)
					m_keep_sprites = ~new_data & 4;
			}

			break;
	}

//  logerror("CPU #0 PC %06X : Warning, sprites reg %04X <- %04X\n",cpu_get_pc(&space.device()),offset*2,data);
}







/* Select the high color background image (out of 32 in the ROMs) */
READ16_MEMBER(kaneko16_state::kaneko16_bg15_select_r)
{
	return m_bg15_select[0];
}
WRITE16_MEMBER(kaneko16_state::kaneko16_bg15_select_w)
{
	COMBINE_DATA(&m_bg15_select[0]);
}

/* ? */
READ16_MEMBER(kaneko16_state::kaneko16_bg15_reg_r)
{
	return m_bg15_reg[0];
}
WRITE16_MEMBER(kaneko16_state::kaneko16_bg15_reg_w)
{
	COMBINE_DATA(&m_bg15_reg[0]);
}



static void kaneko16_render_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	/* Sprites last (rendered with pdrawgfx, so they can slip
       in between the layers) */

	if(state->m_keep_sprites)
	{
		/* keep sprites on screen */
		kaneko16_draw_sprites(machine,state->m_sprites_bitmap,cliprect);
		copybitmap_trans(bitmap,state->m_sprites_bitmap,0,0,0,0,cliprect,0);
	}
	else
	{
		state->m_sprites_bitmap.fill(0, cliprect);
		kaneko16_draw_sprites(machine,bitmap,cliprect);
	}
}

static void kaneko16_render_15bpp_bitmap(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	if (state->m_bg15_bitmap.valid())
	{
		int select	=	state->m_bg15_select[ 0 ];
//      int reg     =   state->m_bg15_reg[ 0 ];
		int flip	=	select & 0x20;
		int sx, sy;

		if (flip)	select ^= 0x1f;

		sx		=	(select & 0x1f) * 256;
		sy		=	0;

		copybitmap(bitmap, state->m_bg15_bitmap, flip, flip, -sx, -sy, cliprect);

//      flag = 0;
	}
}

static void kaneko16_fill_bitmap(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	kaneko16_state *state = machine.driver_data<kaneko16_state>();
	if(state->m_sprite_type == 1)
		bitmap.fill(0x7f00, cliprect);
	else
		/* Fill the bitmap with pen 0. This is wrong, but will work most of
           the times. To do it right, each pixel should be drawn with pen 0
           of the bottomost tile that covers it (which is pretty tricky to do) */
		bitmap.fill(0, cliprect);
}

static SCREEN_UPDATE_IND16( common )
{
	int i;
	kaneko16_state *state = screen.machine().driver_data<kaneko16_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	if (state->m_view2_0) state->m_view2_0->kaneko16_prepare(bitmap, cliprect);
	if (state->m_view2_1) state->m_view2_1->kaneko16_prepare(bitmap, cliprect);

	for ( i = 0; i < 8; i++ )
	{
		if (state->m_view2_0) state->m_view2_0->render_tilemap_chip(bitmap,cliprect,i);
		if (state->m_view2_1) state->m_view2_1->render_tilemap_chip_alt(bitmap,cliprect,i, state->m_priority.VIEW2_2_pri);
	}

	return 0;
}

SCREEN_UPDATE_IND16(berlwall)
{
	kaneko16_state *state = screen.machine().driver_data<kaneko16_state>();
	// berlwall uses a 15bpp bitmap as a bg, not a solid fill
	kaneko16_render_15bpp_bitmap(screen.machine(),bitmap,cliprect);

	// if the display is disabled, do nothing?
	if (!state->m_disp_enable) return 0;

	SCREEN_UPDATE16_CALL(common);
	kaneko16_render_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}


SCREEN_UPDATE_IND16( jchan_view2 )
{
	SCREEN_UPDATE16_CALL(common);
	return 0;
}


SCREEN_UPDATE_IND16( kaneko16 )
{
	kaneko16_state *state = screen.machine().driver_data<kaneko16_state>();
	kaneko16_fill_bitmap(screen.machine(),bitmap,cliprect);

	// if the display is disabled, do nothing?
	if (!state->m_disp_enable) return 0;

	SCREEN_UPDATE16_CALL(common);
	kaneko16_render_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( galsnew )
{
	kaneko16_state *state = screen.machine().driver_data<kaneko16_state>();
//  kaneko16_fill_bitmap(screen.machine(),bitmap,cliprect);
	int y,x;
	int count;


	count = 0;
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (state->m_galsnew_fg_pixram[count] & 0xfffe)>>1;
			dat+=2048;
			dest[x] = dat;
			count++;
		}
	}

	count = 0;
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (state->m_galsnew_bg_pixram[count]);
			//dat &=0x3ff;
			if (dat)
				dest[x] = dat;

			count++;
		}
	}


	// if the display is disabled, do nothing?
	if (!state->m_disp_enable) return 0;

	SCREEN_UPDATE16_CALL(common);

	kaneko16_render_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}


SCREEN_UPDATE_IND16( sandscrp )
{
	kaneko16_state *state = screen.machine().driver_data<kaneko16_state>();
	device_t *pandora = screen.machine().device("pandora");
	kaneko16_fill_bitmap(screen.machine(),bitmap,cliprect);

	// if the display is disabled, do nothing?
	if (!state->m_disp_enable) return 0;

	SCREEN_UPDATE16_CALL(common);

	// copy sprite bitmap to screen
	pandora_update(pandora, bitmap, cliprect);
	return 0;
}


