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

inline void voodoo_device::fifo_state::add(u32 data)
{
	/* compute the value of 'in' after we add this item */
	s32 next_in = in + 1;
	if (next_in >= size)
		next_in = 0;

	/* as long as it's not equal to the output pointer, we can do it */
	if (next_in != out)
	{
		base[in] = data;
		in = next_in;
	}
}


inline u32 voodoo_device::fifo_state::remove()
{
	u32 data = 0xffffffff;

	/* as long as we have data, we can do it */
	if (out != in)
	{
		s32 next_out;

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


inline s32 voodoo_device::fifo_state::items() const
{
	s32 items = in - out;
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

static inline s32 fast_reciplog(s64 value, s32 *log2)
{
	extern u32 voodoo_reciplog[];
	u32 temp, recip, rlog;
	u32 interp;
	u32 *table;
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
		temp = (u32)(value >> 16);
		exp -= 16;
	}
	else
		temp = (u32)value;

	/* if the resulting value is 0, the reciprocal is infinite */
	if (UNEXPECTED(temp == 0))
	{
		*log2 = 1000 << LOG_OUTPUT_PREC;
		return neg ? 0x80000000 : 0x7fffffff;
	}

	/* determine how many leading zeros in the value and shift it up high */
	lz = count_leading_zeros_32(temp);
	temp <<= lz;
	exp += lz;

	/* compute a pointer to the table entries we want */
	/* math is a bit funny here because we shift one less than we need to in order */
	/* to account for the fact that there are two u32's per table entry */
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

static inline s32 float_to_int32(u32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	s32 result = (data & 0x7fffff) | 0x800000;
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


static inline s64 float_to_int64(u32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	s64 result = (data & 0x7fffff) | 0x800000;
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

constexpr u32 voodoo_device::static_raster_info::compute_hash() const
{
	u32 result = eff_color_path;
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

namespace voodoo
{

class dither_helper
{
public:
	// constructor to pre-cache based on mode and Y coordinate
	dither_helper(int y, fbz_mode const fbzmode, fog_mode const fogmode = fog_mode(0)) :
		m_dither_lookup(nullptr),
		m_dither_raw(&s_dither_matrix_4x4[(y & 3) * 4]),
		m_dither_subtract(nullptr)
	{
		// still use a lookup for no dithering since it's rare and we
		// can avoid yet another conditional on the hot path
		if (!fbzmode.enable_dithering())
			m_dither_lookup = &s_nodither_lookup[0];
		else if (fbzmode.dither_type() == 0)
			m_dither_lookup = &s_dither4_lookup[(y & 3) << 11];
		else
			m_dither_lookup = &s_dither2_lookup[(y & 3) << 11];
	}

	// apply dithering to a pixel in separate R/G/B format and assemble as 5-6-5
	u16 pixel(s32 x, s32 r, s32 g, s32 b) const
	{
		u8 const *table = &m_dither_lookup[(x & 3) << 9];
		return (table[r] << 11) | (table[g + 256] << 5) | table[b];
	}

	// apply dithering to an rgbint_t pixel and assemble as 5-6-5
	u16 pixel(s32 x, rgbaint_t const &color) const
	{
		u8 const *table = &m_dither_lookup[(x & 3) << 9];
		return (table[color.get_r()] << 11) | (table[color.get_g() + 256] << 5) | table[color.get_b()];
	}

	// return the raw 4x4 dither pattern
	u32 raw(s32 x) const
	{
		return m_dither_raw[x & 3];
	}

	// return the subtractive dither value for alpha blending
	u32 subtract(s32 x) const
	{
		return (m_dither_subtract != nullptr) ? m_dither_subtract[x & 3] : 0;
	}

	// allocate and initialize static tables
	static void init_static()
	{
		// create static dithering tables
		s_dither4_lookup = std::make_unique<u8[]>(256*4*4*2);
		s_dither2_lookup = std::make_unique<u8[]>(256*4*4*2);
		s_nodither_lookup = std::make_unique<u8[]>(256*4*2);
		for (int val = 0; val < 256*4*4*2; val++)
		{
			int color = (val >> 0) & 0xff;
			int g = (val >> 8) & 1;
			int x = (val >> 9) & 3;
			int y = (val >> 11) & 3;

			if (!g)
			{
				s_dither4_lookup[val] = dither_rb(color, s_dither_matrix_4x4[y * 4 + x]);
				s_dither2_lookup[val] = dither_rb(color, s_dither_matrix_2x2[y * 4 + x]);
			}
			else
			{
				s_dither4_lookup[val] = dither_g(color, s_dither_matrix_4x4[y * 4 + x]);
				s_dither2_lookup[val] = dither_g(color, s_dither_matrix_2x2[y * 4 + x]);
			}
		}
		for (int val = 0; val < 256*4*2; val++)
		{
			int color = (val >> 0) & 0xff;
			int g = (val >> 8) & 1;

			if (!g)
				s_nodither_lookup[val] = color >> 3;
			else
				s_nodither_lookup[val] = color >> 2;
		}
	}

private:
	// hardware-verified equation for applying dither to the red/blue components
	static constexpr u8 dither_rb(u8 value, u8 dither)
	{
		return ((value << 1) - (value >> 4) + (value >> 7) + dither) >> (1+3);
	}

	// hardware-verified equation for applying dither to the green componenets
	static constexpr u8 dither_g(u8 value, u8 dither)
	{
		return ((value << 2) - (value >> 4) + (value >> 6) + dither) >> (2+2);
	}

	// internal state
	u8 const *m_dither_lookup;
	u8 const *m_dither_raw;
	u8 const *m_dither_subtract;

	// static tables
	static std::unique_ptr<u8[]> s_dither4_lookup;
	static std::unique_ptr<u8[]> s_dither2_lookup;
	static std::unique_ptr<u8[]> s_nodither_lookup;
	static u8 const s_dither_matrix_4x4[4*4];
	static u8 const s_dither_matrix_2x2[4*4];
	static u8 const s_dither_matrix_4x4_subtract[4*4];
	static u8 const s_dither_matrix_2x2_subtract[4*4];
};

}


/*************************************
 *
 *  Clamping macros
 *
 *************************************/

static inline rgbaint_t ATTR_FORCE_INLINE clamped_argb(const rgbaint_t &iterargb, voodoo::fbz_colorpath const fbzcp)
{
	rgbaint_t result(iterargb);
	result.shr_imm(12);

	// clamped case is easy
	if (fbzcp.rgbzw_clamp() != 0)
	{
		result.clamp_to_uint8();
		return result;
	}

	// for each component:
	//    result = val & 0xfff;
	//    if (result == 0xfff) result = 0;
	//    if (result == 0x100) result = 0xff;
	result.and_imm(0xfff);

	// check against 0xfff and force to 0
	rgbaint_t temp(result);
	temp.cmpeq_imm(0xfff);
	result.andnot_reg(temp);

	// check against 0x100 and force to 0xff
	temp.set(result);
	temp.cmpeq_imm(0x100);
	result.or_reg(temp);

	// always mask against 0xff
	result.and_imm(0xff);
	return result;
}

static inline s32 ATTR_FORCE_INLINE clamped_z(s32 iterz, voodoo::fbz_colorpath const fbzcp)
{
	s32 result = iterz >> 12;

	// clamped case is easy
	if (fbzcp.rgbzw_clamp() != 0)
		return std::clamp(result, 0, 0xffff);

	// non-clamped case has specific behaviors
	result &= 0xfffff;
	if (result == 0xfffff)
		return 0;
	if (result == 0x10000)
		return 0xffff;
	return result & 0xffff;
}

static inline s32 ATTR_FORCE_INLINE clamped_w(s64 iterw, voodoo::fbz_colorpath const fbzcp)
{
	s32 result = iterw >> 32;

	// clamped case is easy
	if (fbzcp.rgbzw_clamp() != 0)
		return std::clamp(result, 0, 0xff);

	// non-clamped case has specific behaviors
	result &= 0xffff;
	if (result == 0xffff)
		return 0;
	if (result == 0x100)
		return 0xff;
	return result & 0xff;
}



/*************************************
 *
 *  Chroma keying macro
 *
 *************************************/

#define APPLY_CHROMAKEY(STATS, FBZMODE, COLOR)                              \
do                                                                              \
{                                                                               \
	if (FBZMODE.enable_chromakey())                                      \
	{                                                                           \
		/* non-range version */                                                 \
		if (!CHROMARANGE_ENABLE(m_reg[chromaRange].u))                      \
		{                                                                       \
			if (((COLOR.u ^ m_reg[chromaKey].u) & 0xffffff) == 0)           \
			{                                                                   \
				(STATS)->chroma_fail++;                                         \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* tricky range version */                                              \
		else                                                                    \
		{                                                                       \
			s32 low, high, test;                                              \
			int results = 0;                                                    \
																				\
			/* check blue */                                                    \
			low = m_reg[chromaKey].rgb.b;                                   \
			high = m_reg[chromaRange].rgb.b;                                \
			test = COLOR.rgb.b;                                                 \
			results = (test >= low && test <= high);                            \
			results ^= CHROMARANGE_BLUE_EXCLUSIVE(m_reg[chromaRange].u);    \
			results <<= 1;                                                      \
																				\
			/* check green */                                                   \
			low = m_reg[chromaKey].rgb.g;                                   \
			high = m_reg[chromaRange].rgb.g;                                \
			test = COLOR.rgb.g;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_GREEN_EXCLUSIVE(m_reg[chromaRange].u);   \
			results <<= 1;                                                      \
																				\
			/* check red */                                                     \
			low = m_reg[chromaKey].rgb.r;                                   \
			high = m_reg[chromaRange].rgb.r;                                \
			test = COLOR.rgb.r;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_RED_EXCLUSIVE(m_reg[chromaRange].u);     \
																				\
			/* final result */                                                  \
			if (CHROMARANGE_UNION_MODE(m_reg[chromaRange].u))               \
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

inline bool ATTR_FORCE_INLINE voodoo_device::chroma_key_test(thread_stats_block &threadstats, rgbaint_t const &colorin)
{
	{
		rgb_union color;
		color.u = colorin.to_rgba();
		/* non-range version */
		if (!CHROMARANGE_ENABLE(m_reg[chromaRange].u))
		{
			if (((color.u ^ m_reg[chromaKey].u) & 0xffffff) == 0)
			{
				threadstats.chroma_fail++;
				return false;
			}
		}

		/* tricky range version */
		else
		{
			s32 low, high, test;
			int results;

			/* check blue */
			low = m_reg[chromaKey].rgb.b;
			high = m_reg[chromaRange].rgb.b;
			test = color.rgb.b;
			results = (test >= low && test <= high);
			results ^= CHROMARANGE_BLUE_EXCLUSIVE(m_reg[chromaRange].u);
			results <<= 1;

			/* check green */
			low = m_reg[chromaKey].rgb.g;
			high = m_reg[chromaRange].rgb.g;
			test = color.rgb.g;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_GREEN_EXCLUSIVE(m_reg[chromaRange].u);
			results <<= 1;

			/* check red */
			low = m_reg[chromaKey].rgb.r;
			high = m_reg[chromaRange].rgb.r;
			test = color.rgb.r;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_RED_EXCLUSIVE(m_reg[chromaRange].u);

			/* final result */
			if (CHROMARANGE_UNION_MODE(m_reg[chromaRange].u))
			{
				if (results != 0)
				{
					threadstats.chroma_fail++;
					return false;
				}
			}
			else
			{
				if (results == 7)
				{
					threadstats.chroma_fail++;
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

inline bool voodoo_device::alpha_mask_test(thread_stats_block &threadstats, u8 alpha)
{
	if ((alpha & 1) == 0)
	{
		threadstats.afunc_fail++;
		return false;
	}
	return true;
}

/*************************************
 *
 *  Alpha testing macro
 *
 *************************************/

inline bool ATTR_FORCE_INLINE voodoo_device::alpha_test(
	thread_stats_block &threadstats,
	voodoo::alpha_mode const alphamode,
	u8 alpha)
{
	switch (alphamode.alphafunction())
	{
		case 0:     // alphaOP = never
			threadstats.afunc_fail++;
			return false;

		case 1:     // alphaOP = less than
			if (alpha >= alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 2:     // alphaOP = equal
			if (alpha != alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 3:     // alphaOP = less than or equal
			if (alpha > alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 4:     // alphaOP = greater than
			if (alpha <= alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 5:     // alphaOP = not equal
			if (alpha == alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 6:     // alphaOP = greater than or equal
			if (alpha < alphamode.alpharef())
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 7:     // alphaOP = always
			break;
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
	if (ALPHAMODE.alphablend())                                        \
	{                                                                           \
		int dpix = dest[XX];                                                    \
		int dr, dg, db;                                                         \
		EXTRACT_565_TO_888(dpix, dr, dg, db);                                   \
		int da = FBZMODE.enable_alpha_planes() ? depth[XX] : 0xff;       \
		int sr = (RR);                                                          \
		int sg = (GG);                                                          \
		int sb = (BB);                                                          \
		int sa = (AA);                                                          \
		int ta;                                                                 \
																				\
		/* apply dither subtraction */                                          \
		if (FBZMODE.alpha_dither_subtract())                             \
		{                                                                       \
			/* look up the dither value from the appropriate matrix */          \
			int dith = dither.subtract(XX);                                        \
																				\
			/* subtract the dither value */                                     \
			dr = ((dr << 1) + 15 - dith) >> 1;                                  \
			dg = ((dg << 2) + 15 - dith) >> 2;                                  \
			db = ((db << 1) + 15 - dith) >> 1;                                  \
		}                                                                       \
																				\
		/* compute source portion */                                            \
		switch (ALPHAMODE.srcrgbblend())                               \
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
		switch (ALPHAMODE.dstrgbblend())                               \
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
		if (ALPHAMODE.srcalphablend() == 4)                            \
			(AA) = sa;                                                          \
																				\
		/* blend the dest alpha */                                              \
		if (ALPHAMODE.dstalphablend() == 4)                            \
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

static inline void ATTR_FORCE_INLINE alpha_blend(
		rgbaint_t &color,
		voodoo::fbz_mode const FBZMODE,
		voodoo::alpha_mode const ALPHAMODE,
		s32 x,
		voodoo::dither_helper const &dither,
		int dpix,
		u16 *depth,
		rgbaint_t const &prefog,
		rgb_t *convTable)
{
	{
		//int dpix = dest[XX];
		int dr, dg, db;
		//EXTRACT_565_TO_888(dpix, dr, dg, db);
		rgb_t drgb = convTable[dpix];
		drgb.expand_rgb(dr, dg, db);
		int da = FBZMODE.enable_alpha_planes() ? depth[x] : 0xff;
		//int sr = (RR);
		//int sg = (GG);
		//int sb = (BB);
		//int sa = (AA);
		int sa = color.get_a();
		int ta;
		int srcAlphaScale, destAlphaScale;
		rgbaint_t srcScale, destScale;

		/* apply dither subtraction */
		if (FBZMODE.alpha_dither_subtract())
		{
			/* look up the dither value from the appropriate matrix */
			int dith = dither.subtract(x & 3);

			/* subtract the dither value */
			dr += dith;
			db += dith;
			dg += dith >> 1;
		}

		/* blend the source alpha */
		if (ALPHAMODE.srcalphablend() == 4)
			srcAlphaScale = 256;
			//(AA) = sa;
		else
			srcAlphaScale = 0;

		/* compute source portion */
		switch (ALPHAMODE.srcrgbblend())
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
		if (ALPHAMODE.dstalphablend() == 4)
			destAlphaScale = 256;
			//(AA) += da;
		else
			destAlphaScale = 0;

		/* add in dest portion */
		switch (ALPHAMODE.dstrgbblend())
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
				destScale.set(color);
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
				destScale.sub(color);
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
				destScale.set(prefog);
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

		color.scale2_add_and_clamp(srcScale, destColor, destScale);
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

inline void ATTR_FORCE_INLINE voodoo_device::apply_fogging(
	rgbaint_t &color,
	voodoo::fbz_mode const fbzmode,
	voodoo::fog_mode const fogmode,
	voodoo::fbz_colorpath const fbzcp,
	s32 x,
	voodoo::dither_helper const &dither,
	s32 wfloat,
	s32 iterz,
	s64 iterw,
	rgbaint_t const &iterargb)
{
	// constant fog bypasses everything else
	rgbaint_t fog_color_local(m_reg[fogColor].u);
	if (fogmode.fog_constant())
	{
		// if fog_mult is 0, we add this to the original color
		if (fogmode.fog_mult() == 0)
		{
			fog_color_local.add(color);
			fog_color_local.clamp_to_uint8();
		}
	}

	// non-constant fog comes from several sources
	else
	{
		s32 fogblend = 0;

		// if fog_add is zero, we start with the fog color
		if (fogmode.fog_add())
			fog_color_local.zero();

		// if fog_mult is zero, we subtract the incoming color
		// Need to check this, manual states 9 bits
		if (!fogmode.fog_mult())
			fog_color_local.sub(color);

		// fog blending mode
		switch (fogmode.fog_zalpha())
		{
			case 0:     // fog table
			{
				s32 fog_depth = wfloat;

				// add the bias for fog selection
				if (fbzmode.enable_depth_bias())
					fog_depth = std::clamp(fog_depth + s16(m_reg[zaColor].u), 0, 0xffff);

				// perform the multiply against lower 8 bits of wfloat
				s32 delta = m_fbi.fogdelta[fog_depth >> 10];
				s32 deltaval = (delta & m_fbi.fogdelta_mask) * ((fog_depth >> 2) & 0xff);

				// fog zones allow for negating this value
				if (fogmode.fog_zones() && (delta & 2))
					deltaval = -deltaval;
				deltaval >>= 6;

				// apply dither
				if (fogmode.fog_dither())
					deltaval += dither.raw(x);
				deltaval >>= 4;

				// add to the blending factor
				fogblend = m_fbi.fogblend[fog_depth >> 10] + deltaval;
				break;
			}

			case 1:     // iterated A
				fogblend = iterargb.get_a();
				break;

			case 2:     // iterated Z
				fogblend = clamped_z(iterz, fbzcp) >> 8;
				break;

			case 3:     // iterated W - Voodoo 2 only
				fogblend = clamped_w(iterw, fbzcp);
				break;
		}

		// perform the blend
		fogblend++;

		// if fog_mult is 0, we add this to the original color
		fog_color_local.scale_imm_and_clamp(s16(fogblend));
		if (fogmode.fog_mult() == 0)
		{
			fog_color_local.add(color);
			fog_color_local.clamp_to_uint8();
		}
	}

	fog_color_local.merge_alpha16(color);
	color.set(fog_color_local);
}


/*************************************
 *
 *  Texture pipeline macro
 *
 *************************************/

#define TEXTURE_PIPELINE(TT, XX, DITHER, TEXMODE, COTHER, LOOKUP, LODBASE, ITERS, ITERT, ITERW, RESULT) \
do                                                                              \
{                                                                               \
	s32 blendr, blendg, blendb, blenda;                                       \
	s32 tr, tg, tb, ta;                                                       \
	s32 s, t, lod, ilod;                                                 \
	s32 smax, tmax;                                                           \
	u32 texbase;                                                             \
	rgb_union c_local;                                                          \
																				\
	/* determine the S/T/LOD values for this texture */                         \
	if (TEXMODE.enable_perspective())                                    \
	{                                                                           \
		if (USE_FAST_RECIP) {                                                     \
			const s32 oow = fast_reciplog((ITERW), &lod);                         \
			s = ((s64)oow * (ITERS)) >> 29;                                       \
			t = ((s64)oow * (ITERT)) >> 29;                                       \
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
	if (TEXMODE.clamp_neg_w() && (ITERW) < 0)                            \
		s = t = 0;                                                              \
																				\
	/* clamp the LOD */                                                         \
	lod += (TT)->lodbias;                                                       \
	if (TEXMODE.enable_lod_dither())                                     \
		lod += dither.raw(XX) << 4;                                          \
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
	if ((lod == (TT)->lodmin && !TEXMODE.magnification_filter()) ||      \
		(lod != (TT)->lodmin && !TEXMODE.minification_filter()))         \
	{                                                                           \
		/* point sampled */                                                     \
																				\
		u32 texel0;                                                          \
																				\
		/* adjust S/T for the LOD and strip off the fractions */                \
		s >>= ilod + 18;                                                        \
		t >>= ilod + 18;                                                        \
																				\
		/* clamp/wrap S/T if necessary */                                       \
		if (TEXMODE.clamp_s())                                           \
			CLAMP(s, 0, smax);                                                  \
		if (TEXMODE.clamp_t())                                           \
			CLAMP(t, 0, tmax);                                                  \
		s &= smax;                                                              \
		t &= tmax;                                                              \
		t *= smax + 1;                                                          \
																				\
		/* fetch texel data */                                                  \
		if (TEXMODE.format() < 8)                                        \
		{                                                                       \
			texel0 = *(u8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			c_local.u = (LOOKUP)[texel0];                                       \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(u16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			if (TEXMODE.format() >= 10 && TEXMODE.format() <= 12) \
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
		u32 texel0, texel1, texel2, texel3;                                  \
		u32 sfrac, tfrac;                                                    \
		s32 s1, t1;                                                           \
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
		if (TEXMODE.clamp_s())                                           \
		{                                                                       \
			CLAMP(s, 0, smax);                                                  \
			CLAMP(s1, 0, smax);                                                 \
		}                                                                       \
		if (TEXMODE.clamp_t())                                           \
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
		if (TEXMODE.format() < 8)                                        \
		{                                                                       \
			texel0 = *(u8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			texel1 = *(u8 *)&(TT)->ram[(texbase + t + s1) & (TT)->mask];     \
			texel2 = *(u8 *)&(TT)->ram[(texbase + t1 + s) & (TT)->mask];     \
			texel3 = *(u8 *)&(TT)->ram[(texbase + t1 + s1) & (TT)->mask];    \
			texel0 = (LOOKUP)[texel0];                                          \
			texel1 = (LOOKUP)[texel1];                                          \
			texel2 = (LOOKUP)[texel2];                                          \
			texel3 = (LOOKUP)[texel3];                                          \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(u16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			texel1 = *(u16 *)&(TT)->ram[(texbase + 2*(t + s1)) & (TT)->mask];\
			texel2 = *(u16 *)&(TT)->ram[(texbase + 2*(t1 + s)) & (TT)->mask];\
			texel3 = *(u16 *)&(TT)->ram[(texbase + 2*(t1 + s1)) & (TT)->mask];\
			if (TEXMODE.format() >= 10 && TEXMODE.format() <= 12) \
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
	if (!TEXMODE.tc_zero_other())                                        \
	{                                                                           \
		tr = COTHER.rgb.r;                                                      \
		tg = COTHER.rgb.g;                                                      \
		tb = COTHER.rgb.b;                                                      \
	}                                                                           \
	else                                                                        \
		tr = tg = tb = 0;                                                       \
																				\
	/* select zero/other for alpha */                                           \
	if (!TEXMODE.tca_zero_other())                                       \
		ta = COTHER.rgb.a;                                                      \
	else                                                                        \
		ta = 0;                                                                 \
																				\
	/* potentially subtract c_local */                                          \
	if (TEXMODE.tc_sub_clocal())                                         \
	{                                                                           \
		tr -= c_local.rgb.r;                                                    \
		tg -= c_local.rgb.g;                                                    \
		tb -= c_local.rgb.b;                                                    \
	}                                                                           \
	if (TEXMODE.tca_sub_clocal())                                        \
		ta -= c_local.rgb.a;                                                    \
																				\
	/* blend RGB */                                                             \
	switch (TEXMODE.tc_mselect())                                        \
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
	switch (TEXMODE.tca_mselect())                                       \
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
	if (!TEXMODE.tc_reverse_blend())                                     \
	{                                                                           \
		blendr ^= 0xff;                                                         \
		blendg ^= 0xff;                                                         \
		blendb ^= 0xff;                                                         \
	}                                                                           \
																				\
	/* reverse the alpha blend */                                               \
	if (!TEXMODE.tca_reverse_blend())                                    \
		blenda ^= 0xff;                                                         \
																				\
	/* do the blend */                                                          \
	tr = (tr * (blendr + 1)) >> 8;                                              \
	tg = (tg * (blendg + 1)) >> 8;                                              \
	tb = (tb * (blendb + 1)) >> 8;                                              \
	ta = (ta * (blenda + 1)) >> 8;                                              \
																				\
	/* add clocal or alocal to RGB */                                           \
	switch (TEXMODE.tc_add_aclocal())                                    \
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
	if (TEXMODE.tca_add_aclocal())                                       \
		ta += c_local.rgb.a;                                                    \
																				\
	/* clamp */                                                                 \
	RESULT.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;                      \
	RESULT.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;                      \
	RESULT.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;                      \
	RESULT.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;                      \
																				\
	/* invert */                                                                \
	if (TEXMODE.tc_invert_output())                                      \
		RESULT.u ^= 0x00ffffff;                                                 \
	if (TEXMODE.tca_invert_output())                                     \
		RESULT.rgb.a ^= 0xff;                                                   \
}                                                                               \
while (0)



/*************************************
 *
 *  Pixel pipeline macros
 *
 *************************************/

#define PIXEL_PIPELINE_BEGIN(THREADSTATS, XX, YY, FBZCOLORPATH, FBZMODE, ITERZ, ITERW)    \
do                                                                              \
{                                                                               \
	s32 depthval, wfloat, biasdepth;                                  \
																				\
	(THREADSTATS).pixels_in++;                                                       \
																				\
	/* apply clipping */                                                        \
	/* note that for perf reasons, we assume the caller has done clipping */    \
																				\
	/* handle stippling */                                                      \
	if (FBZMODE.enable_stipple())                                        \
	{                                                                           \
		/* rotate mode */                                                       \
		if (FBZMODE.stipple_pattern() == 0)                              \
		{                                                                       \
			m_reg[stipple].u = (m_reg[stipple].u << 1) | (m_reg[stipple].u >> 31);\
			if ((m_reg[stipple].u & 0x80000000) == 0)                         \
			{                                                                   \
				THREADSTATS.stipple_count++;                                     \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* pattern mode */                                                      \
		else                                                                    \
		{                                                                       \
			int stipple_index = (((YY) & 3) << 3) | (~(XX) & 7);                \
			if (((m_reg[stipple].u >> stipple_index) & 1) == 0)               \
			{                                                                   \
				THREADSTATS.stipple_count++;                                     \
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
		u32 temp = (u32)(ITERW);                                      \
		if (!(temp & 0xffff0000))                                               \
			wfloat = 0xffff;                                                    \
		else                                                                    \
		{                                                                       \
			int exp = count_leading_zeros_32(temp);                             \
			wfloat = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1;       \
		}                                                                       \
	}                                                                           \
																				\
	/* compute depth value (W or Z) for this pixel */                           \
	if (FBZMODE.wbuffer_select())                                   \
	{                                                                           \
		if (!FBZMODE.depth_float_select())                          \
			depthval = wfloat;                                                      \
		else                                                                        \
		{                                                                           \
			if ((ITERZ) & 0xf0000000)                                               \
				depthval = 0x0000;                                                  \
			else                                                                    \
			{                                                                       \
				u32 temp = (ITERZ << 4);                                       \
				if (!(temp & 0xffff0000))                                           \
					depthval = 0xffff;                                              \
				else                                                                \
				{                                                                   \
					int exp = count_leading_zeros_32(temp);                         \
					depthval = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1; \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		depthval = clamped_z(ITERZ, FBZCOLORPATH);                               \
	}                                                                           \
	/* add the bias */                                                          \
	biasdepth = depthval;                                                     \
	if (FBZMODE.enable_depth_bias())                                     \
	{                                                                           \
		biasdepth += (s16)m_reg[zaColor].u;                                \
		CLAMP(biasdepth, 0, 0xffff);                                             \
	}


inline bool ATTR_FORCE_INLINE voodoo_device::depth_test(
	thread_stats_block &threadstats,
	voodoo::fbz_mode const fbzmode,
	s32 depthdest,
	s32 biasdepth)
{
	// the source depth is either the iterated W/Z+bias or a constant value
	s32 depthsource = (fbzmode.depth_source_compare() == 0) ? biasdepth : u16(m_reg[zaColor].u);

	/* test against the depth buffer */
	switch (fbzmode.depth_function())
	{
		case 0:     /* depthOP = never */
			threadstats.zfunc_fail++;
			return false;

		case 1:     /* depthOP = less than */
			if (depthsource >= depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 2:     /* depthOP = equal */
			if (depthsource != depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 3:     /* depthOP = less than or equal */
			if (depthsource > depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 4:     /* depthOP = greater than */
			if (depthsource <= depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 5:     /* depthOP = not equal */
			if (depthsource == depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 6:     /* depthOP = greater than or equal */
			if (depthsource < depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 7:     /* depthOP = always */
			break;
	}
	return true;
}

#define PIXEL_PIPELINE_END(THREADSTATS, DITHER, XX, dest, depth, FBZMODE) \
																				\
/*	r = color.get_r(); g = color.get_g(); b = color.get_b(); */                    \
																				\
	/* write to framebuffer */                                                  \
	if (FBZMODE.rgb_buffer_mask())                                       \
		dest[XX] = dither.pixel(XX, color);                                    \
																				\
	/* write to aux buffer */                                                   \
	/*if (depth && FBZMODE_AUX_BUFFER_MASK(FBZMODE))*/                              \
	if (FBZMODE.aux_buffer_mask())                              \
	{                                                                           \
		if (FBZMODE.enable_alpha_planes() == 0)                          \
			depth[XX] = biasdepth;                                               \
		else                                                                    \
			depth[XX] = color.get_a();                                          \
	}                                                                           \
																				\
	/* track pixel writes to the frame buffer regardless of mask */             \
	(THREADSTATS).pixels_out++;                                                      \
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

        if (fbzmode.enable_chromakey() ||
            fbzcp.cc_zero_other() == 0)

    c_local_is_used:

        if (fbzcp.cc_sub_clocal() ||
            fbzcp.cc_mselect() == 1 ||
            fbzcp.cc_add_aclocal() == 1)

    NEEDS_ITER_RGB:

        if ((c_other_is_used && fbzcp.cc_rgbselect() == 0) ||
            (c_local_is_used && (fbzcp.cc_localselect_override() != 0 || fbzcp.cc_localselect() == 0))

    NEEDS_ITER_A:

        if ((a_other_is_used && fbzcp.cc_aselect() == 0) ||
            (a_local_is_used && fbzcp.cca_localselect() == 0))

    NEEDS_ITER_Z:

        if (fbzmode.wbuffer_select() == 0 ||
            fbzmode.depth_float_select() != 0 ||
            fbzcp.cca_localselect() == 2)


*/

inline bool ATTR_FORCE_INLINE voodoo_device::combine_color(
	rgbaint_t &color,
	thread_stats_block &threadstats,
	voodoo::fbz_colorpath const fbzcp,
	voodoo::fbz_mode const fbzmode,
	rgbaint_t texel,
	s32 iterz,
	s64 iterw)
{
	// compute c_other
	rgbaint_t c_other;
	switch (fbzcp.cc_rgbselect())
	{
		case 0:     // iterated RGB
			c_other.set(color);
			break;

		case 1:     // texture RGB
			c_other.set(texel);
			break;

		case 2:     // color1 RGB
			c_other.set(m_reg[color1].u);
			break;

		default:    // reserved - voodoo3 LFB RGB
			c_other.zero();
			break;
	}

	// handle chroma key
	if (fbzmode.enable_chromakey())
		if (!chroma_key_test(threadstats, c_other))
			return false;

	// compute a_other
	switch (fbzcp.cc_aselect())
	{
		case 0:     // iterated alpha
			c_other.merge_alpha16(color);
			break;

		case 1:     // texture alpha
			c_other.merge_alpha16(texel);
			break;

		case 2:     // color1 alpha
			c_other.set_a16(m_reg[color1].rgb.a);
			break;

		default:    // reserved - voodoo3  LFB Alpha
			c_other.zero_alpha();
			break;
	}

	// handle alpha mask
	if (fbzmode.enable_alpha_mask())
		if (!alpha_mask_test(threadstats, c_other.get_a()))
			return false;

	// compute c_local
	rgbaint_t c_local;
	if (fbzcp.cc_localselect_override() == 0)
	{
		if (fbzcp.cc_localselect() == 0)    // iterated RGB
			c_local.set(color);
		else                                // color0 RGB
			c_local.set(m_reg[color0].u);
	}
	else
	{
		if (!(texel.get_a() & 0x80))        // iterated RGB
			c_local.set(color);
		else                                // color0 RGB
			c_local.set(m_reg[color0].u);
	}

	// compute a_local
	switch (fbzcp.cca_localselect())
	{
		default:
		case 0:     // iterated alpha
			c_local.merge_alpha16(color);
			break;

		case 1:     // color0 alpha
			c_local.set_a16(m_reg[color0].rgb.a);
			break;

		case 2:     // clamped iterated Z[27:20]
			c_local.set_a16(u8(clamped_z(iterz, fbzcp) >> 8));
			break;

		case 3:     // clamped iterated W[39:32] (Voodoo 2 only)
			c_local.set_a16(u8(clamped_w(iterw, fbzcp)));
			break;
	}

	// select zero or c_other
	rgbaint_t blend_color;
	if (fbzcp.cc_zero_other())
		blend_color.zero();
	else
		blend_color.set(c_other);

	// select zero or a_other
	if (fbzcp.cca_zero_other())
		blend_color.zero_alpha();
	else
		blend_color.merge_alpha16(c_other);

	// subtract a/c_local
	if (fbzcp.cc_sub_clocal() || fbzcp.cca_sub_clocal())
	{
		rgbaint_t sub_val;

		if (!fbzcp.cc_sub_clocal())
			sub_val.zero();
		else
			sub_val.set(c_local);

		if (!fbzcp.cca_sub_clocal())
			sub_val.zero_alpha();
		else
			sub_val.merge_alpha16(c_local);

		blend_color.sub(sub_val);
	}

	// blend RGB
	rgbaint_t blend_factor;
	switch (fbzcp.cc_mselect())
	{
		default:    // reserved
		case 0:     // 0
			blend_factor.zero();
			break;

		case 1:     // c_local
			blend_factor.set(c_local);
			break;

		case 2:     // a_other
			blend_factor.set(c_other.select_alpha32());
			break;

		case 3:     // a_local
			blend_factor.set(c_local.select_alpha32());
			break;

		case 4:     // texture alpha
			blend_factor.set(texel.select_alpha32());
			break;

		case 5:     // texture RGB (Voodoo 2 only)
			blend_factor.set(texel);
			break;
	}

	// blend alpha
	switch (fbzcp.cca_mselect())
	{
		default:    // reserved
		case 0:     // 0
			blend_factor.zero_alpha();
			break;

		case 1:     // a_local
		case 3:     // a_local
			blend_factor.merge_alpha16(c_local);
			break;

		case 2:     // a_other
			blend_factor.merge_alpha16(c_other);
			break;

		case 4:     // texture alpha
			blend_factor.merge_alpha16(texel);
			break;
	}

	// reverse the RGB blend
	if (!fbzcp.cc_reverse_blend())
		blend_factor.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	// reverse the alpha blend
	if (!fbzcp.cca_reverse_blend())
		blend_factor.xor_imm_rgba(0xff, 0, 0, 0);

	// add clocal or alocal to RGB
	rgbaint_t add_val;
	switch (fbzcp.cc_add_aclocal())
	{
		case 3:     // reserved
		case 0:     // nothing
			add_val.zero();
			break;

		case 1:     // add c_local
			add_val.set(c_local);
			break;

		case 2:     // add_alocal
			add_val.set(c_local.select_alpha32());
			break;
	}

	// add clocal or alocal to alpha
	if (!fbzcp.cca_add_aclocal())
		add_val.zero_alpha();
	else
		add_val.merge_alpha16(c_local);

	// add and clamp
	blend_factor.add_imm(1);
	blend_color.scale_add_and_clamp(blend_factor, add_val);

	// invert
	if (fbzcp.cca_invert_output())
		blend_color.xor_imm_rgba(0xff, 0, 0, 0);
	if (fbzcp.cc_invert_output())
		blend_color.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	color.set(blend_color);
	return true;
}



/*************************************
 *
 *  Rasterizer generator macro
 *
 *************************************/

#define RASTERIZER(name, TMUS, _FBZCOLORPATH, _FBZMODE, _ALPHAMODE, _FOGMODE, _TEXMODE0, _TEXMODE1) \
																				\
void voodoo_device::raster_##name(s32 y, const voodoo_renderer::extent_t &extent, const poly_extra_data &extra, int threadid) \
{                                                                               \
	thread_stats_block &threadstats = m_thread_stats[threadid];                            \
	voodoo::texture_mode const texmode0(_TEXMODE0, (TMUS < 1) ? 0 : m_tmu[0].m_reg[textureMode].u); \
	voodoo::texture_mode const texmode1(_TEXMODE1, (TMUS < 2) ? 0 : m_tmu[1].m_reg[textureMode].u); \
	voodoo::fbz_colorpath const fbzcp(_FBZCOLORPATH, m_reg[fbzColorPath].u); \
	voodoo::alpha_mode const alphamode(_ALPHAMODE, m_reg[alphaMode].u); \
	voodoo::fbz_mode const fbzmode(_FBZMODE, m_reg[fbzMode].u); \
	voodoo::fog_mode const fogmode(_FOGMODE, m_reg[fogMode].u); \
	tmu_state::stw_t iterstw0, iterstw1;                                                     \
	tmu_state::stw_t deltastw0, deltastw1;                                                   \
																				\
	/* determine the screen Y */                                                \
	s32 scry = y;                                                                   \
	if (fbzmode.y_origin())                                              \
		scry = (m_fbi.yorigin - y);                                    \
																				\
	/* apply clipping */                                                        \
	s32 startx = extent.startx;                                              \
	s32 stopx = extent.stopx;                                                \
	if (fbzmode.enable_clipping())                                       \
	{                                                                           \
		s32 tempclip;                                                         \
																				\
		/* Y clipping buys us the whole scanline */                             \
		if (scry < ((m_reg[clipLowYHighY].u >> 16) & 0x3ff) ||                 \
			scry >= (m_reg[clipLowYHighY].u & 0x3ff))                          \
		{                                                                       \
			threadstats.pixels_in += stopx - startx;                                 \
			threadstats.clip_fail += stopx - startx;                                 \
			return;                                                             \
		}                                                                       \
																				\
		/* X clipping */                                                        \
		tempclip = m_reg[clipLeftRight].u & 0x3ff;                             \
		/* Check for start outsize of clipping boundary */                        \
		if (startx >= tempclip)                                                  \
		{                                                                       \
			threadstats.pixels_in += stopx - startx;                               \
			threadstats.clip_fail += stopx - startx;                         \
			return;                                               \
		}                                                                       \
		if (stopx > tempclip)                                                  \
		{                                                                       \
			threadstats.pixels_in += stopx - tempclip;                               \
			threadstats.clip_fail += stopx - tempclip;                         \
			stopx = tempclip;                                               \
		}                                                                       \
		tempclip = (m_reg[clipLeftRight].u >> 16) & 0x3ff;                     \
		if (startx < tempclip)                                                  \
		{                                                                       \
			threadstats.pixels_in += tempclip - startx;                              \
			threadstats.clip_fail += tempclip - startx;                        \
			startx = tempclip;                                                  \
		}                                                                       \
	}                                                                           \
																				\
	/* get pointers to the target buffer and depth buffer */                    \
	u16 *dest = extra.destbase + scry * m_fbi.rowpixels;                        \
	u16 *depth = (m_fbi.auxoffs != ~0) ? ((u16 *)(m_fbi.ram + m_fbi.auxoffs) + scry * m_fbi.rowpixels) : nullptr; \
																				\
	/* compute the starting parameters */                                       \
	s32 dx = startx - (extra.ax >> 4);                                             \
	s32 dy = y - (extra.ay >> 4);                                                  \
	s32 iterr = extra.startr + dy * extra.drdy + dx * extra.drdx;                \
	s32 iterg = extra.startg + dy * extra.dgdy + dx * extra.dgdx;                \
	s32 iterb = extra.startb + dy * extra.dbdy + dx * extra.dbdx;                \
	s32 itera = extra.starta + dy * extra.dady + dx * extra.dadx;                \
\
	rgbaint_t iterargb, iterargb_delta;                                           \
	iterargb.set(itera, iterr, iterg, iterb); \
	iterargb_delta.set(extra.dadx, extra.drdx, extra.dgdx, extra.dbdx); \
	s32 iterz = extra.startz + dy * extra.dzdy + dx * extra.dzdx;                \
	s64 iterw = extra.startw + dy * extra.dwdy + dx * extra.dwdx;                \
	if (TMUS >= 1)                                                              \
	{                                                                           \
		iterstw0.set(                                                           \
		extra.starts0 + dy * extra.ds0dy + dx * extra.ds0dx,        \
		extra.startt0 + dy * extra.dt0dy + dx * extra.dt0dx,        \
		extra.startw0 + dy * extra.dw0dy +   dx * extra.dw0dx);     \
		deltastw0.set(extra.ds0dx, extra.dt0dx, extra.dw0dx);       \
	}                                                                           \
	if (TMUS >= 2)                                                              \
	{                                                                           \
		iterstw1.set(                                                           \
		extra.starts1 + dy * extra.ds1dy + dx * extra.ds1dx,        \
		extra.startt1 + dy * extra.dt1dy + dx * extra.dt1dx,        \
		extra.startw1 + dy * extra.dw1dy + dx * extra.dw1dx);       \
		deltastw1.set(extra.ds1dx, extra.dt1dx, extra.dw1dx);       \
	}                                                                           \
	extra.info->hits++;                                                        \
	/* loop in X */                                                             \
	voodoo::dither_helper dither(y, fbzmode, fogmode);           \
	for (s32 x = startx; x < stopx; x++)                                            \
	{                                                                           \
		rgbaint_t texel(0);                                                \
		rgbaint_t color, prefog;                                                \
																				\
		/* pixel pipeline part 1 handles depth setup and stippling */         \
		PIXEL_PIPELINE_BEGIN(threadstats, x, y, fbzcp, fbzmode, iterz, iterw); \
		/* depth testing */         \
		if (fbzmode.enable_depthbuf())                                                  \
			if (!depth_test(threadstats, fbzmode, depth[x], biasdepth)) \
				goto skipdrawdepth; \
																				\
		/* run the texture pipeline on TMU1 to produce a value in texel */      \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 2 && m_tmu[1].lodmin < (8 << 8))                    {       \
			s32 tmp; \
			const rgbaint_t texelZero(0);  \
			texel = m_tmu[1].genTexture(x, dither, texmode1, m_tmu[1].lookup, extra.lodbase1, \
														iterstw1, tmp); \
			texel = m_tmu[1].combineTexture(texmode1, texel, texelZero, tmp); \
		} \
		/* run the texture pipeline on TMU0 to produce a final */               \
		/* result in texel */                                                   \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 1 && m_tmu[0].lodmin < (8 << 8))                           \
		{                                                                   \
			if (!m_send_config)                                                \
			{                                                                   \
				s32 lod0; \
				rgbaint_t texelT0;                                                \
				texelT0 = m_tmu[0].genTexture(x, dither, texmode0, m_tmu[0].lookup, extra.lodbase0, \
																iterstw0, lod0); \
				texel = m_tmu[0].combineTexture(texmode0, texelT0, texel, lod0); \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel.set(m_tmu_config);                                              \
			}                                                                   \
		}                                                                   \
																				\
		/* colorpath pipeline selects source colors and does blending */        \
		color = clamped_argb(iterargb, fbzcp);                                      \
		if (!combine_color(color, threadstats, fbzcp, fbzmode, texel, iterz, iterw)) \
			goto skipdrawdepth;                                                          \
		/* handle alpha test */                                                          \
		if (alphamode.alphatest())                                              \
			if (!alpha_test(threadstats, alphamode, color.get_a()))   \
				goto skipdrawdepth;                                                      \
																						 \
		/* perform fogging */                                                            \
		prefog.set(color);                                                               \
		if (fogmode.enable_fog())                                                                         \
			apply_fogging(color, fbzmode, fogmode, fbzcp, x, dither, wfloat, iterz, iterw, iterargb); \
																												 \
		/* perform alpha blending */                                                \
		if (alphamode.alphablend())                                                            \
			alpha_blend(color, fbzmode, alphamode, x, dither, dest[x], depth, prefog, m_fbi.rgb565); \
																				\
		/* pixel pipeline part 2 handles final output */        \
		PIXEL_PIPELINE_END(threadstats, dither, x, dest, depth, fbzmode);  \
																				\
		/* update the iterated parameters */                                    \
		iterargb += iterargb_delta;                                              \
		iterz += extra.dzdx;                                                   \
		iterw += extra.dwdx;                                                   \
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
inline s32 ATTR_FORCE_INLINE voodoo_device::tmu_state::new_log2(double &value, const int &offset)
{
	static const s32 new_log2_table[16] = {0, 22, 44, 63, 82, 100, 118, 134, 150, 165, 179, 193, 207, 220, 232, 244};
	u64 ival = *((u64 *)&value);
	// Return 0 if negative
	if (ival & ((u64)1 << 63))
		return 0;
	// We zero the result if negative so don't worry about the sign bit
	s32 exp = (ival>>52);
	exp -= 1023+32-offset;
	exp <<= 8;
	u32 addr = (u64)(ival>>48) & 0xf;
	exp += new_log2_table[addr];
	return exp;
}

inline rgbaint_t ATTR_FORCE_INLINE voodoo_device::tmu_state::genTexture(s32 x, voodoo::dither_helper const &dither, voodoo::texture_mode const TEXMODE, rgb_t *LOOKUP, s32 LODBASE, const stw_t &iterstw, s32 &lod)
{
	rgbaint_t result;
	s32 s, t, ilod;

	/* determine the S/T/LOD values for this texture */
	lod = (LODBASE);
	if (TEXMODE.enable_perspective())
	{
		s32 wLog;
		iterstw.calc_stow(s, t, wLog);
		lod += wLog;
	}
	else
	{
#if ((!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64))
		// Extra shift by 8 due to how sse class is stored
		iterstw.get_st_shiftr(s, t, (14 + 10 + 8));
#else
		iterstw.get_st_shiftr(s, t, (14 + 10));
#endif
	}

	/* clamp W */
	if (TEXMODE.clamp_neg_w() && iterstw.is_w_neg())
	{
		s = t = 0;
	}

	/* clamp the LOD */
	lod += lodbias;
	if (TEXMODE.enable_lod_dither())
		lod += dither.raw(x) << 4;
	CLAMP(lod, lodmin, lodmax);

	/* now the LOD is in range; if we don't own this LOD, take the next one */
	ilod = lod >> 8;
	if (!((lodmask >> ilod) & 1))
		ilod++;

	/* fetch the texture base */
	u32 texbase = lodoffset[ilod];

	/* compute the maximum s and t values at this LOD */
	s32 smax = wmask >> ilod;
	s32 tmax = hmask >> ilod;

	/* determine whether we are point-sampled or bilinear */
	if ((lod == lodmin && !TEXMODE.magnification_filter()) ||
		(lod != lodmin && !TEXMODE.minification_filter()))
	{
		/* point sampled */

		u32 texel0;

		/* adjust S/T for the LOD and strip off the fractions */
		s >>= ilod + (18-10);
		t >>= ilod + (18-10);

		/* clamp/wrap S/T if necessary */
		if (TEXMODE.clamp_s())
			CLAMP(s, 0, smax);
		if (TEXMODE.clamp_t())
			CLAMP(t, 0, tmax);
		s &= smax;
		t &= tmax;
		t *= smax + 1;

		/* fetch texel data */
		if (TEXMODE.format() < 8)
		{
			texel0 = *(u8 *)&ram[(texbase + t + s) & mask];
			result.set((LOOKUP)[texel0]);
		}
		else
		{
			texel0 = *(u16 *)&ram[(texbase + 2*(t + s)) & mask];
			if (TEXMODE.format() >= 10 && TEXMODE.format() <= 12)
				result.set((LOOKUP)[texel0]);
			else
				result.set(((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16));
		}
	}
	else
	{
		/* bilinear filtered */

		u32 texel0, texel1, texel2, texel3;
		u32 sfrac, tfrac;
		s32 s1, t1;

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
		if (TEXMODE.clamp_s())
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

		if (TEXMODE.clamp_t())
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
		if (TEXMODE.format() < 8)
		{
			texel0 = *(u8 *)&ram[(texbase + t + s) & mask];
			texel1 = *(u8 *)&ram[(texbase + t + s1) & mask];
			texel2 = *(u8 *)&ram[(texbase + t1 + s) & mask];
			texel3 = *(u8 *)&ram[(texbase + t1 + s1) & mask];
			texel0 = (LOOKUP)[texel0];
			texel1 = (LOOKUP)[texel1];
			texel2 = (LOOKUP)[texel2];
			texel3 = (LOOKUP)[texel3];
		}
		else
		{
			texel0 = *(u16 *)&ram[(texbase + 2*(t + s)) & mask];
			texel1 = *(u16 *)&ram[(texbase + 2*(t + s1)) & mask];
			texel2 = *(u16 *)&ram[(texbase + 2*(t1 + s)) & mask];
			texel3 = *(u16 *)&ram[(texbase + 2*(t1 + s1)) & mask];
			if (TEXMODE.format() >= 10 && TEXMODE.format() <= 12)
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

inline rgbaint_t ATTR_FORCE_INLINE voodoo_device::tmu_state::combineTexture(voodoo::texture_mode const TEXMODE, const rgbaint_t& c_local, const rgbaint_t& c_other, s32 lod)
{
	rgbaint_t blend_color, blend_factor;
	rgbaint_t add_val;

	/* select zero/other for RGB */
	if (TEXMODE.tc_zero_other())
		blend_color.zero();
	else
		blend_color.set(c_other);

	/* select zero/other for alpha */
	if (TEXMODE.tca_zero_other())
		blend_color.zero_alpha();
	else
		blend_color.merge_alpha16(c_other);

	if (TEXMODE.tc_sub_clocal() || TEXMODE.tca_sub_clocal())
	{
		rgbaint_t sub_val;

		/* potentially subtract c_local */
		if (!TEXMODE.tc_sub_clocal())
			sub_val.zero();
		else
			sub_val.set(c_local);

		if (!TEXMODE.tca_sub_clocal())
			sub_val.zero_alpha();
		else
			sub_val.merge_alpha16(c_local);

		blend_color.sub(sub_val);
	}

	/* blend RGB */
	switch (TEXMODE.tc_mselect())
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
				u8 tmp;
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
	switch (TEXMODE.tca_mselect())
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
				u8 tmp;
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
	if (!TEXMODE.tc_reverse_blend())
		blend_factor.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	/* reverse the alpha blend */
	if (!TEXMODE.tca_reverse_blend())
		blend_factor.xor_imm_rgba(0xff, 0, 0, 0);

	/* do the blend */
	//tr = (tr * (blendr + 1)) >> 8;
	//tg = (tg * (blendg + 1)) >> 8;
	//tb = (tb * (blendb + 1)) >> 8;
	//ta = (ta * (blenda + 1)) >> 8;

	/* add clocal or alocal to RGB */
	switch (TEXMODE.tc_add_aclocal())
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
	if (!TEXMODE.tca_add_aclocal())
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
	if (TEXMODE.tc_invert_output())
		blend_color.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	if (TEXMODE.tca_invert_output())
		blend_color.xor_imm_rgba(0xff, 0, 0, 0);
	return blend_color;
}

#endif // MAME_VIDEO_VOODDEFS_IPP
