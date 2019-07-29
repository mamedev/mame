// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    generic.h

    Generic simple video functions.

*********************************************************************/

#ifndef MAME_EMU_VIDEO_GENERIC_H
#define MAME_EMU_VIDEO_GENERIC_H

#pragma once



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

extern const gfx_layout gfx_8x8x4_packed_msb;
extern const gfx_layout gfx_8x8x4_packed_lsb;
extern const gfx_layout gfx_8x8x8_raw;

extern const gfx_layout gfx_16x16x4_packed_msb;
extern const gfx_layout gfx_16x16x4_packed_lsb;
extern const gfx_layout gfx_16x16x8_raw;

#endif  /* MAME_EMU_VIDEO_GENERIC_H */
