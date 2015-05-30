// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    rgbsse.h

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#ifndef __RGBSSE__
#define __RGBSSE__

#include <emmintrin.h>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* intermediate RGB values are stored in an __m128i */
typedef __m128i rgbint;

/* intermediate RGB values are stored in an __m128i */
typedef __m128i rgbaint;



/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_comp_to_rgbint - converts a trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_comp_to_rgbint(rgbint *rgb, INT16 r, INT16 g, INT16 b)
{
	*rgb = _mm_set_epi16(0, 0, 0, 0, 0, r, g, b);
}


/*-------------------------------------------------
    rgba_comp_to_rgbint - converts a quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_comp_to_rgbaint(rgbaint *rgb, INT16 a, INT16 r, INT16 g, INT16 b)
{
	*rgb = _mm_set_epi16(0, 0, 0, 0, a, r, g, b);
}


/*-------------------------------------------------
    rgb_to_rgbint - converts a packed trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_to_rgbint(rgbint *rgb, rgb_t color)
{
	*rgb = _mm_unpacklo_epi8(_mm_cvtsi32_si128(color), _mm_setzero_si128());
}


/*-------------------------------------------------
    rgba_to_rgbaint - converts a packed quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_to_rgbaint(rgbaint *rgb, rgb_t color)
{
	*rgb = _mm_unpacklo_epi8(_mm_cvtsi32_si128(color), _mm_setzero_si128());
}


/*-------------------------------------------------
    rgbint_to_rgb - converts an rgbint back to
    a packed trio of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb(const rgbint *color)
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(*color, *color));
}


/*-------------------------------------------------
    rgbaint_to_rgba - converts an rgbint back to
    a packed quad of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba(const rgbaint *color)
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(*color, *color));
}


/*-------------------------------------------------
    rgbint_to_rgb_clamp - converts an rgbint back
    to a packed trio of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb_clamp(const rgbint *color)
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(*color, *color));
}


/*-------------------------------------------------
    rgbaint_to_rgba_clamp - converts an rgbint back
    to a packed quad of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba_clamp(const rgbaint *color)
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(*color, *color));
}



/***************************************************************************
    CORE MATH
***************************************************************************/

/*-------------------------------------------------
    rgbint_add - add two rgbint values
-------------------------------------------------*/

