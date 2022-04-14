// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendutil.h

    Core rendering utilities.

***************************************************************************/

#ifndef MAME_EMU_RENDUTIL_H
#define MAME_EMU_RENDUTIL_H

#pragma once

#include "rendertypes.h"

#include <algorithm>
#include <cmath>
#include <utility>


/* ----- image formats ----- */

enum ru_imgformat
{
	RENDUTIL_IMGFORMAT_PNG,
	RENDUTIL_IMGFORMAT_JPEG,
	RENDUTIL_IMGFORMAT_MSDIB,

	RENDUTIL_IMGFORMAT_UNKNOWN,
	RENDUTIL_IMGFORMAT_ERROR
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- render utilities ----- */

void render_resample_argb_bitmap_hq(bitmap_argb32 &dest, bitmap_argb32 &source, const render_color &color, bool force = false);
bool render_clip_line(render_bounds &bounds, const render_bounds &clip);
bool render_clip_quad(render_bounds &bounds, const render_bounds &clip, render_quad_texuv *texcoords);
std::pair<render_bounds, render_bounds> render_line_to_quad(const render_bounds &bounds, float width, float length_extension);
void render_load_msdib(bitmap_argb32 &bitmap, util::random_read &file);
void render_load_jpeg(bitmap_argb32 &bitmap, util::random_read &file);
bool render_load_png(bitmap_argb32 &bitmap, util::random_read &file, bool load_as_alpha_to_existing = false);
ru_imgformat render_detect_image(util::random_read &file);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    render_round_nearest - floating point
    round-to-nearest
-------------------------------------------------*/

static inline float render_round_nearest(float f)
{
	return floorf(f + 0.5f);
}


/*-------------------------------------------------
    orientation_swap_flips - swap the X and Y
    flip flags
-------------------------------------------------*/

constexpr int orientation_swap_flips(int orientation)
{
	return (orientation & ORIENTATION_SWAP_XY) |
			((orientation & ORIENTATION_FLIP_X) ? ORIENTATION_FLIP_Y : 0) |
			((orientation & ORIENTATION_FLIP_Y) ? ORIENTATION_FLIP_X : 0);
}


/*-------------------------------------------------
    orientation_reverse - compute the orientation
    that will undo another orientation
-------------------------------------------------*/

constexpr int orientation_reverse(int orientation)
{
	/* if not swapping X/Y, then just apply the same transform to reverse */
	if (!(orientation & ORIENTATION_SWAP_XY))
		return orientation;

	/* if swapping X/Y, then swap X/Y flip bits to get the reverse */
	else
		return orientation_swap_flips(orientation);
}


/*-------------------------------------------------
    orientation_add - compute effective orientation
    after applying two subsequent orientations
-------------------------------------------------*/

constexpr int orientation_add(int orientation1, int orientation2)
{
	/* if the 2nd transform doesn't swap, just XOR together */
	if (!(orientation2 & ORIENTATION_SWAP_XY))
		return orientation1 ^ orientation2;

	/* otherwise, we need to effectively swap the flip bits on the first transform */
	else
		return orientation_swap_flips(orientation1) ^ orientation2;
}


/*-------------------------------------------------
    apply_brightness_contrast_gamma_fp - apply
    brightness, contrast, and gamma controls to
    a single RGB component
-------------------------------------------------*/

static inline float apply_brightness_contrast_gamma_fp(float srcval, float brightness, float contrast, float gamma)
{
	/* first apply gamma */
	srcval = pow(srcval, 1.0f / gamma);

	/* then contrast/brightness */
	srcval = (srcval * contrast) + brightness - 1.0f;

	/* clamp and return */
	return std::clamp(srcval, 0.0f, 1.0f);
}


/*-------------------------------------------------
    apply_brightness_contrast_gamma - apply
    brightness, contrast, and gamma controls to
    a single RGB component
-------------------------------------------------*/

static inline u8 apply_brightness_contrast_gamma(u8 src, float brightness, float contrast, float gamma)
{
	float srcval = (float)src * (1.0f / 255.0f);
	float result = apply_brightness_contrast_gamma_fp(srcval, brightness, contrast, gamma);
	return u8(result * 255.0f);
}

#endif // MAME_EMU_RENDUTIL_H
