/*********************************************************************

    generic.h

    Generic simple video functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __VIDEO_GENERIC_H__
#define __VIDEO_GENERIC_H__



/***************************************************************************
    COMMON GRAPHICS LAYOUTS
***************************************************************************/

extern const gfx_layout gfx_8x8x1;
extern const gfx_layout gfx_8x8x2_planar;
extern const gfx_layout gfx_8x8x3_planar;
extern const gfx_layout gfx_8x8x4_planar;
extern const gfx_layout gfx_8x8x5_planar;
extern const gfx_layout gfx_8x8x6_planar;

extern const gfx_layout gfx_16x16x4_planar;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization ----- */

/* set up all the common systems */
void generic_video_init(running_machine &machine);



/* ----- global attributes ----- */

/* set global attributes */
void flip_screen_set(running_machine &machine, int on);
void flip_screen_set_no_update(running_machine &machine, int on);	/* will not call update_flip */
void flip_screen_x_set(running_machine &machine, int on);
void flip_screen_y_set(running_machine &machine, int on);
int flip_screen_get(running_machine &machine);
int flip_screen_x_get(running_machine &machine);
int flip_screen_y_get(running_machine &machine);

//#define flip_screen flip_screen_get(machine)



/* ----- generic palette init routines ----- */

PALETTE_INIT( all_black );
PALETTE_INIT( black_and_white );
PALETTE_INIT( monochrome_amber );
PALETTE_INIT( monochrome_green );
PALETTE_INIT( RRRR_GGGG_BBBB );
PALETTE_INIT( RRRRR_GGGGG_BBBBB );
PALETTE_INIT( BBBBB_GGGGG_RRRRR );
PALETTE_INIT( RRRRR_GGGGGG_BBBBB );


#endif	/* __VIDEO_GENERIC_H__ */
