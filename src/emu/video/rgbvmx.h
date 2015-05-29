// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    rgbvmx.h

    VMX/Altivec optimised RGB utilities.

***************************************************************************/

#ifndef __RGBVMX__
#define __RGBVMX__

#if defined(__ALTIVEC__)
#include <altivec.h>
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* intermediate RGB values are stored in a vector */
typedef vector signed short rgbint;

/* intermediate RGB values are stored in a vector */
typedef vector signed short rgbaint;



/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

/*-------------------------------------------------
    rgb_comp_to_rgbint - converts a trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_comp_to_rgbint(rgbint *rgb, INT16 r, INT16 g, INT16 b)
{
	rgbint result = { 0, r, g, b, 0, 0, 0, 0 };
	*rgb = result;
}


/*-------------------------------------------------
    rgba_comp_to_rgbint - converts a quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_comp_to_rgbaint(rgbaint *rgb, INT16 a, INT16 r, INT16 g, INT16 b)
{
	rgbaint result = { a, r, g, b, 0, 0, 0, 0 };
	*rgb = result;
}


/*-------------------------------------------------
    rgb_to_rgbint - converts a packed trio of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgb_to_rgbint(rgbint *rgb, rgb_t const &color)
{
	vector signed char temp = (vector signed char)vec_perm((vector signed int)vec_lde(0, color.ptr()), vec_splat_s32(0), vec_lvsl(0, color.ptr()));
	*rgb = (rgbint)vec_mergeh((vector signed char)vec_splat_s32(0), temp);
}


/*-------------------------------------------------
    rgba_to_rgbaint - converts a packed quad of RGB
    components to an rgbint type
-------------------------------------------------*/

INLINE void rgba_to_rgbaint(rgbaint *rgb, rgb_t const &color)
{
	vector signed char temp = (vector signed char)vec_perm((vector signed int)vec_lde(0, color.ptr()), vec_splat_s32(0), vec_lvsl(0, color.ptr()));
	*rgb = (rgbaint)vec_mergeh((vector signed char)vec_splat_s32(0), temp);
}


/*-------------------------------------------------
    rgbint_to_rgb - converts an rgbint back to
    a packed trio of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb(const rgbint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	UINT32 result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbaint_to_rgba - converts an rgbint back to
    a packed quad of RGB values
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba(const rgbaint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	UINT32 result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbint_to_rgb_clamp - converts an rgbint back
    to a packed trio of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbint_to_rgb_clamp(const rgbint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	UINT32 result;
	vec_ste(temp, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbaint_to_rgba_clamp - converts an rgbint back
    to a packed quad of RGB values, clamping them
    to bytes first
-------------------------------------------------*/

INLINE rgb_t rgbaint_to_rgba_clamp(const rgbaint *color)
{
	vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(*color, *color), 0);
	UINT32 result;
	vec_ste(temp, 0, &result);
	return result;
}



/***************************************************************************
    CORE MATH
***************************************************************************/

/*-------------------------------------------------
    rgbint_add - add two rgbint values
-------------------------------------------------*/

