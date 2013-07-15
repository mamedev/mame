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

extern const rgb_t RGB_MONOCHROME_WHITE[];
extern const rgb_t RGB_MONOCHROME_WHITE_HIGHLIGHT[];
extern const rgb_t RGB_MONOCHROME_AMBER[];
extern const rgb_t RGB_MONOCHROME_GREEN[];
extern const rgb_t RGB_MONOCHROME_GREEN_HIGHLIGHT[];
extern const rgb_t RGB_MONOCHROME_YELLOW[];

#endif  /* __VIDEO_GENERIC_H__ */
