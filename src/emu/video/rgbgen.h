// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    rgbgen.h

    General RGB utilities.

***************************************************************************/

#ifndef __RGBGEN__
#define __RGBGEN__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* intermediate RGB values are stored in a struct */
struct rgbint { INT16 dummy, r, g, b; };

/* intermediate RGB values are stored in a struct */
struct rgbaint { INT16 a, r, g, b; };



/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_comp_to_rgbint - converts a trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_comp_to_rgbint(rgbint *rgb, INT16 r, INT16 g, INT16 b)
{
	rgb->r = r;
	rgb->g = g;
	rgb->b = b;
}


/*-------------------------------------------------
    rgba_comp_to_rgbint - converts a quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_comp_to_rgbaint(rgbaint *rgb, INT16 a, INT16 r, INT16 g, INT16 b)
{
	rgb->a = a;
	rgb->r = r;
	rgb->g = g;
	rgb->b = b;
}


/*-------------------------------------------------
    rgb_to_rgbint - converts a packed trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_to_rgbint(rgbint *rgb, rgb_t color)
{
	rgb->r = color.r();
	rgb->g = color.g();
	rgb->b = color.b();
}


/*-------------------------------------------------
    rgba_to_rgbaint - converts a packed quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_to_rgbaint(rgbaint *rgb, rgb_t color)
{
	rgb->a = color.a();
	rgb->r = color.r();
	rgb->g = color.g();
	rgb->b = color.b();
}


/*-------------------------------------------------
    rgbint_to_rgb - converts an rgbint back to
    a packed trio of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb(const rgbint *color)
{
	return rgb_t(color->r, color->g, color->b);
}


/*-------------------------------------------------
    rgbaint_to_rgba - converts an rgbint back to
    a packed quad of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba(const rgbaint *color)
{
	return rgb_t(color->a, color->r, color->g, color->b);
}


/*-------------------------------------------------
    rgbint_to_rgb_clamp - converts an rgbint back
    to a packed trio of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb_clamp(const rgbint *color)
{
	UINT8 r = (color->r < 0) ? 0 : (color->r > 255) ? 255 : color->r;
	UINT8 g = (color->g < 0) ? 0 : (color->g > 255) ? 255 : color->g;
	UINT8 b = (color->b < 0) ? 0 : (color->b > 255) ? 255 : color->b;
	return rgb_t(r, g, b);
}


/*-------------------------------------------------
    rgbaint_to_rgba_clamp - converts an rgbint back
    to a packed quad of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba_clamp(const rgbaint *color)
{
	UINT8 a = (color->a < 0) ? 0 : (color->a > 255) ? 255 : color->a;
	UINT8 r = (color->r < 0) ? 0 : (color->r > 255) ? 255 : color->r;
	UINT8 g = (color->g < 0) ? 0 : (color->g > 255) ? 255 : color->g;
	UINT8 b = (color->b < 0) ? 0 : (color->b > 255) ? 255 : color->b;
	return rgb_t(a, r, g, b);
}



/***************************************************************************
    CORE MATH
***************************************************************************/

/*-------------------------------------------------
    rgbint_add - add two rgbint values
-------------------------------------------------*/

INLINE void rgbint_add(rgbint *color1, const rgbint *color2)
{
	color1->r += color2->r;
	color1->g += color2->g;
	color1->b += color2->b;
}


/*-------------------------------------------------
    rgbaint_add - add two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_add(rgbaint *color1, const rgbaint *color2)
{
	color1->a += color2->a;
	color1->r += color2->r;
	color1->g += color2->g;
	color1->b += color2->b;
}


/*-------------------------------------------------
    rgbint_sub - subtract two rgbint values
-------------------------------------------------*/

INLINE void rgbint_sub(rgbint *color1, const rgbint *color2)
{
	color1->r -= color2->r;
	color1->g -= color2->g;
	color1->b -= color2->b;
}


/*-------------------------------------------------
    rgbaint_sub - subtract two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_sub(rgbaint *color1, const rgbaint *color2)
{
	color1->a -= color2->a;
	color1->r -= color2->r;
	color1->g -= color2->g;
	color1->b -= color2->b;
}


/*-------------------------------------------------
    rgbint_subr - reverse subtract two rgbint
    values
-------------------------------------------------*/

