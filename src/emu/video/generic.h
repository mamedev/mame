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



/* ----- generic palette RAM write handlers, use MRAx_RAM to read ----- */

/* 3-3-2 RGB palette write handlers */
WRITE8_HANDLER( paletteram_BBGGGRRR_w );
WRITE8_HANDLER( paletteram_RRRGGGBB_w );
WRITE8_HANDLER( paletteram_BBGGRRII_w );
WRITE8_HANDLER( paletteram_IIBBGGRR_w );

/* 4-4-4 RGB palette write handlers */
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_le_w );
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_be_w );
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split2_w );	/* uses paletteram2 */
WRITE16_HANDLER( paletteram16_xxxxBBBBGGGGRRRR_word_w );

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_le_w );
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_be_w );
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split2_w );	/* uses paletteram2 */
WRITE16_HANDLER( paletteram16_xxxxBBBBRRRRGGGG_word_w );

WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split2_w );	/* uses paletteram2 */

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_le_w );
WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_be_w );
WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_split2_w );	/* uses paletteram2 */
WRITE16_HANDLER( paletteram16_xxxxRRRRGGGGBBBB_word_w );

WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_be_w );
WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split2_w );	/* uses paletteram2 */
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBxxxx_word_w );

/* 4-4-4-4 IRGB palette write handlers */
WRITE16_HANDLER( paletteram16_IIIIRRRRGGGGBBBB_word_w );
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBIIII_word_w );

/* 5-5-5 RGB palette write handlers */
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_le_w );
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_be_w );
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split1_w );	/* uses paletteram */
WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split2_w );	/* uses paletteram2 */
WRITE16_HANDLER( paletteram16_xBBBBBGGGGGRRRRR_word_w );

WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split1_w );  /* uses paletteram */
WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split2_w );  /* uses paletteram2 */

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_le_w );
WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_be_w );
WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split1_w );
WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split2_w );
WRITE16_HANDLER( paletteram16_xRRRRRGGGGGBBBBB_word_w );

WRITE16_HANDLER( paletteram16_xGGGGGRRRRRBBBBB_word_w );
WRITE16_HANDLER( paletteram16_xGGGGGBBBBBRRRRR_word_w );

WRITE16_HANDLER( paletteram16_RRRRRGGGGGBBBBBx_word_w );
WRITE16_HANDLER( paletteram16_GGGGGRRRRRBBBBBx_word_w );
WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBRGBx_word_w );

/* 8-8-8 RGB palette write handlers */
WRITE16_HANDLER( paletteram16_xrgb_word_be_w );
WRITE16_HANDLER( paletteram16_xbgr_word_be_w );


#endif	/* __VIDEO_GENERIC_H__ */