INLINE void rgbint_add(rgbint *color1, const rgbint *color2)
{
	*color1 = _mm_add_epi16(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_add - add two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_add(rgbaint *color1, const rgbaint *color2)
{
	*color1 = _mm_add_epi16(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_sub - subtract two rgbint values
-------------------------------------------------*/

INLINE void rgbint_sub(rgbint *color1, const rgbint *color2)
{
	*color1 = _mm_sub_epi16(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_sub - subtract two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_sub(rgbaint *color1, const rgbaint *color2)
{
	*color1 = _mm_sub_epi16(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_subr - reverse subtract two rgbint
    values
-------------------------------------------------*/

INLINE void rgbint_subr(rgbint *color1, const rgbint *color2)
{
	__m128i temp = *color1;
	*color1 = *color2;
	*color1 = _mm_sub_epi16(*color1, temp);
}


/*-------------------------------------------------
    rgbaint_subr - reverse subtract two rgbaint
    values
-------------------------------------------------*/

INLINE void rgbaint_subr(rgbaint *color1, const rgbaint *color2)
{
	__m128i temp = *color1;
	*color1 = *color2;
	*color1 = _mm_sub_epi16(*color1, temp);
}


/*-------------------------------------------------
    rgbint_shl - shift each component of an
    rgbint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbint_shl(rgbint *color, UINT8 shift)
{
	*color = _mm_slli_epi16(*color, shift);
}


/*-------------------------------------------------
    rgbaint_shl - shift each component of an
    rgbaint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbaint_shl(rgbaint *color, UINT8 shift)
{
	*color = _mm_slli_epi16(*color, shift);
}


/*-------------------------------------------------
    rgbint_shr - shift each component of an
    rgbint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbint_shr(rgbint *color, UINT8 shift)
{
	*color = _mm_srli_epi16(*color, shift);
}


/*-------------------------------------------------
    rgbaint_shr - shift each component of an
    rgbaint struct by the given number of bits
-------------------------------------------------*/

INLINE void rgbaint_shr(rgbaint *color, UINT8 shift)
{
	*color = _mm_srli_epi16(*color, shift);
}



/***************************************************************************
    TABLES
***************************************************************************/

extern const struct _rgbsse_statics
{
	__m128  dummy_for_alignment;
	INT16   maxbyte[8];
	INT16   scale_table[256][8];
} rgbsse_statics;



/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbint_blend(rgbint *color1, const rgbint *color2, UINT8 color1scale)
{
	*color1 = _mm_unpacklo_epi16(*color1, *color2);
	*color1 = _mm_madd_epi16(*color1, *(__m128i *)&rgbsse_statics.scale_table[color1scale][0]);
	*color1 = _mm_srli_epi32(*color1, 8);
	*color1 = _mm_packs_epi32(*color1, *color1);
}


/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbaint_blend(rgbaint *color1, const rgbaint *color2, UINT8 color1scale)
{
	rgbint_blend(color1, color2, color1scale);
}


/*-------------------------------------------------
    rgbint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbint_scale_immediate_and_clamp(rgbint *color, INT16 colorscale)
{
	__m128i mscale = _mm_set1_epi16(colorscale);
	*color = _mm_unpacklo_epi16(*color, _mm_setzero_si128());
	*color = _mm_madd_epi16(*color, mscale);
	*color = _mm_srli_epi32(*color, 8);
	*color = _mm_packs_epi32(*color, *color);
	*color = _mm_min_epi16(*color, *(__m128i *)&rgbsse_statics.maxbyte);
}

INLINE void rgbint_scale_channel_and_clamp(rgbint *color, const rgbint *colorscale)
{
	__m128i mscale = _mm_unpacklo_epi16(*colorscale, _mm_setzero_si128());
	*color = _mm_unpacklo_epi16(*color, _mm_setzero_si128());
	*color = _mm_madd_epi16(*color, mscale);
	*color = _mm_srli_epi32(*color, 8);
	*color = _mm_packs_epi32(*color, *color);
	*color = _mm_min_epi16(*color, *(__m128i *)&rgbsse_statics.maxbyte);
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbaint_scale_immediate_and_clamp(rgbaint *color, INT16 colorscale)
{
	rgbint_scale_immediate_and_clamp(color, colorscale);
}

INLINE void rgbaint_scale_channel_and_clamp(rgbaint *color, const rgbint *colorscale)
{
	rgbint_scale_channel_and_clamp(color, color);
}


/*-------------------------------------------------
    rgb_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE UINT32 rgb_bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	__m128i color00 = _mm_cvtsi32_si128(rgb00);
	__m128i color01 = _mm_cvtsi32_si128(rgb01);
	__m128i color10 = _mm_cvtsi32_si128(rgb10);
	__m128i color11 = _mm_cvtsi32_si128(rgb11);

	/* interleave color01 and color00 at the byte level */
	color01 = _mm_unpacklo_epi8(color01, color00);
	color11 = _mm_unpacklo_epi8(color11, color10);
	color01 = _mm_unpacklo_epi8(color01, _mm_setzero_si128());
	color11 = _mm_unpacklo_epi8(color11, _mm_setzero_si128());
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color11 = _mm_madd_epi16(color11, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color01 = _mm_slli_epi32(color01, 15);
	color11 = _mm_srli_epi32(color11, 1);
	color01 = _mm_max_epi16(color01, color11);
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[v][0]);
	color01 = _mm_srli_epi32(color01, 15);
	color01 = _mm_packs_epi32(color01, color01);
	color01 = _mm_packus_epi16(color01, color01);
	return _mm_cvtsi128_si32(color01);
}


/*-------------------------------------------------
    rgba_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE UINT32 rgba_bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	return rgb_bilinear_filter(rgb00, rgb01, rgb10, rgb11, u, v);
}


/*-------------------------------------------------
    rgbint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbint_bilinear_filter(rgbint *color, UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	__m128i color00 = _mm_cvtsi32_si128(rgb00);
	__m128i color01 = _mm_cvtsi32_si128(rgb01);
	__m128i color10 = _mm_cvtsi32_si128(rgb10);
	__m128i color11 = _mm_cvtsi32_si128(rgb11);

	/* interleave color01 and color00 at the byte level */
	color01 = _mm_unpacklo_epi8(color01, color00);
	color11 = _mm_unpacklo_epi8(color11, color10);
	color01 = _mm_unpacklo_epi8(color01, _mm_setzero_si128());
	color11 = _mm_unpacklo_epi8(color11, _mm_setzero_si128());
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color11 = _mm_madd_epi16(color11, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color01 = _mm_slli_epi32(color01, 15);
	color11 = _mm_srli_epi32(color11, 1);
	color01 = _mm_max_epi16(color01, color11);
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[v][0]);
	color01 = _mm_srli_epi32(color01, 15);
	*color = _mm_packs_epi32(color01, color01);
}


/*-------------------------------------------------
    rgbaint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbaint_bilinear_filter(rgbaint *color, UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	rgbint_bilinear_filter(color, rgb00, rgb01, rgb10, rgb11, u, v);
}


#endif /* __RGBSSE__ */