INLINE void rgbint_subr(rgbint *color1, const rgbint *color2)
{
	color1->r = color2->r - color1->r;
	color1->g = color2->g - color1->g;
	color1->b = color2->b - color1->b;
}


/*-------------------------------------------------
    rgbaint_subr - reverse subtract two rgbaint
    values
-------------------------------------------------*/

INLINE void rgbaint_subr(rgbaint *color1, const rgbaint *color2)
{
	color1->a = color2->a - color1->a;
	color1->r = color2->r - color1->r;
	color1->g = color2->g - color1->g;
	color1->b = color2->b - color1->b;
}


/*-------------------------------------------------
    rgbint_shl - shift each component of an
    rgbint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbint_shl(rgbint *color, UINT8 shift)
{
	color->r <<= shift;
	color->g <<= shift;
	color->b <<= shift;
}


/*-------------------------------------------------
    rgbaint_shl - shift each component of an
    rgbaint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbaint_shl(rgbaint *color, UINT8 shift)
{
	color->r <<= shift;
	color->g <<= shift;
	color->b <<= shift;
	color->a <<= shift;
}


/*-------------------------------------------------
    rgbint_shr - shift each component of an
    rgbint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbint_shr(rgbint *color, UINT8 shift)
{
	color->r >>= shift;
	color->g >>= shift;
	color->b >>= shift;
}


/*-------------------------------------------------
    rgbaint_shr - shift each component of an
    rgbaint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbaint_shr(rgbaint *color, UINT8 shift)
{
	color->r >>= shift;
	color->g >>= shift;
	color->b >>= shift;
	color->a >>= shift;
}



/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbint_blend(rgbint *color1, const rgbint *color2, UINT8 color1scale)
{
	int scale1 = (int)color1scale;
	int scale2 = 256 - scale1;

	color1->r = (color1->r * scale1 + color2->r * scale2) >> 8;
	color1->g = (color1->g * scale1 + color2->g * scale2) >> 8;
	color1->b = (color1->b * scale1 + color2->b * scale2) >> 8;
}


/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbaint_blend(rgbaint *color1, const rgbaint *color2, UINT8 color1scale)
{
	int scale1 = (int)color1scale;
	int scale2 = 256 - scale1;

	color1->a = (color1->a * scale1 + color2->a * scale2) >> 8;
	color1->r = (color1->r * scale1 + color2->r * scale2) >> 8;
	color1->g = (color1->g * scale1 + color2->g * scale2) >> 8;
	color1->b = (color1->b * scale1 + color2->b * scale2) >> 8;
}


/*-------------------------------------------------
    rgbint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbint_scale_immediate_and_clamp(rgbint *color, INT16 colorscale)
{
	color->r = (color->r * colorscale) >> 8;
	if ((UINT16)color->r > 255) { color->r = (color->r < 0) ? 0 : 255; }
	color->g = (color->g * colorscale) >> 8;
	if ((UINT16)color->g > 255) { color->g = (color->g < 0) ? 0 : 255; }
	color->b = (color->b * colorscale) >> 8;
	if ((UINT16)color->b > 255) { color->b = (color->b < 0) ? 0 : 255; }
}

INLINE void rgbint_scale_channel_and_clamp(rgbint *color, const rgbint *colorscale)
{
	color->r = (color->r * colorscale->r) >> 8;
	if ((UINT16)color->r > 255) { color->r = (color->r < 0) ? 0 : 255; }
	color->g = (color->g * colorscale->g) >> 8;
	if ((UINT16)color->g > 255) { color->g = (color->g < 0) ? 0 : 255; }
	color->b = (color->b * colorscale->b) >> 8;
	if ((UINT16)color->b > 255) { color->b = (color->b < 0) ? 0 : 255; }
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbaint_scale_immediate_and_clamp(rgbaint *color, INT16 colorscale)
{
	color->a = (color->a * colorscale) >> 8;
	if ((UINT16)color->a > 255) { color->a = (color->a < 0) ? 0 : 255; }
	color->r = (color->r * colorscale) >> 8;
	if ((UINT16)color->r > 255) { color->r = (color->r < 0) ? 0 : 255; }
	color->g = (color->g * colorscale) >> 8;
	if ((UINT16)color->g > 255) { color->g = (color->g < 0) ? 0 : 255; }
	color->b = (color->b * colorscale) >> 8;
	if ((UINT16)color->b > 255) { color->b = (color->b < 0) ? 0 : 255; }
}

INLINE void rgbaint_scale_channel_and_clamp(rgbaint *color, const rgbaint *colorscale)
{
	color->a = (color->a * colorscale->a) >> 8;
	if ((UINT16)color->a > 255) { color->a = (color->a < 0) ? 0 : 255; }
	color->r = (color->r * colorscale->r) >> 8;
	if ((UINT16)color->r > 255) { color->r = (color->r < 0) ? 0 : 255; }
	color->g = (color->g * colorscale->g) >> 8;
	if ((UINT16)color->g > 255) { color->g = (color->g < 0) ? 0 : 255; }
	color->b = (color->b * colorscale->b) >> 8;
	if ((UINT16)color->b > 255) { color->b = (color->b < 0) ? 0 : 255; }
}


/*-------------------------------------------------
    rgb_bilinear_filter - bilinear filter between
    four pixel values; this code is derived from
    code provided by Michael Herf
-------------------------------------------------*/

