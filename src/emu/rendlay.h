// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendlay.h

    Core rendering layout parser and manager.

***************************************************************************/

#ifndef __RENDLAY_H__
#define __RENDLAY_H__

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// no screens layouts
extern const internal_layout layout_noscreens;   // for screenless systems

// single screen layouts
extern const internal_layout layout_horizont;    // horizontal 4:3 screens
extern const internal_layout layout_vertical;    // vertical 4:3 screens

// dual screen layouts
extern const internal_layout layout_dualhsxs;    // dual 4:3 screens side-by-side
extern const internal_layout layout_dualhovu;    // dual 4:3 screens above and below
extern const internal_layout layout_dualhuov;    // dual 4:3 screens below and above

// triple screen layouts
extern const internal_layout layout_triphsxs;    // triple 4:3 screens side-by-side

// quad screen layouts
extern const internal_layout layout_quadhsxs;    // quad 4:3 screens side-by-side

// LCD screen layouts
extern const internal_layout layout_lcd;         // generic 1:1 lcd screen layout
extern const internal_layout layout_lcd_rot;     // same, for use with ROT90 or ROT270

// SVG screen layouts
extern const internal_layout layout_svg;         // generic 1:1 lcd screen layout


#endif  // __RENDLAY_H__