INLINE void rgbint_add(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_add(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_add - add two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_add(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_add(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_sub - subtract two rgbint values
-------------------------------------------------*/

INLINE void rgbint_sub(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_sub(*color1, *color2);
}


/*-------------------------------------------------
    rgbaint_sub - subtract two rgbaint values
-------------------------------------------------*/

INLINE void rgbaint_sub(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_sub(*color1, *color2);
}


/*-------------------------------------------------
    rgbint_subr - reverse subtract two rgbint
    values
-------------------------------------------------*/

INLINE void rgbint_subr(rgbint *color1, const rgbint *color2)
{
	*color1 = vec_sub(*color2, *color1);
}


/*-------------------------------------------------
    rgbaint_subr - reverse subtract two rgbaint
    values
-------------------------------------------------*/

INLINE void rgbaint_subr(rgbaint *color1, const rgbaint *color2)
{
	*color1 = vec_sub(*color2, *color1);
}



/***************************************************************************
    TABLES
***************************************************************************/

extern const struct _rgbvmx_statics
{
	rgbaint maxbyte;
	rgbaint scale_table[256];
} rgbvmx_statics;



/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbint_blend(rgbint *color1, const rgbint *color2, UINT8 color1scale)
{
	vector signed int temp;
	*color1 = vec_mergeh(*color1, *color2);
	temp = vec_msum(*color1, rgbvmx_statics.scale_table[color1scale], vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color1 = vec_packs(temp, temp);
}


/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

INLINE void rgbaint_blend(rgbaint *color1, const rgbaint *color2, UINT8 color1scale)
{
	vector signed int temp;
	*color1 = vec_mergeh(*color1, *color2);
	temp = vec_msum(*color1, rgbvmx_statics.scale_table[color1scale], vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color1 = vec_packs(temp, temp);
}


/*-------------------------------------------------
    rgbint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbint_scale_immediate_and_clamp(rgbint *color, INT16 colorscale)
{
	rgbint splatmap = vec_splat((rgbint)vec_lvsl(0, &colorscale), 0);
	rgbint vecscale = vec_lde(0, &colorscale);
	vector signed int temp;
	vecscale = (rgbint)vec_perm(vecscale, vecscale, (vector unsigned char)splatmap);
	*color = (rgbint)vec_mergeh(*color, (rgbint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}

INLINE void rgbint_scale_channel_and_clamp(rgbint *color, const rgbint *colorscale)
{
	rgbint vecscale = (rgbint)vec_mergeh(*colorscale, (rgbint)vec_splat_s32(0));
	vector signed int temp;
	*color = (rgbint)vec_mergeh(*color, (rgbint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

INLINE void rgbaint_scale_immediate_and_clamp(rgbaint *color, INT16 colorscale)
{
	rgbaint splatmap = vec_splat((rgbaint)vec_lvsl(0, &colorscale), 0);
	rgbaint vecscale = vec_lde(0, &colorscale);
	vector signed int temp;
	vecscale = (rgbaint)vec_perm(vecscale, vecscale, (vector unsigned char)splatmap);
	*color = (rgbaint)vec_mergeh(*color, (rgbaint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}

INLINE void rgbaint_scale_channel_and_clamp(rgbaint *color, const rgbint *colorscale)
{
	rgbaint vecscale = (rgbaint)vec_mergeh(*color, (rgbaint)vec_splat_s32(0));
	vector signed int temp;
	*color = (rgbaint)vec_mergeh(*color, (rgbaint)vec_splat_s32(0));
	temp = vec_msum(*color, vecscale, vec_splat_s32(0));
	temp = (vector signed int)vec_sr(temp, vec_splat_u32(8));
	*color = vec_min(vec_packs(temp, temp), rgbvmx_statics.maxbyte);
}


/*-------------------------------------------------
    rgb_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE rgb_t rgb_bilinear_filter(rgb_t const &rgb00, rgb_t const &rgb01, rgb_t const &rgb10, rgb_t const &rgb11, UINT8 u, UINT8 v)
{
	rgbint  color00 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb00.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb00.ptr()));
	rgbint  color01 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb01.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb01.ptr()));
	rgbint  color10 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb10.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb10.ptr()));
	rgbint  color11 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb11.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb11.ptr()));

	/* interleave color01 and color00 at the byte level */
	color01 = (rgbint)vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = (rgbint)vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = (rgbint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = (rgbint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = (rgbint)vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = (rgbint)vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = (rgbint)vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	color01 = vec_packs((vector signed int)color01, (vector signed int)color01);
	color01 = (rgbint)vec_packsu(color01, color01);

	UINT32 result;
	vec_ste((vector unsigned int)color01, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgba_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE rgb_t rgba_bilinear_filter(rgb_t const &rgb00, rgb_t const &rgb01, rgb_t const &rgb10, rgb_t const &rgb11, UINT8 u, UINT8 v)
{
	rgbaint color00 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb00.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb00.ptr()));
	rgbaint color01 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb01.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb01.ptr()));
	rgbaint color10 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb10.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb10.ptr()));
	rgbaint color11 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb11.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb11.ptr()));

	/* interleave color01 and color00 at the byte level */
	color01 = (rgbaint)vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = (rgbaint)vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = (rgbaint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = (rgbaint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = (rgbaint)vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = (rgbaint)vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbaint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = (rgbaint)vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	color01 = vec_packs((vector signed int)color01, (vector signed int)color01);
	color01 = (rgbaint)vec_packsu(color01, color01);

	UINT32 result;
	vec_ste((vector unsigned int)color01, 0, &result);
	return result;
}


/*-------------------------------------------------
    rgbint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbint_bilinear_filter(rgbint *color, rgb_t const &rgb00, rgb_t const &rgb01, rgb_t const &rgb10, rgb_t const &rgb11, UINT8 u, UINT8 v)
{
	rgbint color00 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb00.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb00.ptr()));
	rgbint color01 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb01.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb01.ptr()));
	rgbint color10 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb10.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb10.ptr()));
	rgbint color11 = (rgbint)vec_perm((vector signed int)vec_lde(0, rgb11.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb11.ptr()));

	/* interleave color01 and color00 at the byte level */
	color01 = (rgbint)vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = (rgbint)vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = (rgbint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = (rgbint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = (rgbint)vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = (rgbint)vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = (rgbint)vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	*color = vec_packs((vector signed int)color01, (vector signed int)color01);
}


/*-------------------------------------------------
    rgbaint_bilinear_filter - bilinear filter between
    four pixel values
-------------------------------------------------*/

INLINE void rgbaint_bilinear_filter(rgbaint *color, rgb_t const &rgb00, rgb_t const &rgb01, rgb_t const &rgb10, rgb_t const &rgb11, UINT8 u, UINT8 v)
{
	rgbaint color00 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb00.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb00.ptr()));
	rgbaint color01 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb01.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb01.ptr()));
	rgbaint color10 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb10.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb10.ptr()));
	rgbaint color11 = (rgbaint)vec_perm((vector signed int)vec_lde(0, rgb11.ptr()), vec_splat_s32(0), vec_lvsl(0, rgb11.ptr()));

	/* interleave color01 and color00 at the byte level */
	color01 = (rgbaint)vec_mergeh((vector signed char)color01, (vector signed char)color00);
	color11 = (rgbaint)vec_mergeh((vector signed char)color11, (vector signed char)color10);
	color01 = (rgbaint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color01);
	color11 = (rgbaint)vec_mergeh((vector signed char)vec_splat_s32(0), (vector signed char)color11);
	color01 = (rgbaint)vec_msum(color01, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color11 = (rgbaint)vec_msum(color11, rgbvmx_statics.scale_table[u], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(1));
	color11 = (rgbaint)vec_sl((vector signed int)color11, vec_splat_u32(15));
	color01 = vec_max(color01, color11);
	color01 = (rgbaint)vec_msum(color01, rgbvmx_statics.scale_table[v], vec_splat_s32(0));
	color01 = (rgbaint)vec_sr((vector signed int)color01, vec_splat_u32(15));
	*color = vec_packs((vector signed int)color01, (vector signed int)color01);
}

// altivec.h somehow redefines "bool" in a bad way on PowerPC Mac OS X.  really.
#ifdef OSX_PPC
#undef vector
#undef pixel
#undef bool
#endif

#endif /* __RGBVMX__ */
