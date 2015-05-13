// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+

**********************************************************************/

#include "emu.h"
#include "includes/hp48.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/


#define VERBOSE       0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    GLOBAL VARIABLES & CONSTANTS
***************************************************************************/

/* base colors */
static const int hp48_bg_color[3] = { 136, 147, 109 };  /* yellow */
static const int hp48_fg_color[3] = {   0,   0,  64 };  /* dark blue */

/* color mixing */
#define mix(c1,c2,x) (c1)*(1-(x))+(c2)*(x)
#define mix2(i,x) mix(hp48_bg_color[i],hp48_fg_color[i],x)




/***************************************************************************
    FUNCTIONS
***************************************************************************/

PALETTE_INIT_MEMBER(hp48_state, hp48)
{
	int i;
	for ( i = 0; i < 255; i++ )
	{
		float c = i/255.;
		m_palette->set_pen_color( i, rgb_t( 0, mix2(0,c), mix2(1,c), mix2(2,c) ) );
	}
}



/* The screen is organised as follows:



            announciators

        ----------------------      ^       ^
       |                      |     |       |
       |                      |     |       |
       |                      |     |       |
       |                      |  M lines    |
       |      main screen     |     |       |
       |                      |     |       |
       |                      |     |    64 lines
       |                      |     |       |
       |                      |     |       |
       |----------------------|     -       |
       |                      |     |       |
       |        menu          |  m lines    |
       |                      |     |       |
        ----------------------      v       v

       <----- 131 columns ---->



        The LCD is 131x64 pixels and has two components:
    - a main screen
    - a menu screen

    The main height (M) and the menu height (m) can be changed with the constraints:
    - m+M=64
    - M>=2

    Pixels are 1-bit, packed in memory.

    The start address of both screens can be changed independently
    (this allows smooth, pixel-precise vertical scrooling of both parts).
    They must be even addresses (in nibbles).

    The stride (offset in nibbles between two scanlines) can be changed for the main screen.
    It must be even.
    It is fixed to 34 for the menu screen.

    The bit offset for the first column of the main screen can be changed
    (this allows smooth, pixel-precise horizontal scrooling).
    It is always 0 for the menu screen.

    Above the LCD, there are 6 annonciators that can be independently turned
    on and off (and independetly from the LCD).
    They are not handled here, but through output_set_value.
 */


/*
        In theory, the LCD is monorchrome, with a global adjustable contrast (32 levels).
    However, by switching between screens at each refresh (64 Hz), one can achieve the
    illusion of grayscale, with moderate flickering.
    This technique was very widespread.
    We emulate it by simply averaging between the last few (HP48_NB_SCREENS) frames.

    HP48_NB_SCREENS should be a multiple of the period of screen flips to avoid
    flickering in the emulation.
 */


#define draw_pixel                          \
	m_screens[ m_cur_screen ][ y ][ xp + 8 ] = (data & 1) ? fg : 0; \
	xp++;                               \
	data >>= 1

#define draw_quart                  \
	UINT8 data = space.read_byte( addr );   \
	draw_pixel; draw_pixel; draw_pixel; draw_pixel;


UINT32 hp48_state::screen_update_hp48(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int x, y, xp, i, addr;
	int display       = HP48_IO_4(0) >> 3;           /* 1=on, 0=off */
	int left_margin   = HP48_IO_4(0) & 7;            /* 0..7 pixels for main bitmap */
	int contrast      = HP48_IO_8(1) & 0x1f;         /* 0..31 */
	int refresh       = HP48_IO_4(3) >> 3;           /* vertical refresh */
	int bitmap_start  = HP48_IO_20(0x20) & ~1;       /* main bitmap address */
	int right_margin  = HP48_IO_12(0x25) & ~1;       /* -2048..2046 nibbles for main bitmap */
	int last_line     = HP48_IO_8(0x28) & 0x3f;      /* 2..63 lines of main bitmap before menu */
	int menu_start    = HP48_IO_20(0x30) & ~1;       /* menu bitmap address */
	int fg = contrast + 2;

	LOG(( "%f hp48 video_update called: ", machine().time().as_double()));

	if ( !display || refresh )
	{
		LOG(( "display off\n" ));
		bitmap.fill(0 );
		return 0;
	}

	/* correcting factors */
	if ( right_margin & 0x800 ) right_margin -= 0x1000;
	if ( last_line <= 1 ) last_line = 0x3f;

	LOG(( "on=%i lmargin=%i rmargin=%i contrast=%i start=%05x lline=%i menu=%05x\n",
			display, left_margin, right_margin, contrast, bitmap_start, last_line, menu_start ));

	/* draw main bitmap */
	addr = bitmap_start;
	for ( y = 0; y <= last_line; y++ )
	{
		xp = -left_margin;
		for ( x = 0; x < 34; x++, addr++ )
		{
			draw_quart;
		}
		addr += (right_margin + (left_margin / 4) + 1) & ~1;
	}

	/* draw menu bitmap */
	addr = menu_start;
	for ( ; y <= 0x3f; y++ )
	{
		xp = 0;
		for ( x = 0; x < 34; x++, addr++ )
		{
			draw_quart;
		}
	}

	/* draw averaged frames */
	for ( y = 0; y < 64; y++ )
	{
		for ( x = 0; x < 131; x++ )
		{
			int acc = 0;
			for ( i = 0; i < HP48_NB_SCREENS; i++ )
			{
				acc += m_screens[ i ][ y ][ x+8 ];
			}
			acc = (acc * 255) / (33 * HP48_NB_SCREENS);
			bitmap.pix16(y, x ) = acc;
		}
	}

	m_cur_screen = (m_cur_screen + 1) % HP48_NB_SCREENS;

	return 0;
}