INLINE UINT32 rgb_bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	UINT32 ag0, ag1, rb0, rb1;

	rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
	ag0 = (rgb00 & 0x0000ff00) + ((((rgb01 & 0x0000ff00) - (rgb00 & 0x0000ff00)) * u) >> 8);
	ag1 = (rgb10 & 0x0000ff00) + ((((rgb11 & 0x0000ff00) - (rgb10 & 0x0000ff00)) * u) >> 8);

	rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
	ag0 = (ag0 & 0x0000ff00) + ((((ag1 & 0x0000ff00) - (ag0 & 0x0000ff00)) * v) >> 8);

	return (ag0 & 0x0000ff00) | (rb0 & 0x00ff00ff);
}


/*-------------------------------------------------
    rgba_bilinear_filter - bilinear filter between
    four pixel values; this code is derived from
    code provided by Michael Herf
-------------------------------------------------*/

INLINE UINT32 rgba_bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	UINT32 ag0, ag1, rb0, rb1;

	rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
	rgb00 = rgb00 >> 8;
	rgb01 = rgb01 >> 8;
	rgb10 = rgb10 >> 8;
	rgb11 = rgb11 >> 8;
	ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

	rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
	ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

	return ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
}


/*-------------------------------------------------
    rgbint_bilinear_filter - bilinear filter between
    four pixel values; this code is derived from
    code provided by Michael Herf
-------------------------------------------------*/

INLINE void rgbint_bilinear_filter(rgbint *color, UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	UINT32 ag0, ag1, rb0, rb1;

	rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
	ag0 = (rgb00 & 0x0000ff00) + ((((rgb01 & 0x0000ff00) - (rgb00 & 0x0000ff00)) * u) >> 8);
	ag1 = (rgb10 & 0x0000ff00) + ((((rgb11 & 0x0000ff00) - (rgb10 & 0x0000ff00)) * u) >> 8);

	rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
	ag0 = (ag0 & 0x0000ff00) + ((((ag1 & 0x0000ff00) - (ag0 & 0x0000ff00)) * v) >> 8);

	color->r = rb0 >> 16;
	color->g = ag0 >> 8;
	color->b = rb0;
}


/*-------------------------------------------------
    rgbaint_bilinear_filter - bilinear filter between
    four pixel values; this code is derived from
    code provided by Michael Herf
-------------------------------------------------*/

INLINE void rgbaint_bilinear_filter(rgbaint *color, UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	UINT32 ag0, ag1, rb0, rb1;

	rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
	rgb00 = rgb00 >> 8;
	rgb01 = rgb01 >> 8;
	rgb10 = rgb10 >> 8;
	rgb11 = rgb11 >> 8;
	ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

	rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
	ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

	color->a = ag0 >> 16;
	color->r = rb0 >> 16;
	color->g = ag0;
	color->b = rb0;
}


#endif /* __RGBUTIL__ */
