	// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vooddefs.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODDEFS_IPP
#define MAME_VIDEO_VOODDEFS_IPP

#pragma once

#include "voodoo.h"



/*************************************
 *
 *  Core types
 *
 *************************************/

typedef voodoo_reg rgb_union;





/*************************************
 *
 *  Inline FIFO management
 *
 *************************************/

inline void voodoo_device::fifo_state::add(uint32_t data)
{
	/* compute the value of 'in' after we add this item */
	int32_t next_in = in + 1;
	if (next_in >= size)
		next_in = 0;

	/* as long as it's not equal to the output pointer, we can do it */
	if (next_in != out)
	{
		base[in] = data;
		in = next_in;
	}
}


inline uint32_t voodoo_device::fifo_state::remove()
{
	uint32_t data = 0xffffffff;

	/* as long as we have data, we can do it */
	if (out != in)
	{
		int32_t next_out;

		/* fetch the data */
		data = base[out];

		/* advance the output pointer */
		next_out = out + 1;
		if (next_out >= size)
			next_out = 0;
		out = next_out;
	}
	return data;
}


inline int32_t voodoo_device::fifo_state::items() const
{
	int32_t items = in - out;
	if (items < 0)
		items += size;
	return items;
}



/*************************************
 *
 *  Computes a fast 16.16 reciprocal
 *  of a 16.32 value; used for
 *  computing 1/w in the rasterizer.
 *
 *  Since it is trivial to also
 *  compute log2(1/w) = -log2(w) at
 *  the same time, we do that as well
 *  to 16.8 precision for LOD
 *  calculations.
 *
 *  On a Pentium M, this routine is
 *  20% faster than a 64-bit integer
 *  divide and also produces the log
 *  for free.
 *
 *************************************/

static inline int32_t fast_reciplog(int64_t value, int32_t *log2)
{
	extern uint32_t voodoo_reciplog[];
	uint32_t temp, recip, rlog;
	uint32_t interp;
	uint32_t *table;
	int neg = false;
	int lz, exp = 0;

	/* always work with unsigned numbers */
	if (value < 0)
	{
		value = -value;
		neg = true;
	}

	/* if we've spilled out of 32 bits, push it down under 32 */
	if (value & 0xffff00000000U)
	{
		temp = (uint32_t)(value >> 16);
		exp -= 16;
	}
	else
		temp = (uint32_t)value;

	/* if the resulting value is 0, the reciprocal is infinite */
	if (UNEXPECTED(temp == 0))
	{
		*log2 = 1000 << LOG_OUTPUT_PREC;
		return neg ? 0x80000000 : 0x7fffffff;
	}

	/* determine how many leading zeros in the value and shift it up high */
	lz = count_leading_zeros(temp);
	temp <<= lz;
	exp += lz;

	/* compute a pointer to the table entries we want */
	/* math is a bit funny here because we shift one less than we need to in order */
	/* to account for the fact that there are two uint32_t's per table entry */
	table = &voodoo_reciplog[(temp >> (31 - RECIPLOG_LOOKUP_BITS - 1)) & ((2 << RECIPLOG_LOOKUP_BITS) - 2)];

	/* compute the interpolation value */
	interp = (temp >> (31 - RECIPLOG_LOOKUP_BITS - 8)) & 0xff;

	/* do a linear interpolatation between the two nearest table values */
	/* for both the log and the reciprocal */
	rlog = (table[1] * (0x100 - interp) + table[3] * interp) >> 8;
	recip = (table[0] * (0x100 - interp) + table[2] * interp) >> 8;

	/* the log result is the fractional part of the log; round it to the output precision */
	rlog = (rlog + (1 << (RECIPLOG_LOOKUP_PREC - LOG_OUTPUT_PREC - 1))) >> (RECIPLOG_LOOKUP_PREC - LOG_OUTPUT_PREC);

	/* the exponent is the non-fractional part of the log; normally, we would subtract it from rlog */
	/* but since we want the log(1/value) = -log(value), we subtract rlog from the exponent */
	*log2 = ((exp - (31 - RECIPLOG_INPUT_PREC)) << LOG_OUTPUT_PREC) - rlog;

	/* adjust the exponent to account for all the reciprocal-related parameters to arrive at a final shift amount */
	exp += (RECIP_OUTPUT_PREC - RECIPLOG_LOOKUP_PREC) - (31 - RECIPLOG_INPUT_PREC);

	/* shift by the exponent */
	if (exp < 0)
		recip >>= -exp;
	else
		recip <<= exp;

	/* on the way out, apply the original sign to the reciprocal */
	return neg ? -recip : recip;
}



/*************************************
 *
 *  Float-to-int conversions
 *
 *************************************/

static inline int32_t float_to_int32(uint32_t data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	int32_t result = (data & 0x7fffff) | 0x800000;
	if (exponent < 0)
	{
		if (exponent > -32)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 32)
			result <<= exponent;
		else
			result = 0x7fffffff;
	}
	if (data & 0x80000000)
		result = -result;
	return result;
}


static inline int64_t float_to_int64(uint32_t data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	int64_t result = (data & 0x7fffff) | 0x800000;
	if (exponent < 0)
	{
		if (exponent > -64)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 64)
			result <<= exponent;
		else
			result = 0x7fffffffffffffffU;
	}
	if (data & 0x80000000)
		result = -result;
	return result;
}



/*************************************
 *
 *  Rasterizer inlines
 *
 *************************************/

static inline uint32_t normalize_color_path(uint32_t eff_color_path)
{
	/* ignore the subpixel adjust and texture enable flags */
	eff_color_path &= ~((1 << 26) | (1 << 27));

	return eff_color_path;
}


static inline uint32_t normalize_alpha_mode(uint32_t eff_alpha_mode)
{
	/* always ignore alpha ref value */
	eff_alpha_mode &= ~(0xff << 24);

	/* if not doing alpha testing, ignore the alpha function and ref value */
	if (!ALPHAMODE_ALPHATEST(eff_alpha_mode))
		eff_alpha_mode &= ~(7 << 1);

	/* if not doing alpha blending, ignore the source and dest blending factors */
	if (!ALPHAMODE_ALPHABLEND(eff_alpha_mode))
		eff_alpha_mode &= ~((15 << 8) | (15 << 12) | (15 << 16) | (15 << 20));

	return eff_alpha_mode;
}


static inline uint32_t normalize_fog_mode(uint32_t eff_fog_mode)
{
	/* if not doing fogging, ignore all the other fog bits */
	if (!FOGMODE_ENABLE_FOG(eff_fog_mode))
		eff_fog_mode = 0;

	return eff_fog_mode;
}


static inline uint32_t normalize_fbz_mode(uint32_t eff_fbz_mode)
{
	/* ignore the draw buffer */
	eff_fbz_mode &= ~(3 << 14);

	return eff_fbz_mode;
}


static inline uint32_t normalize_tex_mode(uint32_t eff_tex_mode)
{
	/* ignore the NCC table and seq_8_downld flags */
	eff_tex_mode &= ~((1 << 5) | (1 << 31));

	/* classify texture formats into 3 format categories */
	if (TEXMODE_FORMAT(eff_tex_mode) < 8)
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (0 << 8);
	else if (TEXMODE_FORMAT(eff_tex_mode) >= 10 && TEXMODE_FORMAT(eff_tex_mode) <= 12)
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (10 << 8);
	else
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (8 << 8);

	return eff_tex_mode;
}


inline uint32_t voodoo_device::raster_info::compute_hash() const
{
	uint32_t result;

	/* make a hash */
	result = eff_color_path;
	result = (result << 1) | (result >> 31);
	result ^= eff_fbz_mode;
	result = (result << 1) | (result >> 31);
	result ^= eff_alpha_mode;
	result = (result << 1) | (result >> 31);
	result ^= eff_fog_mode;
	result = (result << 1) | (result >> 31);
	result ^= eff_tex_mode_0;
	result = (result << 1) | (result >> 31);
	result ^= eff_tex_mode_1;

	return result % RASTER_HASH_SIZE;
}



/*************************************
 *
 *  Dithering macros
 *
 *************************************/

/* note that these equations and the dither matrixes have
   been confirmed to be exact matches to the real hardware */
#define DITHER_RB(val,dith) ((((val) << 1) - ((val) >> 4) + ((val) >> 7) + (dith)) >> 1)
#define DITHER_G(val,dith)  ((((val) << 2) - ((val) >> 4) + ((val) >> 6) + (dith)) >> 2)

#define DECLARE_DITHER_POINTERS                                                 \
	const uint8_t *dither_lookup = nullptr;                                          \
	const uint8_t *dither4 = nullptr;                                                \
	const uint8_t *dither = nullptr
#define DECLARE_DITHER_POINTERS_NO_DITHER_VAR                                               \
	const uint8_t *dither_lookup = nullptr;
#define COMPUTE_DITHER_POINTERS(FBZMODE, YY, FOGMODE)                           \
do                                                                              \
{                                                                               \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE) || FOGMODE_FOG_DITHER(FOGMODE))        \
		dither4 = &dither_matrix_4x4[((YY) & 3) * 4];                           \
	/* compute the dithering pointers */                                        \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		if (FBZMODE_DITHER_TYPE(FBZMODE) == 0)                                  \
		{                                                                       \
			dither = &dither_subtract_4x4[((YY) & 3) * 4];                      \
			dither_lookup = &dither4_lookup[(YY & 3) << 11];                    \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			dither = &dither_subtract_2x2[((YY) & 3) * 4];                      \
			dither_lookup = &dither2_lookup[(YY & 3) << 11];                    \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

#define COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(FBZMODE, YY)                                  \
do                                                                              \
{                                                                               \
	/* compute the dithering pointers */                                        \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		if (FBZMODE_DITHER_TYPE(FBZMODE) == 0)                                  \
		{                                                                       \
			dither_lookup = &dither4_lookup[(YY & 3) << 11];                    \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			dither_lookup = &dither2_lookup[(YY & 3) << 11];                    \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

#define APPLY_DITHER(FBZMODE, XX, DITHER_LOOKUP, RR, GG, BB)                    \
do                                                                              \
{                                                                               \
	/* apply dithering */                                                       \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		/* look up the dither value from the appropriate matrix */              \
		const uint8_t *dith = &DITHER_LOOKUP[((XX) & 3) << 1];                    \
																				\
		/* apply dithering to R,G,B */                                          \
		(RR) = dith[((RR) << 3) + 0];                                           \
		(GG) = dith[((GG) << 3) + 1];                                           \
		(BB) = dith[((BB) << 3) + 0];                                           \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		(RR) >>= 3;                                                             \
		(GG) >>= 2;                                                             \
		(BB) >>= 3;                                                             \
	}                                                                           \
}                                                                               \
while (0)



/*************************************
 *
 *  Clamping macros
 *
 *************************************/

#define CLAMPED_ARGB(ITERR, ITERG, ITERB, ITERA, FBZCP, RESULT)                 \
do                                                                              \
{                                                                               \
	r = (int32_t)(ITERR) >> 12;                                             \
	g = (int32_t)(ITERG) >> 12;                                             \
	b = (int32_t)(ITERB) >> 12;                                             \
	a = (int32_t)(ITERA) >> 12;                                             \
																				\
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		r &= 0xfff;                                                             \
		RESULT.rgb.r = r;                                                       \
		if (r == 0xfff)                                                         \
			RESULT.rgb.r = 0;                                                   \
		else if (r == 0x100)                                                    \
			RESULT.rgb.r = 0xff;                                                \
																				\
		g &= 0xfff;                                                             \
		RESULT.rgb.g = g;                                                       \
		if (g == 0xfff)                                                         \
			RESULT.rgb.g = 0;                                                   \
		else if (g == 0x100)                                                    \
			RESULT.rgb.g = 0xff;                                                \
																				\
		b &= 0xfff;                                                             \
		RESULT.rgb.b = b;                                                       \
		if (b == 0xfff)                                                         \
			RESULT.rgb.b = 0;                                                   \
		else if (b == 0x100)                                                    \
			RESULT.rgb.b = 0xff;                                                \
																				\
		a &= 0xfff;                                                             \
		RESULT.rgb.a = a;                                                       \
		if (a == 0xfff)                                                         \
			RESULT.rgb.a = 0;                                                   \
		else if (a == 0x100)                                                    \
			RESULT.rgb.a = 0xff;                                                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		RESULT.rgb.r = (r < 0) ? 0 : (r > 0xff) ? 0xff : r;                     \
		RESULT.rgb.g = (g < 0) ? 0 : (g > 0xff) ? 0xff : g;                     \
		RESULT.rgb.b = (b < 0) ? 0 : (b > 0xff) ? 0xff : b;                     \
		RESULT.rgb.a = (a < 0) ? 0 : (a > 0xff) ? 0xff : a;                     \
	}                                                                           \
}                                                                               \
while (0)

static inline rgbaint_t ATTR_FORCE_INLINE clampARGB(const rgbaint_t &iterargb, uint32_t FBZCP)
{
	rgbaint_t result(iterargb);
	//rgbaint_t colorint((int32_t) (itera>>12), (int32_t) (iterr>>12), (int32_t) (iterg>>12), (int32_t) (iterb>>12));
	result.shr_imm(12);

	if (!FBZCP_RGBZW_CLAMP(FBZCP))
	{
		//r &= 0xfff;
		result.and_imm(0xfff);
		//if (r == 0xfff)
		rgbaint_t temp(result);
		temp.cmpeq_imm(0xfff);
		//  result.rgb.r = 0;
		result.andnot_reg(temp);
		//else if (r == 0x100)
		//  result.rgb.r = 0xff;
		temp.set(result);
		temp.cmpeq_imm(0x100);
		result.or_reg(temp);
		// else wrap
		result.and_imm(0xff);
	}
	else
	{
		//return colorint.to_rgba_clamp();
		result.clamp_to_uint8();
	}
	return result;
}

#define CLAMPED_Z(ITERZ, FBZCP, RESULT)                                         \
do                                                                              \
{                                                                               \
	(RESULT) = (int32_t)(ITERZ) >> 12;                                            \
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		(RESULT) &= 0xfffff;                                                    \
		if ((RESULT) == 0xfffff)                                                \
			(RESULT) = 0;                                                       \
		else if ((RESULT) == 0x10000)                                           \
			(RESULT) = 0xffff;                                                  \
		else                                                                    \
			(RESULT) &= 0xffff;                                                 \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		CLAMP((RESULT), 0, 0xffff);                                             \
	}                                                                           \
}                                                                               \
while (0)


