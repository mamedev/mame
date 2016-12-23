// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/timex.h
 *
 ****************************************************************************/

#ifndef __TIMEX_H__
#define __TIMEX_H__

/* Border sizes for TS2068. These are guesses based on the number of cycles
   available per frame. */
#define TS2068_TOP_BORDER    32
#define TS2068_BOTTOM_BORDER 32
#define TS2068_SCREEN_HEIGHT (TS2068_TOP_BORDER + SPEC_DISPLAY_YSIZE + TS2068_BOTTOM_BORDER)

/* Double the border sizes to maintain ratio of screen to border */
#define TS2068_LEFT_BORDER   96   /* Number of left hand border pixels */
#define TS2068_DISPLAY_XSIZE 512  /* Horizontal screen resolution */
#define TS2068_RIGHT_BORDER  96   /* Number of right hand border pixels */
#define TS2068_SCREEN_WIDTH (TS2068_LEFT_BORDER + TS2068_DISPLAY_XSIZE + TS2068_RIGHT_BORDER)

enum
{
	TIMEX_CART_NONE,
	TIMEX_CART_DOCK,
	TIMEX_CART_EXROM,
	TIMEX_CART_HOME
};



#endif /* __TIMEX_H__ */
