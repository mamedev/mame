// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendutil.h

    Core rendering utilities.
***************************************************************************/

#ifndef __RENDUTIL_H__
#define __RENDUTIL_H__

#include "render.h"

#include <math.h>


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- render utilities ----- */

void render_resample_argb_bitmap_hq(bitmap_argb32 &dest, bitmap_argb32 &source, const render_color &color, bool force = false);
int render_clip_line(render_bounds *bounds, const render_bounds *clip);
int render_clip_quad(render_bounds *bounds, const render_bounds *clip, render_quad_texuv *texcoords);
void render_line_to_quad(const render_bounds *bounds, float width, render_bounds *bounds0, render_bounds *bounds1);
bool render_load_png(bitmap_argb32 &bitmap, emu_file &file, const char *dirname, const char *filename, bool load_as_alpha_to_existing = false);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    render_round_nearest - floating point
    round-to-nearest
-------------------------------------------------*/

static inline float render_round_nearest(float f)
{
	return floor(f + 0.5f);
}


/*-------------------------------------------------
    set_render_bounds_xy - cleaner way to set the
    bounds
-------------------------------------------------*/

static inline void set_render_bounds_xy(render_bounds *bounds, float x0, float y0, float x1, float y1)
{
	bounds->x0 = x0;
	bounds->y0 = y0;
	bounds->x1 = x1;
	bounds->y1 = y1;
}


/*-------------------------------------------------
    set_render_bounds_wh - cleaner way to set the
    bounds
-------------------------------------------------*/

static inline void set_render_bounds_wh(render_bounds *bounds, float x0, float y0, float width, float height)
{
	bounds->x0 = x0;
	bounds->y0 = y0;
	bounds->x1 = x0 + width;
	bounds->y1 = y0 + height;
}


/*-------------------------------------------------
    sect_render_bounds - compute the intersection
    of two render_bounds
-------------------------------------------------*/

static inline void sect_render_bounds(render_bounds *dest, const render_bounds *src)
{
	dest->x0 = (dest->x0 > src->x0) ? dest->x0 : src->x0;
	dest->x1 = (dest->x1 < src->x1) ? dest->x1 : src->x1;
	dest->y0 = (dest->y0 > src->y0) ? dest->y0 : src->y0;
	dest->y1 = (dest->y1 < src->y1) ? dest->y1 : src->y1;
}


/*-------------------------------------------------
    union_render_bounds - compute the union of two
    render_bounds
-------------------------------------------------*/

static inline void union_render_bounds(render_bounds *dest, const render_bounds *src)
{
	dest->x0 = (dest->x0 < src->x0) ? dest->x0 : src->x0;
	dest->x1 = (dest->x1 > src->x1) ? dest->x1 : src->x1;
	dest->y0 = (dest->y0 < src->y0) ? dest->y0 : src->y0;
	dest->y1 = (dest->y1 > src->y1) ? dest->y1 : src->y1;
}


/*-------------------------------------------------
    set_render_color - cleaner way to set a color
-------------------------------------------------*/

static inline void set_render_color(render_color *color, float a, float r, float g, float b)
{
	color->a = a;
	color->r = r;
	color->g = g;
	color->b = b;
}


/*-------------------------------------------------
    orientation_swap_flips - swap the X and Y
    flip flags
-------------------------------------------------*/

static inline int orientation_swap_flips(int orientation)
{
	return (orientation & ORIENTATION_SWAP_XY) |
			((orientation & ORIENTATION_FLIP_X) ? ORIENTATION_FLIP_Y : 0) |
			((orientation & ORIENTATION_FLIP_Y) ? ORIENTATION_FLIP_X : 0);
}


/*-------------------------------------------------
    orientation_reverse - compute the orientation
    that will undo another orientation
-------------------------------------------------*/

static inline int orientation_reverse(int orientation)
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

static inline int orientation_add(int orientation1, int orientation2)
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
	if (srcval < 0.0f)
		srcval = 0.0f;
	if (srcval > 1.0f)
		srcval = 1.0f;
	return srcval;
}


/*-------------------------------------------------
    apply_brightness_contrast_gamma - apply
    brightness, contrast, and gamma controls to
    a single RGB component
-------------------------------------------------*/

static inline UINT8 apply_brightness_contrast_gamma(UINT8 src, float brightness, float contrast, float gamma)
{
	float srcval = (float)src * (1.0f / 255.0f);
	float result = apply_brightness_contrast_gamma_fp(srcval, brightness, contrast, gamma);
	return (UINT8)(result * 255.0f);
}


#endif  /* __RENDUTIL_H__ */