#define CLAMPED_W(ITERW, FBZCP, RESULT)                                         \
do                                                                              \
{                                                                               \
	(RESULT) = (int16_t)((ITERW) >> 32);                                          \
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		(RESULT) &= 0xffff;                                                     \
		if ((RESULT) == 0xffff)                                                 \
			(RESULT) = 0;                                                       \
		else if ((RESULT) == 0x100)                                             \
			(RESULT) = 0xff;                                                    \
		(RESULT) &= 0xff;                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		CLAMP((RESULT), 0, 0xff);                                               \
	}                                                                           \
}                                                                               \
while (0)



/*************************************
 *
 *  Chroma keying macro
 *
 *************************************/

#define APPLY_CHROMAKEY(VV, STATS, FBZMODE, COLOR)                              \
do                                                                              \
{                                                                               \
	if (FBZMODE_ENABLE_CHROMAKEY(FBZMODE))                                      \
	{                                                                           \
		/* non-range version */                                                 \
		if (!CHROMARANGE_ENABLE((VV)->reg[chromaRange].u))                      \
		{                                                                       \
			if (((COLOR.u ^ (VV)->reg[chromaKey].u) & 0xffffff) == 0)           \
			{                                                                   \
				(STATS)->chroma_fail++;                                         \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* tricky range version */                                              \
		else                                                                    \
		{                                                                       \
			int32_t low, high, test;                                              \
			int results = 0;                                                    \
																				\
			/* check blue */                                                    \
			low = (VV)->reg[chromaKey].rgb.b;                                   \
			high = (VV)->reg[chromaRange].rgb.b;                                \
			test = COLOR.rgb.b;                                                 \
			results = (test >= low && test <= high);                            \
			results ^= CHROMARANGE_BLUE_EXCLUSIVE((VV)->reg[chromaRange].u);    \
			results <<= 1;                                                      \
																				\
			/* check green */                                                   \
			low = (VV)->reg[chromaKey].rgb.g;                                   \
			high = (VV)->reg[chromaRange].rgb.g;                                \
			test = COLOR.rgb.g;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_GREEN_EXCLUSIVE((VV)->reg[chromaRange].u);   \
			results <<= 1;                                                      \
																				\
			/* check red */                                                     \
			low = (VV)->reg[chromaKey].rgb.r;                                   \
			high = (VV)->reg[chromaRange].rgb.r;                                \
			test = COLOR.rgb.r;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_RED_EXCLUSIVE((VV)->reg[chromaRange].u);     \
																				\
			/* final result */                                                  \
			if (CHROMARANGE_UNION_MODE((VV)->reg[chromaRange].u))               \
			{                                                                   \
				if (results != 0)                                               \
				{                                                               \
					(STATS)->chroma_fail++;                                     \
					goto skipdrawdepth;                                         \
				}                                                               \
			}                                                                   \
			else                                                                \
			{                                                                   \
				if (results == 7)                                               \
				{                                                               \
					(STATS)->chroma_fail++;                                     \
					goto skipdrawdepth;                                         \
				}                                                               \
			}                                                                   \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

inline bool ATTR_FORCE_INLINE voodoo_device::chromaKeyTest(voodoo_device *vd, stats_block *stats, uint32_t fbzModeReg, rgbaint_t rgbaIntColor)
{
	{
		rgb_union color;
		color.u = rgbaIntColor.to_rgba();
		/* non-range version */
		if (!CHROMARANGE_ENABLE(vd->reg[chromaRange].u))
		{
			if (((color.u ^ vd->reg[chromaKey].u) & 0xffffff) == 0)
			{
				stats->chroma_fail++;
				return false;
			}
		}

		/* tricky range version */
		else
		{
			int32_t low, high, test;
			int results;

			/* check blue */
			low = vd->reg[chromaKey].rgb.b;
			high = vd->reg[chromaRange].rgb.b;
			test = color.rgb.b;
			results = (test >= low && test <= high);
			results ^= CHROMARANGE_BLUE_EXCLUSIVE(vd->reg[chromaRange].u);
			results <<= 1;

			/* check green */
			low = vd->reg[chromaKey].rgb.g;
			high = vd->reg[chromaRange].rgb.g;
			test = color.rgb.g;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_GREEN_EXCLUSIVE(vd->reg[chromaRange].u);
			results <<= 1;

			/* check red */
			low = vd->reg[chromaKey].rgb.r;
			high = vd->reg[chromaRange].rgb.r;
			test = color.rgb.r;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_RED_EXCLUSIVE(vd->reg[chromaRange].u);

			/* final result */
			if (CHROMARANGE_UNION_MODE(vd->reg[chromaRange].u))
			{
				if (results != 0)
				{
					stats->chroma_fail++;
					return false;
				}
			}
			else
			{
				if (results == 7)
				{
					stats->chroma_fail++;
					return false;
				}
			}
		}
	}
	return true;
}



/*************************************
 *
 *  Alpha masking macro
 *
 *************************************/

#define APPLY_ALPHAMASK(VV, STATS, FBZMODE, AA)                                 \
do                                                                              \
{                                                                               \
	if (FBZMODE_ENABLE_ALPHA_MASK(FBZMODE))                                     \
	{                                                                           \
		if (((AA) & 1) == 0)                                                    \
		{                                                                       \
			(STATS)->afunc_fail++;                                              \
			goto skipdrawdepth;                                                 \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

inline bool voodoo_device::alphaMaskTest(stats_block *stats, uint32_t fbzModeReg, uint8_t alpha)
{
	{
		if ((alpha & 1) == 0)
		{
			stats->afunc_fail++;
			return false;
		}
	}
	return true;
}

/*************************************
 *
 *  Alpha testing macro
 *
 *************************************/

#define APPLY_ALPHATEST(VV, STATS, ALPHAMODE, AA)                               \
do                                                                              \
{                                                                               \
	if (ALPHAMODE_ALPHATEST(ALPHAMODE))                                         \
	{                                                                           \
		uint8_t alpharef = (VV)->reg[alphaMode].rgb.a;                            \
		switch (ALPHAMODE_ALPHAFUNCTION(ALPHAMODE))                             \
		{                                                                       \
			case 0:     /* alphaOP = never */                                   \
				(STATS)->afunc_fail++;                                          \
				goto skipdrawdepth;                                             \
																				\
			case 1:     /* alphaOP = less than */                               \
				if ((AA) >= alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 2:     /* alphaOP = equal */                                   \
				if ((AA) != alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 3:     /* alphaOP = less than or equal */                      \
				if ((AA) > alpharef)                                            \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 4:     /* alphaOP = greater than */                            \
				if ((AA) <= alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 5:     /* alphaOP = not equal */                               \
				if ((AA) == alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 6:     /* alphaOP = greater than or equal */                   \
				if ((AA) < alpharef)                                            \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 7:     /* alphaOP = always */                                  \
				break;                                                          \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

inline bool ATTR_FORCE_INLINE voodoo_device::alphaTest(uint8_t alpharef, stats_block *stats, uint32_t alphaModeReg, uint8_t alpha)
{
	{
		switch (ALPHAMODE_ALPHAFUNCTION(alphaModeReg))
		{
			case 0:     /* alphaOP = never */
				stats->afunc_fail++;
				return false;

			case 1:     /* alphaOP = less than */
				if (alpha >= alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 2:     /* alphaOP = equal */
				if (alpha != alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 3:     /* alphaOP = less than or equal */
				if (alpha > alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 4:     /* alphaOP = greater than */
				if (alpha <= alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 5:     /* alphaOP = not equal */
				if (alpha == alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 6:     /* alphaOP = greater than or equal */
				if (alpha < alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 7:     /* alphaOP = always */
				break;
		}
	}
	return true;
}


/*************************************
 *
 *  Alpha blending macro
 *
 *************************************/

#define APPLY_ALPHA_BLEND(FBZMODE, ALPHAMODE, XX, DITHER, RR, GG, BB, AA)       \
do                                                                              \
{                                                                               \
	if (ALPHAMODE_ALPHABLEND(ALPHAMODE))                                        \
	{                                                                           \
		int dpix = dest[XX];                                                    \
		int dr, dg, db;                                                         \
		EXTRACT_565_TO_888(dpix, dr, dg, db);                                   \
		int da = FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) ? depth[XX] : 0xff;       \
		int sr = (RR);                                                          \
		int sg = (GG);                                                          \
		int sb = (BB);                                                          \
		int sa = (AA);                                                          \
		int ta;                                                                 \
																				\
		/* apply dither subtraction */                                          \
		if (FBZMODE_ALPHA_DITHER_SUBTRACT(FBZMODE))                             \
		{                                                                       \
			/* look up the dither value from the appropriate matrix */          \
			int dith = DITHER[(XX) & 3];                                        \
																				\
			/* subtract the dither value */                                     \
			dr = ((dr << 1) + 15 - dith) >> 1;                                  \
			dg = ((dg << 2) + 15 - dith) >> 2;                                  \
			db = ((db << 1) + 15 - dith) >> 1;                                  \
		}                                                                       \
																				\
		/* compute source portion */                                            \
		switch (ALPHAMODE_SRCRGBBLEND(ALPHAMODE))                               \
		{                                                                       \
			default:    /* reserved */                                          \
			case 0:     /* AZERO */                                             \
				(RR) = (GG) = (BB) = 0;                                         \
				break;                                                          \
																				\
			case 1:     /* ASRC_ALPHA */                                        \
				(RR) = (sr * (sa + 1)) >> 8;                                    \
				(GG) = (sg * (sa + 1)) >> 8;                                    \
				(BB) = (sb * (sa + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 2:     /* A_COLOR */                                           \
				(RR) = (sr * (dr + 1)) >> 8;                                    \
				(GG) = (sg * (dg + 1)) >> 8;                                    \
				(BB) = (sb * (db + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 3:     /* ADST_ALPHA */                                        \
				(RR) = (sr * (da + 1)) >> 8;                                    \
				(GG) = (sg * (da + 1)) >> 8;                                    \
				(BB) = (sb * (da + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 4:     /* AONE */                                              \
				break;                                                          \
																				\
			case 5:     /* AOMSRC_ALPHA */                                      \
				(RR) = (sr * (0x100 - sa)) >> 8;                                \
				(GG) = (sg * (0x100 - sa)) >> 8;                                \
				(BB) = (sb * (0x100 - sa)) >> 8;                                \
				break;                                                          \
																				\
			case 6:     /* AOM_COLOR */                                         \
				(RR) = (sr * (0x100 - dr)) >> 8;                                \
				(GG) = (sg * (0x100 - dg)) >> 8;                                \
				(BB) = (sb * (0x100 - db)) >> 8;                                \
				break;                                                          \
																				\
			case 7:     /* AOMDST_ALPHA */                                      \
				(RR) = (sr * (0x100 - da)) >> 8;                                \
				(GG) = (sg * (0x100 - da)) >> 8;                                \
				(BB) = (sb * (0x100 - da)) >> 8;                                \
				break;                                                          \
																				\
			case 15:    /* ASATURATE */                                         \
				ta = (sa < (0x100 - da)) ? sa : (0x100 - da);                   \
				(RR) = (sr * (ta + 1)) >> 8;                                    \
				(GG) = (sg * (ta + 1)) >> 8;                                    \
				(BB) = (sb * (ta + 1)) >> 8;                                    \
				break;                                                          \
		}                                                                       \
																				\
		/* add in dest portion */                                               \
		switch (ALPHAMODE_DSTRGBBLEND(ALPHAMODE))                               \
		{                                                                       \
			default:    /* reserved */                                          \
			case 0:     /* AZERO */                                             \
				break;                                                          \
																				\
			case 1:     /* ASRC_ALPHA */                                        \
				(RR) += (dr * (sa + 1)) >> 8;                                   \
				(GG) += (dg * (sa + 1)) >> 8;                                   \
				(BB) += (db * (sa + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 2:     /* A_COLOR */                                           \
				(RR) += (dr * (sr + 1)) >> 8;                                   \
				(GG) += (dg * (sg + 1)) >> 8;                                   \
				(BB) += (db * (sb + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 3:     /* ADST_ALPHA */                                        \
				(RR) += (dr * (da + 1)) >> 8;                                   \
				(GG) += (dg * (da + 1)) >> 8;                                   \
				(BB) += (db * (da + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 4:     /* AONE */                                              \
				(RR) += dr;                                                     \
				(GG) += dg;                                                     \
				(BB) += db;                                                     \
				break;                                                          \
																				\
			case 5:     /* AOMSRC_ALPHA */                                      \
				(RR) += (dr * (0x100 - sa)) >> 8;                               \
				(GG) += (dg * (0x100 - sa)) >> 8;                               \
				(BB) += (db * (0x100 - sa)) >> 8;                               \
				break;                                                          \
																				\
			case 6:     /* AOM_COLOR */                                         \
				(RR) += (dr * (0x100 - sr)) >> 8;                               \
				(GG) += (dg * (0x100 - sg)) >> 8;                               \
				(BB) += (db * (0x100 - sb)) >> 8;                               \
				break;                                                          \
																				\
			case 7:     /* AOMDST_ALPHA */                                      \
				(RR) += (dr * (0x100 - da)) >> 8;                               \
				(GG) += (dg * (0x100 - da)) >> 8;                               \
				(BB) += (db * (0x100 - da)) >> 8;                               \
				break;                                                          \
																				\
			case 15:    /* A_COLORBEFOREFOG */                                  \
				(RR) += (dr * (prefogr + 1)) >> 8;                              \
				(GG) += (dg * (prefogg + 1)) >> 8;                              \
				(BB) += (db * (prefogb + 1)) >> 8;                              \
				break;                                                          \
		}                                                                       \
																				\
		/* blend the source alpha */                                            \
		(AA) = 0;                                                               \
		if (ALPHAMODE_SRCALPHABLEND(ALPHAMODE) == 4)                            \
			(AA) = sa;                                                          \
																				\
		/* blend the dest alpha */                                              \
		if (ALPHAMODE_DSTALPHABLEND(ALPHAMODE) == 4)                            \
			(AA) += da;                                                         \
																				\
		/* clamp */                                                             \
		CLAMP((RR), 0x00, 0xff);                                                \
		CLAMP((GG), 0x00, 0xff);                                                \
		CLAMP((BB), 0x00, 0xff);                                                \
		CLAMP((AA), 0x00, 0xff);                                                \
	}                                                                           \
}                                                                               \
while (0)

static inline void ATTR_FORCE_INLINE alphaBlend(uint32_t FBZMODE, uint32_t ALPHAMODE, int32_t x, const uint8_t *dither, int dpix, uint16_t *depth, rgbaint_t &preFog, rgbaint_t &srcColor, rgb_t *convTable)
{
	{
		//int dpix = dest[XX];
		int dr, dg, db;
		//EXTRACT_565_TO_888(dpix, dr, dg, db);
		rgb_t drgb = convTable[dpix];
		drgb.expand_rgb(dr, dg, db);
		int da = FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) ? depth[x] : 0xff;
		//int sr = (RR);
		//int sg = (GG);
		//int sb = (BB);
		//int sa = (AA);
		int sa = srcColor.get_a();
		int ta;
		int srcAlphaScale, destAlphaScale;
		rgbaint_t srcScale, destScale;

		/* apply dither subtraction */
		if (FBZMODE_ALPHA_DITHER_SUBTRACT(FBZMODE))
		{
			/* look up the dither value from the appropriate matrix */
			int dith = dither[x & 3];

			/* subtract the dither value */
			dr += dith;
			db += dith;
			dg += dith >> 1;
		}

		/* blend the source alpha */
		if (ALPHAMODE_SRCALPHABLEND(ALPHAMODE) == 4)
			srcAlphaScale = 256;
			//(AA) = sa;
		else
			srcAlphaScale = 0;

		/* compute source portion */
		switch (ALPHAMODE_SRCRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				srcScale.zero();
				//(RR) = (GG) = (BB) = 0;
				break;

			case 1:     /* ASRC_ALPHA */
				ta = sa + 1;
				srcScale.set_all(ta);
				//(RR) = (sr * (sa + 1)) >> 8;
				//(GG) = (sg * (sa + 1)) >> 8;
				//(BB) = (sb * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				srcScale.set(dr, dr, dg, db);
				srcScale.add_imm(1);
				//(RR) = (sr * (dr + 1)) >> 8;
				//(GG) = (sg * (dg + 1)) >> 8;
				//(BB) = (sb * (db + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				srcScale.set_all(ta);
				//(RR) = (sr * (da + 1)) >> 8;
				//(GG) = (sg * (da + 1)) >> 8;
				//(BB) = (sb * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				srcScale.set_all(256);
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				srcScale.set_all(ta);
				//(RR) = (sr * (0x100 - sa)) >> 8;
				//(GG) = (sg * (0x100 - sa)) >> 8;
				//(BB) = (sb * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				srcScale.set((0x100 - dr), (0x100 - dr), (0x100 - dg), (0x100 - db));
				//(RR) = (sr * (0x100 - dr)) >> 8;
				//(GG) = (sg * (0x100 - dg)) >> 8;
				//(BB) = (sb * (0x100 - db)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				srcScale.set_all(ta);
				//(RR) = (sr * (0x100 - da)) >> 8;
				//(GG) = (sg * (0x100 - da)) >> 8;
				//(BB) = (sb * (0x100 - da)) >> 8;
				break;

			case 15:    /* ASATURATE */
				ta = (sa < (0x100 - da)) ? sa : (0x100 - da);
				ta++;
				srcScale.set_all(ta);
				//(RR) = (sr * (ta + 1)) >> 8;
				//(GG) = (sg * (ta + 1)) >> 8;
				//(BB) = (sb * (ta + 1)) >> 8;
				break;
		}
		// Set srcScale alpha
		srcScale.set_a16(srcAlphaScale);

		/* blend the dest alpha */
		if (ALPHAMODE_DSTALPHABLEND(ALPHAMODE) == 4)
			destAlphaScale = 256;
			//(AA) += da;
		else
			destAlphaScale = 0;

		/* add in dest portion */
		switch (ALPHAMODE_DSTRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				destScale.zero();
				break;

			case 1:     /* ASRC_ALPHA */
				ta = sa + 1;
				destScale.set_all(ta);
				//(RR) += (dr * (sa + 1)) >> 8;
				//(GG) += (dg * (sa + 1)) >> 8;
				//(BB) += (db * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				destScale.set(srcColor);
				destScale.add_imm(1);
				//(RR) += (dr * (sr + 1)) >> 8;
				//(GG) += (dg * (sg + 1)) >> 8;
				//(BB) += (db * (sb + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				destScale.set_all(ta);
				//(RR) += (dr * (da + 1)) >> 8;
				//(GG) += (dg * (da + 1)) >> 8;
				//(BB) += (db * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				destScale.set_all(256);
				//(RR) += dr;
				//(GG) += dg;
				//(BB) += db;
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				destScale.set_all(ta);
				//(RR) += (dr * (0x100 - sa)) >> 8;
				//(GG) += (dg * (0x100 - sa)) >> 8;
				//(BB) += (db * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				destScale.set_all(0x100);
				destScale.sub(srcColor);
				//destScale.set(destAlphaScale, (0x100 - color.rgb.r), (0x100 - color.rgb.g), (0x100 - color.rgb.b));
				//(RR) += (dr * (0x100 - sr)) >> 8;
				//(GG) += (dg * (0x100 - sg)) >> 8;
				//(BB) += (db * (0x100 - sb)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				destScale.set_all(ta);
				//(RR) += (dr * (0x100 - da)) >> 8;
				//(GG) += (dg * (0x100 - da)) >> 8;
				//(BB) += (db * (0x100 - da)) >> 8;
				break;

			case 15:    /* A_COLORBEFOREFOG */
				destScale.set(preFog);
				destScale.add_imm(1);
				//(RR) += (dr * (prefogr + 1)) >> 8;
				//(GG) += (dg * (prefogg + 1)) >> 8;
				//(BB) += (db * (prefogb + 1)) >> 8;
				break;
		}

		// Set destScale alpha
		destScale.set_a16(destAlphaScale);

		// Main blend
		rgbaint_t destColor(da, dr, dg, db);

		srcColor.scale2_add_and_clamp(srcScale, destColor, destScale);
		/* clamp */
		//CLAMP((RR), 0x00, 0xff);
		//CLAMP((GG), 0x00, 0xff);
		//CLAMP((BB), 0x00, 0xff);
		//CLAMP((AA), 0x00, 0xff);
	}
}


/*************************************
 *
 *  Fogging macro
 *
 *************************************/

#define APPLY_FOGGING(VV, FOGMODE, FBZCP, XX, DITHER4, RR, GG, BB, ITERZ, ITERW, ITERAXXX)  \
do                                                                              \
{                                                                               \
	if (FOGMODE_ENABLE_FOG(FOGMODE))                                            \
	{                                                                           \
		rgb_union fogcolor = (VV)->reg[fogColor];                               \
		int32_t fr, fg, fb;                                                       \
																				\
		/* constant fog bypasses everything else */                             \
		if (FOGMODE_FOG_CONSTANT(FOGMODE))                                      \
		{                                                                       \
			fr = fogcolor.rgb.r;                                                \
			fg = fogcolor.rgb.g;                                                \
			fb = fogcolor.rgb.b;                                                \
		}                                                                       \
																				\
		/* non-constant fog comes from several sources */                       \
		else                                                                    \
		{                                                                       \
			int32_t fogblend = 0;                                                 \
																				\
			/* if fog_add is zero, we start with the fog color */               \
			if (FOGMODE_FOG_ADD(FOGMODE) == 0)                                  \
			{                                                                   \
				fr = fogcolor.rgb.r;                                            \
				fg = fogcolor.rgb.g;                                            \
				fb = fogcolor.rgb.b;                                            \
			}                                                                   \
			else                                                                \
				fr = fg = fb = 0;                                               \
																				\
			/* if fog_mult is zero, we subtract the incoming color */           \
			if (FOGMODE_FOG_MULT(FOGMODE) == 0)                                 \
			{                                                                   \
				fr -= (RR);                                                     \
				fg -= (GG);                                                     \
				fb -= (BB);                                                     \
			}                                                                   \
																				\
			/* fog blending mode */                                             \
			switch (FOGMODE_FOG_ZALPHA(FOGMODE))                                \
			{                                                                   \
				case 0:     /* fog table */                                     \
				{                                                               \
					int32_t delta = (VV)->fbi.fogdelta[fogdepth >> 10];             \
					int32_t deltaval;                                             \
																				\
					/* perform the multiply against lower 8 bits of wfloat */   \
					deltaval = (delta & (VV)->fbi.fogdelta_mask) *              \
								((fogdepth >> 2) & 0xff);                         \
																				\
					/* fog zones allow for negating this value */               \
					if (FOGMODE_FOG_ZONES(FOGMODE) && (delta & 2))              \
						deltaval = -deltaval;                                   \
					deltaval >>= 6;                                             \
																				\
					/* apply dither */                                          \
					if (FOGMODE_FOG_DITHER(FOGMODE))                            \
						deltaval += DITHER4[(XX) & 3];                          \
					deltaval >>= 4;                                             \
																				\
					/* add to the blending factor */                            \
					fogblend = (VV)->fbi.fogblend[fogdepth >> 10] + deltaval;     \
					break;                                                      \
				}                                                               \
																				\
				case 1:     /* iterated A */                                    \
					fogblend = ITERAXXX.rgb.a;                                  \
					break;                                                      \
																				\
				case 2:     /* iterated Z */                                    \
					CLAMPED_Z((ITERZ), FBZCP, fogblend);                        \
					fogblend >>= 8;                                             \
					break;                                                      \
																				\
				case 3:     /* iterated W - Voodoo 2 only */                    \
					CLAMPED_W((ITERW), FBZCP, fogblend);                        \
					break;                                                      \
			}                                                                   \
																				\
			/* perform the blend */                                             \
			fogblend++;                                                         \
			fr = (fr * fogblend) >> 8;                                          \
			fg = (fg * fogblend) >> 8;                                          \
			fb = (fb * fogblend) >> 8;                                          \
		}                                                                       \
																				\
		/* if fog_mult is 0, we add this to the original color */               \
		if (FOGMODE_FOG_MULT(FOGMODE) == 0)                                     \
		{                                                                       \
			(RR) += fr;                                                         \
			(GG) += fg;                                                         \
			(BB) += fb;                                                         \
		}                                                                       \
																				\
		/* otherwise this just becomes the new color */                         \
		else                                                                    \
		{                                                                       \
			(RR) = fr;                                                          \
			(GG) = fg;                                                          \
			(BB) = fb;                                                          \
		}                                                                       \
																				\
		/* clamp */                                                             \
		CLAMP((RR), 0x00, 0xff);                                                \
		CLAMP((GG), 0x00, 0xff);                                                \
		CLAMP((BB), 0x00, 0xff);                                                \
	}                                                                           \
}                                                                               \
while (0)

static inline void ATTR_FORCE_INLINE applyFogging(voodoo_device *vd, uint32_t fbzModeReg, uint32_t fogModeReg, uint32_t fbzCpReg,  int32_t x, const uint8_t *dither4, int32_t wFloat,
	rgbaint_t &color, int32_t iterz, int64_t iterw, const rgbaint_t &iterargb)
{
	{
		/* constant fog bypasses everything else */
		rgbaint_t fogColorLocal(vd->reg[fogColor].u);

		if (FOGMODE_FOG_CONSTANT(fogModeReg))
		{
			/* if fog_mult is 0, we add this to the original color */
			if (FOGMODE_FOG_MULT(fogModeReg) == 0)
			{
				fogColorLocal.add(color);
				fogColorLocal.clamp_to_uint8();
				//color += fog;
			}

			/* otherwise this just becomes the new color */
			else
			{
				//color = fogColorLocal;
				//color = fog;
			}
		}
		/* non-constant fog comes from several sources */
		else
		{
			int32_t fogblend = 0;

			/* if fog_add is zero, we start with the fog color */
			if (FOGMODE_FOG_ADD(fogModeReg))
				fogColorLocal.zero();
				//fr = fg = fb = 0;

			/* if fog_mult is zero, we subtract the incoming color */
			if (!FOGMODE_FOG_MULT(fogModeReg))
			{
				// Need to check this, manual states 9 bits
				fogColorLocal.sub(color);
				//fog.rgb -= color.rgb;
				//fr -= (RR);
				//fg -= (GG);
				//fb -= (BB);
			}

			/* fog blending mode */
			switch (FOGMODE_FOG_ZALPHA(fogModeReg))
			{
				case 0:     /* fog table */
				{
					int32_t fogDepth = wFloat;
					/* add the bias for fog selection*/
					if (FBZMODE_ENABLE_DEPTH_BIAS(fbzModeReg))
					{
						fogDepth += (int16_t)vd->reg[zaColor].u;
						CLAMP(fogDepth, 0, 0xffff);
					}
					int32_t delta = vd->fbi.fogdelta[fogDepth >> 10];
					int32_t deltaval;

					/* perform the multiply against lower 8 bits of wfloat */
					deltaval = (delta & vd->fbi.fogdelta_mask) *
								((fogDepth >> 2) & 0xff);

					/* fog zones allow for negating this value */
					if (FOGMODE_FOG_ZONES(fogModeReg) && (delta & 2))
						deltaval = -deltaval;
					deltaval >>= 6;

					/* apply dither */
					if (FOGMODE_FOG_DITHER(fogModeReg))
						deltaval += dither4[x&3];
					deltaval >>= 4;

					/* add to the blending factor */
					fogblend = vd->fbi.fogblend[fogDepth >> 10] + deltaval;
					break;
				}

				case 1:     /* iterated A */
					fogblend = iterargb.get_a();
					break;

				case 2:     /* iterated Z */
					CLAMPED_Z(iterz, fbzCpReg, fogblend);
					fogblend >>= 8;
					break;

				case 3:     /* iterated W - Voodoo 2 only */
					CLAMPED_W(iterw, fbzCpReg, fogblend);
					break;
			}

			/* perform the blend */
			fogblend++;

			//fr = (fr * fogblend) >> 8;
			//fg = (fg * fogblend) >> 8;
			//fb = (fb * fogblend) >> 8;
			/* if fog_mult is 0, we add this to the original color */
			fogColorLocal.scale_imm_and_clamp((int16_t)fogblend);
			if (FOGMODE_FOG_MULT(fogModeReg) == 0)
			{
				fogColorLocal.add(color);
				fogColorLocal.clamp_to_uint8();
				//color += fog;
				//(RR) += fr;
				//(GG) += fg;
				//(BB) += fb;
			}

			/* otherwise this just becomes the new color */
			else
			{
				//color = fog;
				//(RR) = fr;
				//(GG) = fg;
				//(BB) = fb;
			}
		}


		/* clamp */
		//CLAMP((RR), 0x00, 0xff);
		//CLAMP((GG), 0x00, 0xff);
		//CLAMP((BB), 0x00, 0xff);
		fogColorLocal.merge_alpha16(color);
		color.set(fogColorLocal);
	}
}


/*************************************
 *
 *  Texture pipeline macro
 *
 *************************************/

#define TEXTURE_PIPELINE(TT, XX, DITHER4, TEXMODE, COTHER, LOOKUP, LODBASE, ITERS, ITERT, ITERW, RESULT) \
do                                                                              \
{                                                                               \
	int32_t blendr, blendg, blendb, blenda;                                       \
	int32_t tr, tg, tb, ta;                                                       \
	int32_t s, t, lod, ilod;                                                 \
	int32_t smax, tmax;                                                           \
	uint32_t texbase;                                                             \
	rgb_union c_local;                                                          \
																				\
	/* determine the S/T/LOD values for this texture */                         \
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))                                    \
	{                                                                           \
		if (USE_FAST_RECIP) {                                                     \
			const int32_t oow = fast_reciplog((ITERW), &lod);                         \
			s = ((int64_t)oow * (ITERS)) >> 29;                                       \
			t = ((int64_t)oow * (ITERT)) >> 29;                                       \
		} else {                                                                  \
				multi_reciplog(ITERS, ITERT, ITERW, lod, s, t);                      \
		}                                                                       \
		lod += (LODBASE);                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		s = (ITERS) >> 14;                                                      \
		t = (ITERT) >> 14;                                                      \
		lod = (LODBASE);                                                        \
	}                                                                           \
																				\
	/* clamp W */                                                               \
	if (TEXMODE_CLAMP_NEG_W(TEXMODE) && (ITERW) < 0)                            \
		s = t = 0;                                                              \
																				\
	/* clamp the LOD */                                                         \
	lod += (TT)->lodbias;                                                       \
	if (TEXMODE_ENABLE_LOD_DITHER(TEXMODE))                                     \
		lod += DITHER4[(XX) & 3] << 4;                                          \
	if (lod < (TT)->lodmin)                                                     \
		lod = (TT)->lodmin;                                                     \
	if (lod > (TT)->lodmax)                                                     \
		lod = (TT)->lodmax;                                                     \
																				\
	/* now the LOD is in range; if we don't own this LOD, take the next one */  \
	ilod = lod >> 8;                                                            \
	if (!(((TT)->lodmask >> ilod) & 1))                                         \
		ilod++;                                                                 \
																				\
	/* fetch the texture base */                                                \
	texbase = (TT)->lodoffset[ilod];                                            \
																				\
	/* compute the maximum s and t values at this LOD */                        \
	smax = (TT)->wmask >> ilod;                                                 \
	tmax = (TT)->hmask >> ilod;                                                 \
																				\
	/* determine whether we are point-sampled or bilinear */                    \
	if ((lod == (TT)->lodmin && !TEXMODE_MAGNIFICATION_FILTER(TEXMODE)) ||      \
		(lod != (TT)->lodmin && !TEXMODE_MINIFICATION_FILTER(TEXMODE)))         \
	{                                                                           \
		/* point sampled */                                                     \
																				\
		uint32_t texel0;                                                          \
																				\
		/* adjust S/T for the LOD and strip off the fractions */                \
		s >>= ilod + 18;                                                        \
		t >>= ilod + 18;                                                        \
																				\
		/* clamp/wrap S/T if necessary */                                       \
		if (TEXMODE_CLAMP_S(TEXMODE))                                           \
			CLAMP(s, 0, smax);                                                  \
		if (TEXMODE_CLAMP_T(TEXMODE))                                           \
			CLAMP(t, 0, tmax);                                                  \
		s &= smax;                                                              \
		t &= tmax;                                                              \
		t *= smax + 1;                                                          \
																				\
		/* fetch texel data */                                                  \
		if (TEXMODE_FORMAT(TEXMODE) < 8)                                        \
		{                                                                       \
			texel0 = *(uint8_t *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			c_local.u = (LOOKUP)[texel0];                                       \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(uint16_t *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12) \
				c_local.u = (LOOKUP)[texel0];                                   \
			else                                                                \
				c_local.u = ((LOOKUP)[texel0 & 0xff] & 0xffffff) |              \
							((texel0 & 0xff00) << 16);                          \
		}                                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		/* bilinear filtered */                                                 \
																				\
		uint32_t texel0, texel1, texel2, texel3;                                  \
		uint32_t sfrac, tfrac;                                                    \
		int32_t s1, t1;                                                           \
																				\
		/* adjust S/T for the LOD and strip off all but the low 8 bits of */    \
		/* the fraction */                                                      \
		s >>= ilod + 10;                                                        \
		t >>= ilod + 10;                                                        \
																				\
		/* also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel */    \
		s -= 0x80;                                                              \
		t -= 0x80;                                                              \
																				\
		/* extract the fractions */                                             \
		sfrac = s & (TT)->bilinear_mask;                                        \
		tfrac = t & (TT)->bilinear_mask;                                        \
																				\
		/* now toss the rest */                                                 \
		s >>= 8;                                                                \
		t >>= 8;                                                                \
		s1 = s + 1;                                                             \
		t1 = t + 1;                                                             \
																				\
		/* clamp/wrap S/T if necessary */                                       \
		if (TEXMODE_CLAMP_S(TEXMODE))                                           \
		{                                                                       \
			CLAMP(s, 0, smax);                                                  \
			CLAMP(s1, 0, smax);                                                 \
		}                                                                       \
		if (TEXMODE_CLAMP_T(TEXMODE))                                           \
		{                                                                       \
			CLAMP(t, 0, tmax);                                                  \
			CLAMP(t1, 0, tmax);                                                 \
		}                                                                       \
		s &= smax;                                                              \
		s1 &= smax;                                                             \
		t &= tmax;                                                              \
		t1 &= tmax;                                                             \
		t *= smax + 1;                                                          \
		t1 *= smax + 1;                                                         \
																				\
		/* fetch texel data */                                                  \
		if (TEXMODE_FORMAT(TEXMODE) < 8)                                        \
		{                                                                       \
			texel0 = *(uint8_t *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			texel1 = *(uint8_t *)&(TT)->ram[(texbase + t + s1) & (TT)->mask];     \
			texel2 = *(uint8_t *)&(TT)->ram[(texbase + t1 + s) & (TT)->mask];     \
			texel3 = *(uint8_t *)&(TT)->ram[(texbase + t1 + s1) & (TT)->mask];    \
			texel0 = (LOOKUP)[texel0];                                          \
			texel1 = (LOOKUP)[texel1];                                          \
			texel2 = (LOOKUP)[texel2];                                          \
			texel3 = (LOOKUP)[texel3];                                          \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(uint16_t *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			texel1 = *(uint16_t *)&(TT)->ram[(texbase + 2*(t + s1)) & (TT)->mask];\
			texel2 = *(uint16_t *)&(TT)->ram[(texbase + 2*(t1 + s)) & (TT)->mask];\
			texel3 = *(uint16_t *)&(TT)->ram[(texbase + 2*(t1 + s1)) & (TT)->mask];\
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12) \
			{                                                                   \
				texel0 = (LOOKUP)[texel0];                                      \
				texel1 = (LOOKUP)[texel1];                                      \
				texel2 = (LOOKUP)[texel2];                                      \
				texel3 = (LOOKUP)[texel3];                                      \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel0 = ((LOOKUP)[texel0 & 0xff] & 0xffffff) |                 \
							((texel0 & 0xff00) << 16);                          \
				texel1 = ((LOOKUP)[texel1 & 0xff] & 0xffffff) |                 \
							((texel1 & 0xff00) << 16);                          \
				texel2 = ((LOOKUP)[texel2 & 0xff] & 0xffffff) |                 \
							((texel2 & 0xff00) << 16);                          \
				texel3 = ((LOOKUP)[texel3 & 0xff] & 0xffffff) |                 \
							((texel3 & 0xff00) << 16);                          \
			}                                                                   \
		}                                                                       \
																				\
		/* weigh in each texel */                                               \
		c_local.u = rgbaint_t::bilinear_filter(texel0, texel1, texel2, texel3, sfrac, tfrac); \
	}                                                                           \
																				\
	/* select zero/other for RGB */                                             \
	if (!TEXMODE_TC_ZERO_OTHER(TEXMODE))                                        \
	{                                                                           \
		tr = COTHER.rgb.r;                                                      \
		tg = COTHER.rgb.g;                                                      \
		tb = COTHER.rgb.b;                                                      \
	}                                                                           \
	else                                                                        \
		tr = tg = tb = 0;                                                       \
																				\
	/* select zero/other for alpha */                                           \
	if (!TEXMODE_TCA_ZERO_OTHER(TEXMODE))                                       \
		ta = COTHER.rgb.a;                                                      \
	else                                                                        \
		ta = 0;                                                                 \
																				\
	/* potentially subtract c_local */                                          \
	if (TEXMODE_TC_SUB_CLOCAL(TEXMODE))                                         \
	{                                                                           \
		tr -= c_local.rgb.r;                                                    \
		tg -= c_local.rgb.g;                                                    \
		tb -= c_local.rgb.b;                                                    \
	}                                                                           \
	if (TEXMODE_TCA_SUB_CLOCAL(TEXMODE))                                        \
		ta -= c_local.rgb.a;                                                    \
																				\
	/* blend RGB */                                                             \
	switch (TEXMODE_TC_MSELECT(TEXMODE))                                        \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* zero */                                                  \
			blendr = blendg = blendb = 0;                                       \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blendr = c_local.rgb.r;                                             \
			blendg = c_local.rgb.g;                                             \
			blendb = c_local.rgb.b;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blendr = blendg = blendb = COTHER.rgb.a;                            \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blendr = blendg = blendb = c_local.rgb.a;                           \
			break;                                                              \
																				\
		case 4:     /* LOD (detail factor) */                                   \
			if ((TT)->detailbias <= lod)                                        \
				blendr = blendg = blendb = 0;                                   \
			else                                                                \
			{                                                                   \
				blendr = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);\
				if (blendr > (TT)->detailmax)                                   \
					blendr = (TT)->detailmax;                                   \
				blendg = blendb = blendr;                                       \
			}                                                                   \
			break;                                                              \
																				\
		case 5:     /* LOD fraction */                                          \
			blendr = blendg = blendb = lod & 0xff;                              \
			break;                                                              \
	}                                                                           \
																				\
	/* blend alpha */                                                           \
	switch (TEXMODE_TCA_MSELECT(TEXMODE))                                       \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* zero */                                                  \
			blenda = 0;                                                         \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blenda = COTHER.rgb.a;                                              \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 4:     /* LOD (detail factor) */                                   \
			if ((TT)->detailbias <= lod)                                        \
				blenda = 0;                                                     \
			else                                                                \
			{                                                                   \
				blenda = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);\
				if (blenda > (TT)->detailmax)                                   \
					blenda = (TT)->detailmax;                                   \
			}                                                                   \
			break;                                                              \
																				\
		case 5:     /* LOD fraction */                                          \
			blenda = lod & 0xff;                                                \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the RGB blend */                                                 \
	if (!TEXMODE_TC_REVERSE_BLEND(TEXMODE))                                     \
	{                                                                           \
		blendr ^= 0xff;                                                         \
		blendg ^= 0xff;                                                         \
		blendb ^= 0xff;                                                         \
	}                                                                           \
																				\
	/* reverse the alpha blend */                                               \
	if (!TEXMODE_TCA_REVERSE_BLEND(TEXMODE))                                    \
		blenda ^= 0xff;                                                         \
																				\
	/* do the blend */                                                          \
	tr = (tr * (blendr + 1)) >> 8;                                              \
	tg = (tg * (blendg + 1)) >> 8;                                              \
	tb = (tb * (blendb + 1)) >> 8;                                              \
	ta = (ta * (blenda + 1)) >> 8;                                              \
																				\
	/* add clocal or alocal to RGB */                                           \
	switch (TEXMODE_TC_ADD_ACLOCAL(TEXMODE))                                    \
	{                                                                           \
		case 3:     /* reserved */                                              \
		case 0:     /* nothing */                                               \
			break;                                                              \
																				\
		case 1:     /* add c_local */                                           \
			tr += c_local.rgb.r;                                                \
			tg += c_local.rgb.g;                                                \
			tb += c_local.rgb.b;                                                \
			break;                                                              \
																				\
		case 2:     /* add_alocal */                                            \
			tr += c_local.rgb.a;                                                \
			tg += c_local.rgb.a;                                                \
			tb += c_local.rgb.a;                                                \
			break;                                                              \
	}                                                                           \
																				\
	/* add clocal or alocal to alpha */                                         \
	if (TEXMODE_TCA_ADD_ACLOCAL(TEXMODE))                                       \
		ta += c_local.rgb.a;                                                    \
																				\
	/* clamp */                                                                 \
	RESULT.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;                      \
	RESULT.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;                      \
	RESULT.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;                      \
	RESULT.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;                      \
																				\
	/* invert */                                                                \
	if (TEXMODE_TC_INVERT_OUTPUT(TEXMODE))                                      \
		RESULT.u ^= 0x00ffffff;                                                 \
	if (TEXMODE_TCA_INVERT_OUTPUT(TEXMODE))                                     \
		RESULT.rgb.a ^= 0xff;                                                   \
}                                                                               \
while (0)



/*************************************
 *
 *  Pixel pipeline macros
 *
 *************************************/

#define PIXEL_PIPELINE_BEGIN(vd, STATS, XX, YY, FBZCOLORPATH, FBZMODE, ITERZ, ITERW)    \
do                                                                              \
{                                                                               \
	int32_t depthval, wfloat, biasdepth;                                  \
	int32_t r, g, b;                                                           \
																				\
	(STATS)->pixels_in++;                                                       \
																				\
	/* apply clipping */                                                        \
	/* note that for perf reasons, we assume the caller has done clipping */    \
																				\
	/* handle stippling */                                                      \
	if (FBZMODE_ENABLE_STIPPLE(FBZMODE))                                        \
	{                                                                           \
		/* rotate mode */                                                       \
		if (FBZMODE_STIPPLE_PATTERN(FBZMODE) == 0)                              \
		{                                                                       \
			vd->reg[stipple].u = (vd->reg[stipple].u << 1) | (vd->reg[stipple].u >> 31);\
			if ((vd->reg[stipple].u & 0x80000000) == 0)                         \
			{                                                                   \
				vd->stats.total_stippled++;                                     \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* pattern mode */                                                      \
		else                                                                    \
		{                                                                       \
			int stipple_index = (((YY) & 3) << 3) | (~(XX) & 7);                \
			if (((vd->reg[stipple].u >> stipple_index) & 1) == 0)               \
			{                                                                   \
				vd->stats.total_stippled++;                                     \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
	}                                                                           \
																				\
	/* compute "floating point" W value (used for depth and fog) */             \
	if ((ITERW) & 0xffff00000000U)                                              \
		wfloat = 0x0000;                                                        \
	else                                                                        \
	{                                                                           \
		uint32_t temp = (uint32_t)(ITERW);                                      \
		if (!(temp & 0xffff0000))                                               \
			wfloat = 0xffff;                                                    \
		else                                                                    \
		{                                                                       \
			int exp = count_leading_zeros(temp);                                \
			wfloat = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1;       \
		}                                                                       \
	}                                                                           \
																				\
	/* compute depth value (W or Z) for this pixel */                           \
	if (FBZMODE_WBUFFER_SELECT(FBZMODE))                                   \
	{                                                                           \
		if (!FBZMODE_DEPTH_FLOAT_SELECT(FBZMODE))                          \
			depthval = wfloat;                                                      \
		else                                                                        \
		{                                                                           \
			if ((ITERZ) & 0xf0000000)                                               \
				depthval = 0x0000;                                                  \
			else                                                                    \
			{                                                                       \
				uint32_t temp = (ITERZ << 4);                                       \
				if (!(temp & 0xffff0000))                                           \
					depthval = 0xffff;                                              \
				else                                                                \
				{                                                                   \
					int exp = count_leading_zeros(temp);                            \
					depthval = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1; \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		CLAMPED_Z(ITERZ, FBZCOLORPATH, depthval);                               \
	}                                                                           \
	/* add the bias */                                                          \
	biasdepth = depthval;                                                     \
	if (FBZMODE_ENABLE_DEPTH_BIAS(FBZMODE))                                     \
	{                                                                           \
		biasdepth += (int16_t)vd->reg[zaColor].u;                                \
		CLAMP(biasdepth, 0, 0xffff);                                             \
	}


#define DEPTH_TEST(vd, STATS, XX, FBZMODE)    \
do                                                                              \
{                                                                               \
	/* handle depth buffer testing */                                           \
	if (FBZMODE_ENABLE_DEPTHBUF(FBZMODE))                                       \
	{                                                                           \
		int32_t depthsource;                                                      \
																				\
		/* the source depth is either the iterated W/Z+bias or a */             \
		/* constant value */                                                    \
		if (FBZMODE_DEPTH_SOURCE_COMPARE(FBZMODE) == 0)                         \
			depthsource = biasdepth;                                             \
		else                                                                    \
			depthsource = (uint16_t)vd->reg[zaColor].u;                         \
																				\
		/* test against the depth buffer */                                     \
		switch (FBZMODE_DEPTH_FUNCTION(FBZMODE))                                \
		{                                                                       \
			case 0:     /* depthOP = never */                                   \
				(STATS)->zfunc_fail++;                                          \
				goto skipdrawdepth;                                             \
																				\
			case 1:     /* depthOP = less than */                               \
				if (depthsource >= depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 2:     /* depthOP = equal */                                   \
				if (depthsource != depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 3:     /* depthOP = less than or equal */                      \
				if (depthsource > depth[XX])                                    \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 4:     /* depthOP = greater than */                            \
				if (depthsource <= depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 5:     /* depthOP = not equal */                               \
				if (depthsource == depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 6:     /* depthOP = greater than or equal */                   \
				if (depthsource < depth[XX])                                    \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 7:     /* depthOP = always */                                  \
				break;                                                          \
		}                                                                       \
	}                                                                       \
}                                                                               \
while (0)

inline bool ATTR_FORCE_INLINE voodoo_device::depthTest(uint16_t zaColorReg, stats_block *stats, int32_t destDepth, uint32_t fbzModeReg, int32_t biasdepth)
{
	/* handle depth buffer testing */
	{
		int32_t depthsource;

		/* the source depth is either the iterated W/Z+bias or a */
		/* constant value */
		if (FBZMODE_DEPTH_SOURCE_COMPARE(fbzModeReg) == 0)
			depthsource = biasdepth;
		else
			depthsource = zaColorReg;

		/* test against the depth buffer */
		switch (FBZMODE_DEPTH_FUNCTION(fbzModeReg))
		{
			case 0:     /* depthOP = never */
				stats->zfunc_fail++;
				return false;

			case 1:     /* depthOP = less than */
				if (depthsource >= destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 2:     /* depthOP = equal */
				if (depthsource != destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 3:     /* depthOP = less than or equal */
				if (depthsource > destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 4:     /* depthOP = greater than */
				if (depthsource <= destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 5:     /* depthOP = not equal */
				if (depthsource == destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 6:     /* depthOP = greater than or equal */
				if (depthsource < destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 7:     /* depthOP = always */
				break;
		}
	}
	return true;
}

#define PIXEL_PIPELINE_END(STATS, DITHER_LOOKUP, XX, dest, depth, FBZMODE) \
																				\
	r = color.get_r(); g = color.get_g(); b = color.get_b();                     \
	/* modify the pixel for debugging purposes */                               \
	MODIFY_PIXEL(VV);                                                           \
																				\
	/* write to framebuffer */                                                  \
	if (FBZMODE_RGB_BUFFER_MASK(FBZMODE))                                       \
	{                                                                           \
		/* apply dithering */                                                   \
		APPLY_DITHER(FBZMODE, XX, DITHER_LOOKUP, r, g, b);                      \
		dest[XX] = (r << 11) | (g << 5) | b;                                    \
	}                                                                           \
																				\
	/* write to aux buffer */                                                   \
	/*if (depth && FBZMODE_AUX_BUFFER_MASK(FBZMODE))*/                              \
	if (FBZMODE_AUX_BUFFER_MASK(FBZMODE))                              \
	{                                                                           \
		if (FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) == 0)                          \
			depth[XX] = biasdepth;                                               \
		else                                                                    \
			depth[XX] = color.get_a();                                          \
	}                                                                           \
																				\
	/* track pixel writes to the frame buffer regardless of mask */             \
	(STATS)->pixels_out++;                                                      \
																				\
skipdrawdepth:                                                                  \
	;                                                                           \
}                                                                               \
while (0)



/*************************************
 *
 *  Colorpath pipeline macro
 *
 *************************************/

/*

    c_other_is_used:

        if (FBZMODE_ENABLE_CHROMAKEY(FBZMODE) ||
            FBZCP_CC_ZERO_OTHER(FBZCOLORPATH) == 0)

    c_local_is_used:

        if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH) ||
            FBZCP_CC_MSELECT(FBZCOLORPATH) == 1 ||
            FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH) == 1)

    NEEDS_ITER_RGB:

        if ((c_other_is_used && FBZCP_CC_RGBSELECT(FBZCOLORPATH) == 0) ||
            (c_local_is_used && (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) != 0 || FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0))

    NEEDS_ITER_A:

        if ((a_other_is_used && FBZCP_CC_ASELECT(FBZCOLORPATH) == 0) ||
            (a_local_is_used && FBZCP_CCA_LOCALSELECT(FBZCOLORPATH) == 0))

    NEEDS_ITER_Z:

        if (FBZMODE_WBUFFER_SELECT(FBZMODE) == 0 ||
            FBZMODE_DEPTH_FLOAT_SELECT(FBZMODE) != 0 ||
            FBZCP_CCA_LOCALSELECT(FBZCOLORPATH) == 2)


*/

/*
    Expects the following declarations to be outside of this scope:

    int32_t r, g, b, a;
*/
#define COLORPATH_PIPELINE(VV, STATS, FBZCOLORPATH, FBZMODE, ALPHAMODE, TEXELARGB, ITERZ, ITERW, ITERARGB) \
do                                                                              \
{                                                                               \
	int32_t blendr, blendg, blendb, blenda;                                       \
	rgb_union c_other;                                                          \
	rgb_union c_local;                                                          \
																				\
	/* compute c_other */                                                       \
	switch (FBZCP_CC_RGBSELECT(FBZCOLORPATH))                                   \
	{                                                                           \
		case 0:     /* iterated RGB */                                          \
			c_other.u = ITERARGB.u;                                             \
			break;                                                              \
																				\
		case 1:     /* texture RGB */                                           \
			c_other.u = TEXELARGB.u;                                            \
			break;                                                              \
																				\
		case 2:     /* color1 RGB */                                            \
			c_other.u = (VV)->reg[color1].u;                                    \
			break;                                                              \
																				\
		default:    /* reserved - voodoo3 framebufferRGB */                   \
			c_other.u = 0;                                                      \
			break;                                                              \
	}                                                                           \
																				\
	/* handle chroma key */                                                     \
	APPLY_CHROMAKEY(VV, STATS, FBZMODE, c_other);                               \
																				\
	/* compute a_other */                                                       \
	switch (FBZCP_CC_ASELECT(FBZCOLORPATH))                                     \
	{                                                                           \
		case 0:     /* iterated alpha */                                        \
			c_other.rgb.a = ITERARGB.rgb.a;                                     \
			break;                                                              \
																				\
		case 1:     /* texture alpha */                                         \
			c_other.rgb.a = TEXELARGB.rgb.a;                                    \
			break;                                                              \
																				\
		case 2:     /* color1 alpha */                                          \
			c_other.rgb.a = (VV)->reg[color1].rgb.a;                            \
			break;                                                              \
																				\
		default:    /* reserved */                                              \
			c_other.rgb.a = 0;                                                  \
			break;                                                              \
	}                                                                           \
																				\
	/* handle alpha mask */                                                     \
	APPLY_ALPHAMASK(VV, STATS, FBZMODE, c_other.rgb.a);                         \
																				\
	/* compute c_local */                                                       \
	if (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) == 0)                       \
	{                                                                           \
		if (FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0)    /* iterated RGB */      \
			c_local.u = ITERARGB.u;                                             \
		else                                            /* color0 RGB */        \
			c_local.u = (VV)->reg[color0].u;                                    \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (!(TEXELARGB.rgb.a & 0x80))                  /* iterated RGB */      \
			c_local.u = ITERARGB.u;                                             \
		else                                            /* color0 RGB */        \
			c_local.u = (VV)->reg[color0].u;                                    \
	}                                                                           \
																				\
	/* compute a_local */                                                       \
	switch (FBZCP_CCA_LOCALSELECT(FBZCOLORPATH))                                \
	{                                                                           \
		default:                                                                \
		case 0:     /* iterated alpha */                                        \
			c_local.rgb.a = ITERARGB.rgb.a;                                     \
			break;                                                              \
																				\
		case 1:     /* color0 alpha */                                          \
			c_local.rgb.a = (VV)->reg[color0].rgb.a;                            \
			break;                                                              \
																				\
		case 2:     /* clamped iterated Z[27:20] */                             \
		{                                                                       \
			int temp;                                                           \
			CLAMPED_Z(ITERZ, FBZCOLORPATH, temp);                               \
			c_local.rgb.a = (uint8_t)temp;                                        \
			break;                                                              \
		}                                                                       \
																				\
		case 3:     /* clamped iterated W[39:32] */                             \
		{                                                                       \
			int temp;                                                           \
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */ \
			c_local.rgb.a = (uint8_t)temp;                                        \
			break;                                                              \
		}                                                                       \
	}                                                                           \
																				\
	/* select zero or a_other */                                                \
	if (!FBZCP_CCA_ZERO_OTHER(FBZCOLORPATH))                                \
		a = c_other.rgb.a;                                                      \
	else                                                                        \
		a = 0;                                                                  \
																				\
	/* subtract a_local */                                                      \
	if (FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH))                                     \
		a -= c_local.rgb.a;                                                     \
																				\
	/* blend alpha */                                                           \
	switch (FBZCP_CCA_MSELECT(FBZCOLORPATH))                                    \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* 0 */                                                     \
			blenda = 0;                                                         \
			break;                                                              \
																				\
		case 1:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blenda = c_other.rgb.a;                                             \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 4:     /* texture alpha */                                         \
			blenda = TEXELARGB.rgb.a;                                           \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the alpha blend */                                               \
	if (!FBZCP_CCA_REVERSE_BLEND(FBZCOLORPATH))                                 \
		blenda ^= 0xff;                                                         \
																				\
	/* do the blend */                                                          \
	a = (a * (blenda + 1)) >> 8;                                                \
																				\
	/* add clocal or alocal to alpha */                                         \
	if (FBZCP_CCA_ADD_ACLOCAL(FBZCOLORPATH))                                    \
		a += c_local.rgb.a;                                                     \
																				\
	/* clamp */                                                                 \
	CLAMP(a, 0x00, 0xff);                                                       \
																				\
	/* invert */                                                                \
	if (FBZCP_CCA_INVERT_OUTPUT(FBZCOLORPATH))                                  \
		a ^= 0xff;                                                              \
																				\
	/* handle alpha test */                                                     \
	APPLY_ALPHATEST(VV, STATS, ALPHAMODE, a);                                   \
																				\
																				\
	/* select zero or c_other */                                                \
	if (FBZCP_CC_ZERO_OTHER(FBZCOLORPATH) == 0)                                 \
	{                                                                           \
		r = c_other.rgb.r;                                                      \
		g = c_other.rgb.g;                                                      \
		b = c_other.rgb.b;                                                      \
	}                                                                           \
	else                                                                        \
		r = g = b = 0;                                                          \
																				\
	/* subtract c_local */                                                      \
	if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH))                                      \
	{                                                                           \
		r -= c_local.rgb.r;                                                     \
		g -= c_local.rgb.g;                                                     \
		b -= c_local.rgb.b;                                                     \
	}                                                                           \
																				\
	/* blend RGB */                                                             \
	switch (FBZCP_CC_MSELECT(FBZCOLORPATH))                                     \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* 0 */                                                     \
			blendr = blendg = blendb = 0;                                       \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blendr = c_local.rgb.r;                                             \
			blendg = c_local.rgb.g;                                             \
			blendb = c_local.rgb.b;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blendr = blendg = blendb = c_other.rgb.a;                           \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blendr = blendg = blendb = c_local.rgb.a;                           \
			break;                                                              \
																				\
		case 4:     /* texture alpha */                                         \
			blendr = blendg = blendb = TEXELARGB.rgb.a;                         \
			break;                                                              \
																				\
		case 5:     /* texture RGB (Voodoo 2 only) */                           \
			blendr = TEXELARGB.rgb.r;                                           \
			blendg = TEXELARGB.rgb.g;                                           \
			blendb = TEXELARGB.rgb.b;                                           \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the RGB blend */                                                 \
	if (!FBZCP_CC_REVERSE_BLEND(FBZCOLORPATH))                                  \
	{                                                                           \
		blendr ^= 0xff;                                                         \
		blendg ^= 0xff;                                                         \
		blendb ^= 0xff;                                                         \
	}                                                                           \
																				\
	/* do the blend */                                                          \
	r = (r * (blendr + 1)) >> 8;                                                \
	g = (g * (blendg + 1)) >> 8;                                                \
	b = (b * (blendb + 1)) >> 8;                                                \
																				\
	/* add clocal or alocal to RGB */                                           \
	switch (FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH))                                 \
	{                                                                           \
		case 3:     /* reserved */                                              \
		case 0:     /* nothing */                                               \
			break;                                                              \
																				\
		case 1:     /* add c_local */                                           \
			r += c_local.rgb.r;                                                 \
			g += c_local.rgb.g;                                                 \
			b += c_local.rgb.b;                                                 \
			break;                                                              \
																				\
		case 2:     /* add_alocal */                                            \
			r += c_local.rgb.a;                                                 \
			g += c_local.rgb.a;                                                 \
			b += c_local.rgb.a;                                                 \
			break;                                                              \
	}                                                                           \
																				\
	/* clamp */                                                                 \
	CLAMP(r, 0x00, 0xff);                                                       \
	CLAMP(g, 0x00, 0xff);                                                       \
	CLAMP(b, 0x00, 0xff);                                                       \
																				\
	/* invert */                                                                \
	if (FBZCP_CC_INVERT_OUTPUT(FBZCOLORPATH))                                   \
	{                                                                           \
		r ^= 0xff;                                                              \
		g ^= 0xff;                                                              \
		b ^= 0xff;                                                              \
	}                                                                           \
}                                                                               \
while (0)

inline bool ATTR_FORCE_INLINE voodoo_device::combineColor(voodoo_device *vd, stats_block *STATS, uint32_t FBZCOLORPATH, uint32_t FBZMODE,
													rgbaint_t TEXELARGB, int32_t ITERZ, int64_t ITERW, rgbaint_t &srcColor)
{
	rgbaint_t c_other;
	rgbaint_t c_local;
	rgbaint_t blend_color, blend_factor;
	rgbaint_t add_val;

	/* compute c_other */
	switch (FBZCP_CC_RGBSELECT(FBZCOLORPATH))
	{
		case 0:     /* iterated RGB */
			c_other.set(srcColor);
			break;

		case 1:     /* texture RGB */
			c_other.set(TEXELARGB);
			break;

		case 2:     /* color1 RGB */
			c_other.set(vd->reg[color1].u);
			break;

		default:    /* reserved - voodoo3 LFB RGB */
			c_other.zero();
			break;
	}

	/* handle chroma key */
	if (FBZMODE_ENABLE_CHROMAKEY(FBZMODE))
		if (!chromaKeyTest(vd, STATS, FBZMODE, c_other))
			return false;
	//APPLY_CHROMAKEY(vd->m_vds, STATS, FBZMODE, c_other);

	/* compute a_other */
	switch (FBZCP_CC_ASELECT(FBZCOLORPATH))
	{
		case 0:     /* iterated alpha */
			c_other.merge_alpha16(srcColor);
			break;

		case 1:     /* texture alpha */
			c_other.merge_alpha16(TEXELARGB);
			break;

		case 2:     /* color1 alpha */
			c_other.set_a16(vd->reg[color1].rgb.a);
			break;

		default:    /* reserved - voodoo3  LFB Alpha*/
			c_other.zero_alpha();
			break;
	}

	/* handle alpha mask */
	if (FBZMODE_ENABLE_ALPHA_MASK(FBZMODE))
		if (!alphaMaskTest(STATS, FBZMODE, c_other.get_a()))
			return false;
	//APPLY_ALPHAMASK(vd->m_vds, STATS, FBZMODE, c_other.rgb.a);


	/* compute c_local */
	if (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) == 0)
	{
		if (FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0)    /* iterated RGB */
			c_local.set(srcColor);
		else                                            /* color0 RGB */
			c_local.set(vd->reg[color0].u);
	}
	else
	{
		if (!(TEXELARGB.get_a() & 0x80))                  /* iterated RGB */
			c_local.set(srcColor);
		else                                            /* color0 RGB */
			c_local.set(vd->reg[color0].u);
	}

	/* compute a_local */
	switch (FBZCP_CCA_LOCALSELECT(FBZCOLORPATH))
	{
		default:
		case 0:     /* iterated alpha */
			c_local.merge_alpha16(srcColor);
			break;

		case 1:     /* color0 alpha */
			c_local.set_a16(vd->reg[color0].rgb.a);
			break;

		case 2:     /* clamped iterated Z[27:20] */
		{
			int temp;
			CLAMPED_Z(ITERZ, FBZCOLORPATH, temp);
			c_local.set_a16((uint8_t) temp);
			break;
		}

		case 3:     /* clamped iterated W[39:32] */
		{
			int temp;
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */
			c_local.set_a16((uint8_t) temp);
			break;
		}
	}

	/* select zero or c_other */
	if (FBZCP_CC_ZERO_OTHER(FBZCOLORPATH))
		//r = g = b = 0;
		blend_color.zero();
	else
		blend_color.set(c_other);

	/* select zero or a_other */
	if (FBZCP_CCA_ZERO_OTHER(FBZCOLORPATH))
		blend_color.zero_alpha();
	else
		blend_color.merge_alpha16(c_other);

	/* subtract a/c_local */
	if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH) || (FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH)))
	{
		rgbaint_t sub_val;

		if (!FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.zero();
		else
			sub_val.set(c_local);

		if (!FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.zero_alpha();
		else
			sub_val.merge_alpha16(c_local);

		blend_color.sub(sub_val);
	}

	/* blend RGB */
	switch (FBZCP_CC_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			blend_factor.zero();
			break;

		case 1:     /* c_local */
			blend_factor.set(c_local);
			break;

		case 2:     /* a_other */
			blend_factor.set(c_other.select_alpha32());
			break;

		case 3:     /* a_local */
			blend_factor.set(c_local.select_alpha32());
			break;

		case 4:     /* texture alpha */
			blend_factor.set(TEXELARGB.select_alpha32());
			break;

		case 5:     /* texture RGB (Voodoo 2 only) */
			blend_factor.set(TEXELARGB);
			break;
	}

	/* blend alpha */
	switch (FBZCP_CCA_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			blend_factor.zero_alpha();
			break;

		case 1:     /* a_local */
		case 3:     /* a_local */
			blend_factor.merge_alpha16(c_local);
			break;

		case 2:     /* a_other */
			blend_factor.merge_alpha16(c_other);
			break;

		case 4:     /* texture alpha */
			blend_factor.merge_alpha16(TEXELARGB);
			break;
	}

	/* reverse the RGB blend */
	if (!FBZCP_CC_REVERSE_BLEND(FBZCOLORPATH))
		blend_factor.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	/* reverse the alpha blend */
	if (!FBZCP_CCA_REVERSE_BLEND(FBZCOLORPATH))
		blend_factor.xor_imm_rgba(0xff, 0, 0, 0);

	/* do the blend */
	//color.rgb.a = (color.rgb.a * (blenda + 1)) >> 8;
	//color.rgb.r = (color.rgb.r * (blendr + 1)) >> 8;
	//color.rgb.g = (color.rgb.g * (blendg + 1)) >> 8;
	//color.rgb.b = (color.rgb.b * (blendb + 1)) >> 8;

	/* add clocal or alocal to RGB */
	switch (FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH))
	{
		case 3:     /* reserved */
		case 0:     /* nothing */
			add_val.zero();
			break;

		case 1:     /* add c_local */
			add_val.set(c_local);
			break;

		case 2:     /* add_alocal */
			add_val.set(c_local.select_alpha32());
			break;
	}

	/* add clocal or alocal to alpha */
	if (!FBZCP_CCA_ADD_ACLOCAL(FBZCOLORPATH))
		add_val.zero_alpha();
	else
		//color.rgb.a += c_local.rgb.a;
		add_val.merge_alpha16(c_local);

	/* clamp */
	//CLAMP(color.rgb.a, 0x00, 0xff);
	//CLAMP(color.rgb.r, 0x00, 0xff);
	//CLAMP(color.rgb.g, 0x00, 0xff);
	//CLAMP(color.rgb.b, 0x00, 0xff);
	blend_factor.add_imm(1);
	blend_color.scale_add_and_clamp(blend_factor, add_val);

	/* invert */
	if (FBZCP_CCA_INVERT_OUTPUT(FBZCOLORPATH))
		blend_color.xor_imm_rgba(0xff, 0, 0, 0);
	/* invert */
	if (FBZCP_CC_INVERT_OUTPUT(FBZCOLORPATH))
		blend_color.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	srcColor.set(blend_color);

	return true;
}



/*************************************
 *
 *  Rasterizer generator macro
 *
 *************************************/

#define RASTERIZER(name, TMUS, FBZCOLORPATH, FBZMODE, ALPHAMODE, FOGMODE, TEXMODE0, TEXMODE1) \
																				\
void voodoo_device::raster_##name(void *destbase, int32_t y, const poly_extent *extent, const void *extradata, int threadid) \
{                                                                               \
	const poly_extra_data *extra = (const poly_extra_data *)extradata;          \
	voodoo_device *vd = extra->device; \
	stats_block *stats = &vd->thread_stats[threadid];                            \
	DECLARE_DITHER_POINTERS;                                                    \
	int32_t startx = extent->startx;                                              \
	int32_t stopx = extent->stopx;                                                \
	rgbaint_t iterargb, iterargbDelta;                                           \
	int32_t iterz;                                                                \
	int64_t iterw;                                                                \
	tmu_state::stw_t iterstw0, iterstw1;                                                     \
	tmu_state::stw_t deltastw0, deltastw1;                                                   \
	uint16_t *depth;                                                              \
	uint16_t *dest;                                                               \
	int32_t dx, dy;                                                               \
	int32_t scry;                                                                 \
	int32_t x;                                                                    \
																				\
	/* determine the screen Y */                                                \
	scry = y;                                                                   \
	if (FBZMODE_Y_ORIGIN(FBZMODE))                                              \
		scry = (vd->fbi.yorigin - y);                                    \
																				\
	/* compute dithering */                                                     \
	COMPUTE_DITHER_POINTERS(FBZMODE, y, FOGMODE);                               \
																				\
	/* apply clipping */                                                        \
	if (FBZMODE_ENABLE_CLIPPING(FBZMODE))                                       \
	{                                                                           \
		int32_t tempclip;                                                         \
																				\
		/* Y clipping buys us the whole scanline */                             \
		if (scry < ((vd->reg[clipLowYHighY].u >> 16) & 0x3ff) ||                 \
			scry >= (vd->reg[clipLowYHighY].u & 0x3ff))                          \
		{                                                                       \
			stats->pixels_in += stopx - startx;                                 \
			stats->clip_fail += stopx - startx;                                 \
			return;                                                             \
		}                                                                       \
																				\
		/* X clipping */                                                        \
		tempclip = vd->reg[clipLeftRight].u & 0x3ff;                             \
		/* Check for start outsize of clipping boundary */                        \
		if (startx >= tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += stopx - startx;                               \
			vd->stats.total_clipped += stopx - startx;                         \
			return;                                               \
		}                                                                       \
		if (stopx > tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += stopx - tempclip;                               \
			vd->stats.total_clipped += stopx - tempclip;                         \
			stopx = tempclip;                                               \
		}                                                                       \
		tempclip = (vd->reg[clipLeftRight].u >> 16) & 0x3ff;                     \
		if (startx < tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += tempclip - startx;                              \
			vd->stats.total_clipped += tempclip - startx;                        \
			startx = tempclip;                                                  \
		}                                                                       \
	}                                                                           \
																				\
	/* get pointers to the target buffer and depth buffer */                    \
	dest = (uint16_t *)destbase + scry * vd->fbi.rowpixels;                        \
	depth = (vd->fbi.auxoffs != ~0) ? ((uint16_t *)(vd->fbi.ram + vd->fbi.auxoffs) + scry * vd->fbi.rowpixels) : nullptr; \
																				\
	/* compute the starting parameters */                                       \
	dx = startx - (extra->ax >> 4);                                             \
	dy = y - (extra->ay >> 4);                                                  \
	int32_t iterr = extra->startr + dy * extra->drdy + dx * extra->drdx;                \
	int32_t iterg = extra->startg + dy * extra->dgdy + dx * extra->dgdx;                \
	int32_t iterb = extra->startb + dy * extra->dbdy + dx * extra->dbdx;                \
	int32_t itera = extra->starta + dy * extra->dady + dx * extra->dadx;                \
	iterargb.set(itera, iterr, iterg, iterb); \
	iterargbDelta.set(extra->dadx, extra->drdx, extra->dgdx, extra->dbdx); \
	iterz = extra->startz + dy * extra->dzdy + dx * extra->dzdx;                \
	iterw = extra->startw + dy * extra->dwdy + dx * extra->dwdx;                \
	if (TMUS >= 1)                                                              \
	{                                                                           \
		iterstw0.set(                                                           \
		extra->starts0 + dy * extra->ds0dy + dx * extra->ds0dx,        \
		extra->startt0 + dy * extra->dt0dy + dx * extra->dt0dx,        \
		extra->startw0 + dy * extra->dw0dy +   dx * extra->dw0dx);     \
		deltastw0.set(extra->ds0dx, extra->dt0dx, extra->dw0dx);       \
	}                                                                           \
	if (TMUS >= 2)                                                              \
	{                                                                           \
		iterstw1.set(                                                           \
		extra->starts1 + dy * extra->ds1dy + dx * extra->ds1dx,        \
		extra->startt1 + dy * extra->dt1dy + dx * extra->dt1dx,        \
		extra->startw1 + dy * extra->dw1dy + dx * extra->dw1dx);       \
		deltastw1.set(extra->ds1dx, extra->dt1dx, extra->dw1dx);       \
	}                                                                           \
	extra->info->hits++;                                                        \
	/* loop in X */                                                             \
	for (x = startx; x < stopx; x++)                                            \
	{                                                                           \
		rgbaint_t texel(0);                                                \
		rgbaint_t color, preFog;                                                \
																				\
		/* pixel pipeline part 1 handles depth setup and stippling */         \
		PIXEL_PIPELINE_BEGIN(vd, stats, x, y, FBZCOLORPATH, FBZMODE, iterz, iterw); \
		/* depth testing */         \
		if (FBZMODE_ENABLE_DEPTHBUF(FBZMODE))                                                  \
			if (!depthTest((uint16_t) vd->reg[zaColor].u, stats, depth[x], FBZMODE, biasdepth)) \
				goto skipdrawdepth; \
																				\
		/* run the texture pipeline on TMU1 to produce a value in texel */      \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 2 && vd->tmu[1].lodmin < (8 << 8))                    {       \
			int32_t tmp; \
			const rgbaint_t texelZero(0);  \
			texel = vd->tmu[1].genTexture(x, dither4, TEXMODE1, vd->tmu[1].lookup, extra->lodbase1, \
														iterstw1, tmp); \
			texel = vd->tmu[1].combineTexture(TEXMODE1, texel, texelZero, tmp); \
		} \
		/* run the texture pipeline on TMU0 to produce a final */               \
		/* result in texel */                                                   \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 1 && vd->tmu[0].lodmin < (8 << 8))                           \
		{                                                                   \
			if (!vd->send_config)                                                \
			{                                                                   \
				int32_t lod0; \
				rgbaint_t texelT0;                                                \
				texelT0 = vd->tmu[0].genTexture(x, dither4, TEXMODE0, vd->tmu[0].lookup, extra->lodbase0, \
																iterstw0, lod0); \
				texel = vd->tmu[0].combineTexture(TEXMODE0, texelT0, texel, lod0); \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel.set(vd->tmu_config);                                              \
			}                                                                   \
		}                                                                   \
																				\
		/* colorpath pipeline selects source colors and does blending */        \
		color = clampARGB(iterargb, FBZCOLORPATH);                                      \
		if (!combineColor(vd, stats, FBZCOLORPATH, FBZMODE, texel, iterz, iterw, color)) \
			goto skipdrawdepth;                                                          \
		/* handle alpha test */                                                          \
		if (ALPHAMODE_ALPHATEST(ALPHAMODE))                                              \
			if (!alphaTest(vd->reg[alphaMode].rgb.a, stats, ALPHAMODE, color.get_a()))   \
				goto skipdrawdepth;                                                      \
																						 \
		/* perform fogging */                                                            \
		preFog.set(color);                                                               \
		if (FOGMODE_ENABLE_FOG(FOGMODE))                                                                         \
			applyFogging(vd, FBZMODE, FOGMODE, FBZCOLORPATH, x, dither4, wfloat, color, iterz, iterw, iterargb); \
																												 \
		/* perform alpha blending */                                                \
		if (ALPHAMODE_ALPHABLEND(ALPHAMODE))                                                            \
			alphaBlend(FBZMODE, ALPHAMODE, x, dither, dest[x], depth, preFog, color, vd->fbi.rgb565); \
																				\
		/* pixel pipeline part 2 handles final output */        \
		PIXEL_PIPELINE_END(stats, dither_lookup, x, dest, depth, FBZMODE);  \
																				\
		/* update the iterated parameters */                                    \
		iterargb += iterargbDelta;                                              \
		iterz += extra->dzdx;                                                   \
		iterw += extra->dwdx;                                                   \
		if (TMUS >= 1)                                                          \
		{                                                                       \
			iterstw0.add(deltastw0);                                            \
		}                                                                       \
		if (TMUS >= 2)                                                          \
		{                                                                       \
			iterstw1.add(deltastw1);                                            \
		}                                                                       \
	}                                                                           \
}


// ******************************************************************************************************************************
// Computes a log2 of a 16.32 value to 2 fractional bits of precision.
// The return value is coded as a 24.8 value.
// The maximum error using a 4 bit lookup from the mantissa is 0.0875, which is less than 1/2 lsb (0.125) for 2 bits of fraction.
// An offset of  +(56 << 8) is added for alignment in multi_reciplog
// ******************************************************************************************************************************
inline int32_t ATTR_FORCE_INLINE voodoo_device::tmu_state::new_log2(double &value, const int &offset)
{
	static const int32_t new_log2_table[16] = {0, 22, 44, 63, 82, 100, 118, 134, 150, 165, 179, 193, 207, 220, 232, 244};
	uint64_t ival = *((uint64_t *)&value);
	// Return 0 if negative
	if (ival & ((uint64_t)1 << 63))
		return 0;
	// We zero the result if negative so don't worry about the sign bit
	int32_t exp = (ival>>52);
	exp -= 1023+32-offset;
	exp <<= 8;
	uint32_t addr = (uint64_t)(ival>>48) & 0xf;
	exp += new_log2_table[addr];
	return exp;
}

inline rgbaint_t ATTR_FORCE_INLINE voodoo_device::tmu_state::genTexture(int32_t x, const uint8_t *dither4, const uint32_t TEXMODE, rgb_t *LOOKUP, int32_t LODBASE, const stw_t &iterstw, int32_t &lod)
{
	rgbaint_t result;
	int32_t s, t, ilod;

	/* determine the S/T/LOD values for this texture */
	lod = (LODBASE);
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))
	{
		int32_t wLog;
		iterstw.calc_stow(s, t, wLog);
		lod += wLog;
	}
	else
	{
		iterstw.get_st_shiftr(s, t, (14 + 10));
	}

	/* clamp W */
	if (TEXMODE_CLAMP_NEG_W(TEXMODE) && iterstw.is_w_neg())
	{
		s = t = 0;
	}

	/* clamp the LOD */
	lod += lodbias;
	if (TEXMODE_ENABLE_LOD_DITHER(TEXMODE))
		lod += dither4[x&3] << 4;
	CLAMP(lod, lodmin, lodmax);

	/* now the LOD is in range; if we don't own this LOD, take the next one */
	ilod = lod >> 8;
	if (!((lodmask >> ilod) & 1))
		ilod++;

	/* fetch the texture base */
	uint32_t texbase = lodoffset[ilod];

	/* compute the maximum s and t values at this LOD */
	int32_t smax = wmask >> ilod;
	int32_t tmax = hmask >> ilod;

	/* determine whether we are point-sampled or bilinear */
	if ((lod == lodmin && !TEXMODE_MAGNIFICATION_FILTER(TEXMODE)) ||
		(lod != lodmin && !TEXMODE_MINIFICATION_FILTER(TEXMODE)))
	{
		/* point sampled */

		uint32_t texel0;

		/* adjust S/T for the LOD and strip off the fractions */
		s >>= ilod + (18-10);
		t >>= ilod + (18-10);

		/* clamp/wrap S/T if necessary */
		if (TEXMODE_CLAMP_S(TEXMODE))
			CLAMP(s, 0, smax);
		if (TEXMODE_CLAMP_T(TEXMODE))
			CLAMP(t, 0, tmax);
		s &= smax;
		t &= tmax;
		t *= smax + 1;

		/* fetch texel data */
		if (TEXMODE_FORMAT(TEXMODE) < 8)
		{
			texel0 = *(uint8_t *)&ram[(texbase + t + s) & mask];
			result.set((LOOKUP)[texel0]);
		}
		else
		{
			texel0 = *(uint16_t *)&ram[(texbase + 2*(t + s)) & mask];
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12)
				result.set((LOOKUP)[texel0]);
			else
				result.set(((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16));
		}
	}
	else
	{
		/* bilinear filtered */

		uint32_t texel0, texel1, texel2, texel3;
		uint32_t sfrac, tfrac;
		int32_t s1, t1;

		/* adjust S/T for the LOD and strip off all but the low 8 bits of */
		/* the fraction */
		s >>= ilod; // + (10-10);
		t >>= ilod; // + (10-10);

		/* also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel */
		s -= 0x80;
		t -= 0x80;

		/* extract the fractions */
		sfrac = s & bilinear_mask;
		tfrac = t & bilinear_mask;

		/* now toss the rest */
		s >>= 8;
		t >>= 8;
		s1 = s + 1;
		t1 = t + 1;

		/* clamp/wrap S/T if necessary */
		if (TEXMODE_CLAMP_S(TEXMODE))
		{
			if (s < 0) {
				s = 0;
				s1 = 0;
			} else if (s >= smax) {
				s = smax;
				s1 = smax;
			}
			//CLAMP(s, 0, smax);
			//CLAMP(s1, 0, smax);
		} else {
			s &= smax;
			s1 &= smax;
		}

		if (TEXMODE_CLAMP_T(TEXMODE))
		{
			if (t < 0) {
				t = 0;
				t1 = 0;
			} else if (t >= tmax) {
				t = tmax;
				t1 = tmax;
			}
			//CLAMP(t, 0, tmax);
			//CLAMP(t1, 0, tmax);
		} else {
			t &= tmax;
			t1 &= tmax;
		}
		t *= smax + 1;
		t1 *= smax + 1;

		/* fetch texel data */
		if (TEXMODE_FORMAT(TEXMODE) < 8)
		{
			texel0 = *(uint8_t *)&ram[(texbase + t + s) & mask];
			texel1 = *(uint8_t *)&ram[(texbase + t + s1) & mask];
			texel2 = *(uint8_t *)&ram[(texbase + t1 + s) & mask];
			texel3 = *(uint8_t *)&ram[(texbase + t1 + s1) & mask];
			texel0 = (LOOKUP)[texel0];
			texel1 = (LOOKUP)[texel1];
			texel2 = (LOOKUP)[texel2];
			texel3 = (LOOKUP)[texel3];
		}
		else
		{
			texel0 = *(uint16_t *)&ram[(texbase + 2*(t + s)) & mask];
			texel1 = *(uint16_t *)&ram[(texbase + 2*(t + s1)) & mask];
			texel2 = *(uint16_t *)&ram[(texbase + 2*(t1 + s)) & mask];
			texel3 = *(uint16_t *)&ram[(texbase + 2*(t1 + s1)) & mask];
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12)
			{
				texel0 = (LOOKUP)[texel0];
				texel1 = (LOOKUP)[texel1];
				texel2 = (LOOKUP)[texel2];
				texel3 = (LOOKUP)[texel3];
			}
			else
			{
				texel0 = ((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16);
				texel1 = ((LOOKUP)[texel1 & 0xff] & 0xffffff) | ((texel1 & 0xff00) << 16);
				texel2 = ((LOOKUP)[texel2 & 0xff] & 0xffffff) | ((texel2 & 0xff00) << 16);
				texel3 = ((LOOKUP)[texel3 & 0xff] & 0xffffff) | ((texel3 & 0xff00) << 16);
			}
		}

		/* weigh in each texel */

		result.bilinear_filter_rgbaint(texel0, texel1, texel2, texel3, sfrac, tfrac);
	}
	return result;
}

inline rgbaint_t ATTR_FORCE_INLINE voodoo_device::tmu_state::combineTexture(const uint32_t TEXMODE, const rgbaint_t& c_local, const rgbaint_t& c_other, int32_t lod)
{
	rgbaint_t blend_color, blend_factor;
	rgbaint_t add_val;

	/* select zero/other for RGB */
	if (TEXMODE_TC_ZERO_OTHER(TEXMODE))
		blend_color.zero();
	else
		blend_color.set(c_other);

	/* select zero/other for alpha */
	if (TEXMODE_TCA_ZERO_OTHER(TEXMODE))
		blend_color.zero_alpha();
	else
		blend_color.merge_alpha16(c_other);

	if (TEXMODE_TC_SUB_CLOCAL(TEXMODE) || TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
	{
		rgbaint_t sub_val;

		/* potentially subtract c_local */
		if (!TEXMODE_TC_SUB_CLOCAL(TEXMODE))
			sub_val.zero();
		else
			sub_val.set(c_local);

		if (!TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
			sub_val.zero_alpha();
		else
			sub_val.merge_alpha16(c_local);

		blend_color.sub(sub_val);
	}

	/* blend RGB */
	switch (TEXMODE_TC_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			blend_factor.zero();
			break;

		case 1:     /* c_local */
			blend_factor.set(c_local);
			break;

		case 2:     /* a_other */
			blend_factor.set(c_other.select_alpha32());
			break;

		case 3:     /* a_local */
			blend_factor.set(c_local.select_alpha32());
			break;

		case 4:     /* LOD (detail factor) */
			if (detailbias <= lod)
				blend_factor.zero();
			else
			{
				uint8_t tmp;
				tmp = (((detailbias - lod) << detailscale) >> 8);
				if (tmp > detailmax)
					tmp = detailmax;
				blend_factor.set_all(tmp);
			}
			break;

		case 5:     /* LOD fraction */
			blend_factor.set_all(lod & 0xff);
			break;
	}

	/* blend alpha */
	switch (TEXMODE_TCA_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			blend_factor.zero_alpha();
			break;

		case 1:     /* c_local */
			blend_factor.merge_alpha16(c_local);
			break;

		case 2:     /* a_other */
			blend_factor.merge_alpha16(c_other);
			break;

		case 3:     /* a_local */
			blend_factor.merge_alpha16(c_local);
			break;

		case 4:     /* LOD (detail factor) */
			if (detailbias <= lod)
				blend_factor.zero_alpha();
			else
			{
				uint8_t tmp;
				tmp = (((detailbias - lod) << detailscale) >> 8);
				if (tmp > detailmax)
					tmp = detailmax;
				blend_factor.set_a16(tmp);
			}
			break;

		case 5:     /* LOD fraction */
			blend_factor.set_a16(lod & 0xff);
			break;
	}

	/* reverse the RGB blend */
	if (!TEXMODE_TC_REVERSE_BLEND(TEXMODE))
		blend_factor.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	/* reverse the alpha blend */
	if (!TEXMODE_TCA_REVERSE_BLEND(TEXMODE))
		blend_factor.xor_imm_rgba(0xff, 0, 0, 0);

	/* do the blend */
	//tr = (tr * (blendr + 1)) >> 8;
	//tg = (tg * (blendg + 1)) >> 8;
	//tb = (tb * (blendb + 1)) >> 8;
	//ta = (ta * (blenda + 1)) >> 8;

	/* add clocal or alocal to RGB */
	switch (TEXMODE_TC_ADD_ACLOCAL(TEXMODE))
	{
		case 3:     /* reserved */
		case 0:     /* nothing */
			add_val.zero();
			break;

		case 1:     /* add c_local */
			add_val.set(c_local);
			break;

		case 2:     /* add_alocal */
			add_val.set(c_local.select_alpha32());
			break;
	}

	/* add clocal or alocal to alpha */
	if (!TEXMODE_TCA_ADD_ACLOCAL(TEXMODE))
		add_val.zero_alpha();
	else
		add_val.merge_alpha16(c_local);

	/* clamp */
	//result.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;
	//result.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;
	//result.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;
	//result.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;
	blend_factor.add_imm(1);
	blend_color.scale_add_and_clamp(blend_factor, add_val);

	/* invert */
	if (TEXMODE_TC_INVERT_OUTPUT(TEXMODE))
		blend_color.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	if (TEXMODE_TCA_INVERT_OUTPUT(TEXMODE))
		blend_color.xor_imm_rgba(0xff, 0, 0, 0);
	return blend_color;
}

#endif // MAME_VIDEO_VOODDEFS_IPP
