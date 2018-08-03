// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendlay.h

    Core rendering layout parser and manager.

***************************************************************************/

#ifndef MAME_EMU_RENDLAY_H
#define MAME_EMU_RENDLAY_H

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// no screens layouts
extern const internal_layout layout_noscreens;   // for screenless systems

// dual screen layouts
extern const internal_layout layout_dualhsxs;    // dual 4:3 screens side-by-side
extern const internal_layout layout_dualhovu;    // dual 4:3 screens above and below
extern const internal_layout layout_dualhuov;    // dual 4:3 screens below and above

// triple screen layouts
extern const internal_layout layout_triphsxs;    // triple 4:3 screens side-by-side

// quad screen layouts
extern const internal_layout layout_quadhsxs;    // quad 4:3 screens side-by-side

#endif // MAME_EMU_RENDLAY_H
