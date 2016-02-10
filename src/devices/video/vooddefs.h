	// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vooddefs.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/




/*************************************
 *
 *  Core types
 *
 *************************************/

struct voodoo_state;
struct poly_extra_data;





/*************************************
 *
 *  Inline FIFO management
 *
 *************************************/

static inline void fifo_reset(fifo_state *f)
{
	f->in = f->out = 0;
}


static inline void fifo_add(fifo_state *f, UINT32 data)
{
	INT32 next_in;

	/* compute the value of 'in' after we add this item */
	next_in = f->in + 1;
	if (next_in >= f->size)
		next_in = 0;

	/* as long as it's not equal to the output pointer, we can do it */
	if (next_in != f->out)
	{
		f->base[f->in] = data;
		f->in = next_in;
	}
}


static inline UINT32 fifo_remove(fifo_state *f)
{
	UINT32 data = 0xffffffff;

	/* as long as we have data, we can do it */
	if (f->out != f->in)
	{
		INT32 next_out;

		/* fetch the data */
		data = f->base[f->out];

		/* advance the output pointer */
		next_out = f->out + 1;
		if (next_out >= f->size)
			next_out = 0;
		f->out = next_out;
	}
	return data;
}


static inline UINT32 fifo_peek(fifo_state *f)
{
	return f->base[f->out];
}


static inline int fifo_empty(fifo_state *f)
{
	return (f->in == f->out);
}


static inline int fifo_full(fifo_state *f)
{
	return (f->in + 1 == f->out || (f->in == f->size - 1 && f->out == 0));
}


static inline INT32 fifo_items(fifo_state *f)
{
	INT32 items = f->in - f->out;
	if (items < 0)
		items += f->size;
	return items;
}


static inline INT32 fifo_space(fifo_state *f)
{
	INT32 items = f->in - f->out;
	if (items < 0)
		items += f->size;
	return f->size - 1 - items;
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

static inline INT32 fast_reciplog(INT64 value, INT32 *log2)
{
	extern UINT32 voodoo_reciplog[];
	UINT32 temp, recip, rlog;
	UINT32 interp;
	UINT32 *table;
	int neg = FALSE;
	int lz, exp = 0;

	/* always work with unsigned numbers */
	if (value < 0)
	{
		value = -value;
		neg = TRUE;
	}

	/* if we've spilled out of 32 bits, push it down under 32 */
	if (value & U64(0xffff00000000))
	{
		temp = (UINT32)(value >> 16);
		exp -= 16;
	}
	else
		temp = (UINT32)value;

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
	/* to account for the fact that there are two UINT32's per table entry */
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

static inline INT32 float_to_int32(UINT32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	INT32 result = (data & 0x7fffff) | 0x800000;
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


static inline INT64 float_to_int64(UINT32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	INT64 result = (data & 0x7fffff) | 0x800000;
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
			result = U64(0x7fffffffffffffff);
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

static inline UINT32 normalize_color_path(UINT32 eff_color_path)
{
	/* ignore the subpixel adjust and texture enable flags */
	eff_color_path &= ~((1 << 26) | (1 << 27));

	return eff_color_path;
}


static inline UINT32 normalize_alpha_mode(UINT32 eff_alpha_mode)
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


static inline UINT32 normalize_fog_mode(UINT32 eff_fog_mode)
{
	/* if not doing fogging, ignore all the other fog bits */
	if (!FOGMODE_ENABLE_FOG(eff_fog_mode))
		eff_fog_mode = 0;

	return eff_fog_mode;
}


static inline UINT32 normalize_fbz_mode(UINT32 eff_fbz_mode)
{
	/* ignore the draw buffer */
	eff_fbz_mode &= ~(3 << 14);

	return eff_fbz_mode;
}


static inline UINT32 normalize_tex_mode(UINT32 eff_tex_mode)
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


static inline UINT32 compute_raster_hash(const raster_info *info)
{
	UINT32 hash;

	/* make a hash */
	hash = info->eff_color_path;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_fbz_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_alpha_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_fog_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_tex_mode_0;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_tex_mode_1;

	return hash % RASTER_HASH_SIZE;
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
	const UINT8 *dither_lookup = NULL;                                          \
	const UINT8 *dither4 = NULL;                                                \
	const UINT8 *dither = NULL
#define DECLARE_DITHER_POINTERS_NO_DITHER_VAR                                               \
	const UINT8 *dither_lookup = NULL;
#define COMPUTE_DITHER_POINTERS(FBZMODE, YY)                                    \
do                                                                              \
{                                                                               \
	/* compute the dithering pointers */                                        \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		dither4 = &dither_matrix_4x4[((YY) & 3) * 4];                           \
		if (FBZMODE_DITHER_TYPE(FBZMODE) == 0)                                  \
		{                                                                       \
			dither = dither4;                                                   \
			dither_lookup = &dither4_lookup[(YY & 3) << 11];                    \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			dither = &dither_matrix_2x2[((YY) & 3) * 4];                        \
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
		const UINT8 *dith = &DITHER_LOOKUP[((XX) & 3) << 1];                    \
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
	r = (INT32)(ITERR) >> 12;                                             \
	g = (INT32)(ITERG) >> 12;                                             \
	b = (INT32)(ITERB) >> 12;                                             \
	a = (INT32)(ITERA) >> 12;                                             \
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

static inline rgbaint_t ATTR_FORCE_INLINE clampARGB(const rgbaint_t &iterargb, UINT32 FBZCP)
{
	rgbaint_t result(iterargb);
	//rgbaint_t colorint((INT32) (itera>>12), (INT32) (iterr>>12), (INT32) (iterg>>12), (INT32) (iterb>>12));
	result.shr_imm(12);

	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)
	{
		//r &= 0xfff;
		result.and_imm(0xfff);
		//if (r == 0xfff)
		rgbaint_t temp(result);
		temp.cmpeq_imm(0xfff);
		//  result.rgb.r = 0;
		result.andnot_reg(temp);
		//else if (r == 0x100)
		temp.set(result);
		temp.cmpeq_imm(0x100);
		// Shift by 1 so that INT32 result is not negative
		temp.shr_imm(1);
		//  result.rgb.r = 0xff;
		result.or_reg(temp);
	}
	else
	{
		//return colorint.to_rgba_clamp();
	}
	result.clamp_to_uint8();
	return result;
}

#define CLAMPED_Z(ITERZ, FBZCP, RESULT)                                         \
do                                                                              \
{                                                                               \
	(RESULT) = (INT32)(ITERZ) >> 12;                                            \
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
	(RESULT) = (INT16)((ITERW) >> 32);                                          \
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
			INT32 low, high, test;                                              \
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

static inline bool ATTR_FORCE_INLINE chromaKeyTest(voodoo_device *vd, stats_block *stats, UINT32 fbzModeReg, rgbaint_t rgbaIntColor)
{
	if (FBZMODE_ENABLE_CHROMAKEY(fbzModeReg))
	{
		rgb_union color;
		color.u = (rgbaIntColor.get_a()<<24) | (rgbaIntColor.get_r()<<16) | (rgbaIntColor.get_g()<<8) | rgbaIntColor.get_b();
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
			INT32 low, high, test;
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

static inline bool alphaMaskTest(stats_block *stats, UINT32 fbzModeReg, UINT8 alpha)
{
	if (FBZMODE_ENABLE_ALPHA_MASK(fbzModeReg))
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
		UINT8 alpharef = (VV)->reg[alphaMode].rgb.a;                            \
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

static inline bool ATTR_FORCE_INLINE alphaTest(voodoo_device *vd, stats_block *stats, UINT32 alphaModeReg, UINT8 alpha)
{
	if (ALPHAMODE_ALPHATEST(alphaModeReg))
	{
		UINT8 alpharef = vd->reg[alphaMode].rgb.a;
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

static inline void ATTR_FORCE_INLINE alphaBlend(UINT32 FBZMODE, UINT32 ALPHAMODE, INT32 x, const UINT8 *dither, int dpix, UINT16 *depth, rgbaint_t &preFog, rgbaint_t &srcColor)
{
	if (ALPHAMODE_ALPHABLEND(ALPHAMODE))
	{
		//int dpix = dest[XX];
		int dr, dg, db;
		EXTRACT_565_TO_888(dpix, dr, dg, db);
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
			//int dith = DITHER[(XX) & 3];

			/* subtract the dither value */
			dr += (15 - dither[x&3]) >> 1;
			dg += (15 - dither[x&3]) >> 2;
			db += (15 - dither[x&3]) >> 1;
		}

		/* blend the source alpha */
		srcAlphaScale = 0;
		if (ALPHAMODE_SRCALPHABLEND(ALPHAMODE) == 4)
			srcAlphaScale = 256;
			//(AA) = sa;

		/* compute source portion */
		switch (ALPHAMODE_SRCRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				srcScale.set(srcAlphaScale, 0, 0, 0);
				//(RR) = (GG) = (BB) = 0;
				break;

			case 1:     /* ASRC_ALPHA */
				srcScale.set(srcAlphaScale-1, sa, sa, sa);
				srcScale.add_imm(1);
				//(RR) = (sr * (sa + 1)) >> 8;
				//(GG) = (sg * (sa + 1)) >> 8;
				//(BB) = (sb * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				srcScale.set(srcAlphaScale-1, dr, dg, db);
				srcScale.add_imm(1);
				//(RR) = (sr * (dr + 1)) >> 8;
				//(GG) = (sg * (dg + 1)) >> 8;
				//(BB) = (sb * (db + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (da + 1)) >> 8;
				//(GG) = (sg * (da + 1)) >> 8;
				//(BB) = (sb * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				srcScale.set(srcAlphaScale, 256, 256, 256);
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (0x100 - sa)) >> 8;
				//(GG) = (sg * (0x100 - sa)) >> 8;
				//(BB) = (sb * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				srcScale.set(srcAlphaScale, (0x100 - dr), (0x100 - dg), (0x100 - db));
				//(RR) = (sr * (0x100 - dr)) >> 8;
				//(GG) = (sg * (0x100 - dg)) >> 8;
				//(BB) = (sb * (0x100 - db)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (0x100 - da)) >> 8;
				//(GG) = (sg * (0x100 - da)) >> 8;
				//(BB) = (sb * (0x100 - da)) >> 8;
				break;

			case 15:    /* ASATURATE */
				ta = (sa < (0x100 - da)) ? sa : (0x100 - da);
				ta++;
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (ta + 1)) >> 8;
				//(GG) = (sg * (ta + 1)) >> 8;
				//(BB) = (sb * (ta + 1)) >> 8;
				break;
		}

		/* blend the dest alpha */
		destAlphaScale = 0;
		if (ALPHAMODE_DSTALPHABLEND(ALPHAMODE) == 4)
			destAlphaScale = 256;
			//(AA) += da;

		/* add in dest portion */
		switch (ALPHAMODE_DSTRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				destScale.set(destAlphaScale, 0, 0, 0);
				break;

			case 1:     /* ASRC_ALPHA */
				destScale.set(destAlphaScale-1, sa, sa, sa);
				destScale.add_imm(1);
				//(RR) += (dr * (sa + 1)) >> 8;
				//(GG) += (dg * (sa + 1)) >> 8;
				//(BB) += (db * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				destScale.set(srcColor);
				destScale.add_imm(1);
				destScale.set_a(destAlphaScale);
				//(RR) += (dr * (sr + 1)) >> 8;
				//(GG) += (dg * (sg + 1)) >> 8;
				//(BB) += (db * (sb + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (da + 1)) >> 8;
				//(GG) += (dg * (da + 1)) >> 8;
				//(BB) += (db * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				destScale.set(destAlphaScale, 256, 256, 256);
				//(RR) += dr;
				//(GG) += dg;
				//(BB) += db;
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (0x100 - sa)) >> 8;
				//(GG) += (dg * (0x100 - sa)) >> 8;
				//(BB) += (db * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				destScale.set(0x100, 0x100, 0x100, 0x100);
				destScale.sub(srcColor);
				destScale.set_a(destAlphaScale);
				//destScale.set(destAlphaScale, (0x100 - color.rgb.r), (0x100 - color.rgb.g), (0x100 - color.rgb.b));
				//(RR) += (dr * (0x100 - sr)) >> 8;
				//(GG) += (dg * (0x100 - sg)) >> 8;
				//(BB) += (db * (0x100 - sb)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (0x100 - da)) >> 8;
				//(GG) += (dg * (0x100 - da)) >> 8;
				//(BB) += (db * (0x100 - da)) >> 8;
				break;

			case 15:    /* A_COLORBEFOREFOG */
				destScale.set(preFog);
				destScale.add_imm(1);
				destScale.set_a(destAlphaScale);
				//destScale.set((rgb_t) (((destAlphaScale-1)<<24) | (preFog.u & 0x00ffffff)));
				//destScale.add_imm(1);
				//(RR) += (dr * (prefogr + 1)) >> 8;
				//(GG) += (dg * (prefogg + 1)) >> 8;
				//(BB) += (db * (prefogb + 1)) >> 8;
				break;
		}
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
		INT32 fr, fg, fb;                                                       \
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
			INT32 fogblend = 0;                                                 \
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
					INT32 delta = (VV)->fbi.fogdelta[fogdepth >> 10];             \
					INT32 deltaval;                                             \
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

static inline void ATTR_FORCE_INLINE applyFogging(voodoo_device *vd, UINT32 fogModeReg, UINT32 fbzCpReg,  INT32 x, const UINT8 *dither4, INT32 fogDepth,
	rgbaint_t &color, INT32 iterz, INT64 iterw, UINT8 itera)
{
	if (FOGMODE_ENABLE_FOG(fogModeReg))
	{
		UINT32 color_alpha = color.get_a();

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
			INT32 fogblend = 0;

			/* if fog_add is zero, we start with the fog color */
			if (FOGMODE_FOG_ADD(fogModeReg))
				fogColorLocal.set(0, 0, 0, 0);
				//fr = fg = fb = 0;

			/* if fog_mult is zero, we subtract the incoming color */
			if (!FOGMODE_FOG_MULT(fogModeReg))
			{
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
					INT32 delta = vd->fbi.fogdelta[fogDepth >> 10];
					INT32 deltaval;

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
					fogblend = itera;
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
			if (FOGMODE_FOG_MULT(fogModeReg) == 0)
			{
				fogColorLocal.scale_imm_add_and_clamp(fogblend, color);
				//color += fog;
				//(RR) += fr;
				//(GG) += fg;
				//(BB) += fb;
			}

			/* otherwise this just becomes the new color */
			else
			{
				fogColorLocal.scale_imm_and_clamp(fogblend);
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
		fogColorLocal.set_a(color_alpha);
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
	INT32 blendr, blendg, blendb, blenda;                                       \
	INT32 tr, tg, tb, ta;                                                       \
	INT32 s, t, lod, ilod;                                                 \
	INT32 smax, tmax;                                                           \
	UINT32 texbase;                                                             \
	rgb_union c_local;                                                          \
																				\
	/* determine the S/T/LOD values for this texture */                         \
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))                                    \
	{                                                                           \
		if (USE_FAST_RECIP) {                                                     \
			const INT32 oow = fast_reciplog((ITERW), &lod);                         \
			s = ((INT64)oow * (ITERS)) >> 29;                                       \
			t = ((INT64)oow * (ITERT)) >> 29;                                       \
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
		UINT32 texel0;                                                          \
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
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			c_local.u = (LOOKUP)[texel0];                                       \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
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
		UINT32 texel0, texel1, texel2, texel3;                                  \
		UINT32 sfrac, tfrac;                                                    \
		INT32 s1, t1;                                                           \
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
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			texel1 = *(UINT8 *)&(TT)->ram[(texbase + t + s1) & (TT)->mask];     \
			texel2 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s) & (TT)->mask];     \
			texel3 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s1) & (TT)->mask];    \
			texel0 = (LOOKUP)[texel0];                                          \
			texel1 = (LOOKUP)[texel1];                                          \
			texel2 = (LOOKUP)[texel2];                                          \
			texel3 = (LOOKUP)[texel3];                                          \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			texel1 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s1)) & (TT)->mask];\
			texel2 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s)) & (TT)->mask];\
			texel3 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s1)) & (TT)->mask];\
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
	INT32 depthval, wfloat, fogdepth, biasdepth;                                  \
	INT32 r, g, b, a;                                                           \
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
			if ((vd->reg[stipple].u & 0x80000000) == 0)                       \
			{                                                                   \
				vd->stats.total_stippled++;                                   \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* pattern mode */                                                      \
		else                                                                    \
		{                                                                       \
			int stipple_index = (((YY) & 3) << 3) | (~(XX) & 7);                \
			if (((vd->reg[stipple].u >> stipple_index) & 1) == 0)             \
			{                                                                   \
				vd->stats.total_stippled++;                                   \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
	}                                                                           \
																				\
	/* compute "floating point" W value (used for depth and fog) */             \
	if ((ITERW) & U64(0xffff00000000))                                          \
		wfloat = 0x0000;                                                        \
	else                                                                        \
	{                                                                           \
		UINT32 temp = (UINT32)(ITERW);                                \
		if (!(temp & 0xffff0000))                                           \
			wfloat = 0xffff;                                                    \
		else                                                                    \
		{                                                                       \
			int exp = count_leading_zeros(temp);                                \
			wfloat = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1;       \
		}                                                                       \
	}                                                                           \
	fogdepth = wfloat;                                                         \
	/* add the bias for fog selection*/                                         \
	if (FBZMODE_ENABLE_DEPTH_BIAS(FBZMODE))                                     \
	{                                                                           \
		fogdepth += (INT16)vd->reg[zaColor].u;                                \
		CLAMP(fogdepth, 0, 0xffff);                                             \
	}                                                                           \
																				\
	/* compute depth value (W or Z) for this pixel */                           \
	if (FBZMODE_WBUFFER_SELECT(FBZMODE) == 0)                                   \
	{                                                                           \
		CLAMPED_Z(ITERZ, FBZCOLORPATH, depthval);                               \
	}                                                                           \
	else if (FBZMODE_DEPTH_FLOAT_SELECT(FBZMODE) == 0)                          \
		depthval = wfloat;                                                      \
	else                                                                        \
	{                                                                           \
		if ((ITERZ) & 0xf0000000)                                               \
			depthval = 0x0000;                                                  \
		else                                                                    \
		{                                                                       \
			UINT32 temp = (ITERZ << 4);                             \
			if (!(temp & 0xffff0000))                                                           \
				depthval = 0xffff;                                              \
			else                                                                \
			{                                                                   \
				int exp = count_leading_zeros(temp);                            \
				depthval = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1; \
			}                                                                   \
		}                                                                       \
	}                                                                            \
	/* add the bias */                                                          \
	biasdepth = depthval;                                                     \
	if (FBZMODE_ENABLE_DEPTH_BIAS(FBZMODE))                                     \
	{                                                                           \
		biasdepth += (INT16)vd->reg[zaColor].u;                                \
		CLAMP(biasdepth, 0, 0xffff);                                             \
	}


#define DEPTH_TEST(vd, STATS, XX, FBZMODE)    \
do                                                                              \
{                                                                               \
	/* handle depth buffer testing */                                           \
	if (FBZMODE_ENABLE_DEPTHBUF(FBZMODE))                                       \
	{                                                                           \
		INT32 depthsource;                                                      \
																				\
		/* the source depth is either the iterated W/Z+bias or a */             \
		/* constant value */                                                    \
		if (FBZMODE_DEPTH_SOURCE_COMPARE(FBZMODE) == 0)                         \
			depthsource = biasdepth;                                             \
		else                                                                    \
			depthsource = (UINT16)vd->reg[zaColor].u;                         \
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

static inline bool ATTR_FORCE_INLINE depthTest(UINT16 zaColorReg, stats_block *stats, INT32 destDepth, UINT32 fbzModeReg, INT32 biasdepth)
{
	/* handle depth buffer testing */
	if (FBZMODE_ENABLE_DEPTHBUF(fbzModeReg))
	{
		INT32 depthsource;

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

#define PIXEL_PIPELINE_END(vd, STATS, DITHER, DITHER4, DITHER_LOOKUP, XX, dest, depth, FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE, ITERZ, ITERW, ITERAXXX) \
																				\
	/* perform fogging */                                                       \
	preFog.set(color); \
	applyFogging(vd, FOGMODE, FBZCOLORPATH, XX, DITHER4, fogdepth, color, ITERZ, ITERW, ITERAXXX.get_a()); \
	/* perform alpha blending */                                                \
	alphaBlend(FBZMODE, ALPHAMODE, XX, DITHER, dest[XX], depth, preFog, color); \
	a = color.get_a(); r = color.get_r(); g = color.get_g(); b = color.get_b();                     \
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
	if (depth && FBZMODE_AUX_BUFFER_MASK(FBZMODE))                              \
	{                                                                           \
		if (FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) == 0)                          \
			depth[XX] = biasdepth;                                               \
		else                                                                    \
			depth[XX] = a;                                                      \
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

    INT32 r, g, b, a;
*/
#define COLORPATH_PIPELINE(VV, STATS, FBZCOLORPATH, FBZMODE, ALPHAMODE, TEXELARGB, ITERZ, ITERW, ITERARGB) \
do                                                                              \
{                                                                               \
	INT32 blendr, blendg, blendb, blenda;                                       \
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
			c_local.rgb.a = (UINT8)temp;                                        \
			break;                                                              \
		}                                                                       \
																				\
		case 3:     /* clamped iterated W[39:32] */                             \
		{                                                                       \
			int temp;                                                           \
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */ \
			c_local.rgb.a = (UINT8)temp;                                        \
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

static inline bool ATTR_FORCE_INLINE combineColor(voodoo_device *vd, stats_block *STATS, UINT32 FBZCOLORPATH, UINT32 FBZMODE, UINT32 ALPHAMODE,
													rgbaint_t TEXELARGB, INT32 ITERZ, INT64 ITERW, rgbaint_t &srcColor)
{
	rgbaint_t c_other;
	rgbaint_t c_local;

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

		default:    /* reserved - voodoo3 framebufferRGB */
			c_other.set(0);
			break;
	}

	/* handle chroma key */
	if (!chromaKeyTest(vd, STATS, FBZMODE, c_other))
		return false;
	//APPLY_CHROMAKEY(vd->m_vds, STATS, FBZMODE, c_other);

	/* compute a_other */
	switch (FBZCP_CC_ASELECT(FBZCOLORPATH))
	{
		case 0:     /* iterated alpha */
			c_other.merge_alpha(srcColor);
			break;

		case 1:     /* texture alpha */
			c_other.merge_alpha(TEXELARGB);
			break;

		case 2:     /* color1 alpha */
			c_other.set_a(vd->reg[color1].rgb.a);
			break;

		default:    /* reserved */
			c_other.set_a(0);
			break;
	}

	/* handle alpha mask */
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
			c_local.merge_alpha(srcColor);
			break;

		case 1:     /* color0 alpha */
			c_local.set_a(vd->reg[color0].rgb.a);
			break;

		case 2:     /* clamped iterated Z[27:20] */
		{
			int temp;
			CLAMPED_Z(ITERZ, FBZCOLORPATH, temp);
			c_local.set_a((UINT8) temp);
			break;
		}

		case 3:     /* clamped iterated W[39:32] */
		{
			int temp;
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */
			c_local.set_a((UINT8) temp);
			break;
		}
	}

	UINT8 a_other = c_other.get_a();
	UINT8 a_local = c_local.get_a();
	UINT8 tmp;
	rgbaint_t add_val(c_local);

	/* select zero or c_other */
	if (FBZCP_CC_ZERO_OTHER(FBZCOLORPATH))
		c_other.and_imm_rgba(-1, 0, 0, 0);
		//r = g = b = 0;

	/* select zero or a_other */
	if (FBZCP_CCA_ZERO_OTHER(FBZCOLORPATH))
		c_other.set_a(0);

	/* subtract a/c_local */
	if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH) || (FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH)))
	{
		rgbaint_t sub_val = c_local;

		if (!FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.set(a_local, 0, 0, 0);

		if (!FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.set_a(0);

		c_other.sub(sub_val);
	}

	/* blend RGB */
	switch (FBZCP_CC_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			c_local.and_imm_rgba(-1, 0, 0, 0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set(a_local, a_other, a_other, a_other);
			break;

		case 3:     /* a_local */
			c_local.set(a_local, a_local, a_local, a_local);
			break;

		case 4:     /* texture alpha */
			tmp = TEXELARGB.get_a();
			c_local.set(a_local, tmp, tmp, tmp);
			break;

		case 5:     /* texture RGB (Voodoo 2 only) */
			c_local.set(TEXELARGB);
			break;
	}

	/* blend alpha */
	switch (FBZCP_CCA_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			c_local.set_a(0);
			break;

		case 1:     /* a_local */
		case 3:     /* a_local */
			c_local.set_a(a_local);
			break;

		case 2:     /* a_other */
			c_local.set_a(a_other);
			break;

		case 4:     /* texture alpha */
				c_local.merge_alpha(TEXELARGB);
			break;
	}

	/* reverse the RGB blend */
	if (!FBZCP_CC_REVERSE_BLEND(FBZCOLORPATH))
		c_local.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	/* reverse the alpha blend */
	if (!FBZCP_CCA_REVERSE_BLEND(FBZCOLORPATH))
		c_local.xor_imm_rgba(0xff, 0, 0, 0);

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
			add_val.set(a_local, 0, 0, 0);
			break;

		case 1:     /* add c_local */
			break;

		case 2:     /* add_alocal */
			add_val.set(a_local, a_local, a_local, a_local);
			break;
	}

	/* add clocal or alocal to alpha */
	if (!FBZCP_CCA_ADD_ACLOCAL(FBZCOLORPATH))
		add_val.set_a(0);
		//color.rgb.a += c_local.rgb.a;

	/* clamp */
	//CLAMP(color.rgb.a, 0x00, 0xff);
	//CLAMP(color.rgb.r, 0x00, 0xff);
	//CLAMP(color.rgb.g, 0x00, 0xff);
	//CLAMP(color.rgb.b, 0x00, 0xff);
	c_local.add_imm(1);
	c_other.scale_add_and_clamp(c_local, add_val);
	srcColor.set(c_other);

	/* invert */
	if (FBZCP_CCA_INVERT_OUTPUT(FBZCOLORPATH))
		srcColor.xor_imm_rgba(0xff, 0, 0, 0);
	/* invert */
	if (FBZCP_CC_INVERT_OUTPUT(FBZCOLORPATH))
		srcColor.xor_imm_rgba(0, 0xff, 0xff, 0xff);


	/* handle alpha test */
	if (!alphaTest(vd, STATS, ALPHAMODE, srcColor.get_a()))
		return false;
	//APPLY_ALPHATEST(vd->m_vds, STATS, ALPHAMODE, color.rgb.a);

	return true;
}



/*************************************
 *
 *  Rasterizer generator macro
 *
 *************************************/

#define RASTERIZER(name, TMUS, FBZCOLORPATH, FBZMODE, ALPHAMODE, FOGMODE, TEXMODE0, TEXMODE1) \
																				\
void voodoo_device::raster_##name(void *destbase, INT32 y, const poly_extent *extent, const void *extradata, int threadid) \
{                                                                               \
	const poly_extra_data *extra = (const poly_extra_data *)extradata;          \
	voodoo_device *vd = extra->device; \
	stats_block *stats = &vd->thread_stats[threadid];                            \
	DECLARE_DITHER_POINTERS;                                                    \
	INT32 startx = extent->startx;                                              \
	INT32 stopx = extent->stopx;                                                \
	rgbaint_t iterargb, iterargbDelta;                                           \
	INT32 iterz;                                                                \
	INT64 iterw, iterw0 = 0, iterw1 = 0;                                        \
	INT64 iters0 = 0, iters1 = 0;                                               \
	INT64 itert0 = 0, itert1 = 0;                                               \
	UINT16 *depth;                                                              \
	UINT16 *dest;                                                               \
	INT32 dx, dy;                                                               \
	INT32 scry;                                                                 \
	INT32 x;                                                                    \
																				\
	/* determine the screen Y */                                                \
	scry = y;                                                                   \
	if (FBZMODE_Y_ORIGIN(FBZMODE))                                              \
		scry = (vd->fbi.yorigin - y) & 0x3ff;                                    \
																				\
	/* compute dithering */                                                     \
	COMPUTE_DITHER_POINTERS(FBZMODE, y);                                        \
																				\
	/* apply clipping */                                                        \
	if (FBZMODE_ENABLE_CLIPPING(FBZMODE))                                       \
	{                                                                           \
		INT32 tempclip;                                                         \
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
		tempclip = (vd->reg[clipLeftRight].u >> 16) & 0x3ff;                     \
		if (startx < tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += tempclip - startx;                              \
			vd->stats.total_clipped += tempclip - startx;                        \
			startx = tempclip;                                                  \
		}                                                                       \
		tempclip = vd->reg[clipLeftRight].u & 0x3ff;                             \
		if (stopx >= tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += stopx - tempclip;                               \
			vd->stats.total_clipped += stopx - tempclip;                         \
			stopx = tempclip - 1;                                               \
		}                                                                       \
	}                                                                           \
																				\
	/* get pointers to the target buffer and depth buffer */                    \
	dest = (UINT16 *)destbase + scry * vd->fbi.rowpixels;                        \
	depth = (vd->fbi.auxoffs != ~0) ? ((UINT16 *)(vd->fbi.ram + vd->fbi.auxoffs) + scry * vd->fbi.rowpixels) : NULL; \
																				\
	/* compute the starting parameters */                                       \
	dx = startx - (extra->ax >> 4);                                             \
	dy = y - (extra->ay >> 4);                                                  \
	INT32 iterr = extra->startr + dy * extra->drdy + dx * extra->drdx;                \
	INT32 iterg = extra->startg + dy * extra->dgdy + dx * extra->dgdx;                \
	INT32 iterb = extra->startb + dy * extra->dbdy + dx * extra->dbdx;                \
	INT32 itera = extra->starta + dy * extra->dady + dx * extra->dadx;                \
	iterargb.set(itera, iterr, iterg, iterb); \
	iterargbDelta.set(extra->dadx, extra->drdx, extra->dgdx, extra->dbdx); \
	iterz = extra->startz + dy * extra->dzdy + dx * extra->dzdx;                \
	iterw = extra->startw + dy * extra->dwdy + dx * extra->dwdx;                \
	if (TMUS >= 1)                                                              \
	{                                                                           \
		iterw0 = extra->startw0 + dy * extra->dw0dy +   dx * extra->dw0dx;      \
		iters0 = extra->starts0 + dy * extra->ds0dy + dx * extra->ds0dx;        \
		itert0 = extra->startt0 + dy * extra->dt0dy + dx * extra->dt0dx;        \
	}                                                                           \
	if (TMUS >= 2)                                                              \
	{                                                                           \
		iterw1 = extra->startw1 + dy * extra->dw1dy +   dx * extra->dw1dx;      \
		iters1 = extra->starts1 + dy * extra->ds1dy + dx * extra->ds1dx;        \
		itert1 = extra->startt1 + dy * extra->dt1dy + dx * extra->dt1dx;        \
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
		if (!depthTest((UINT16) vd->reg[zaColor].u, stats, depth[x], FBZMODE, biasdepth)) \
			goto skipdrawdepth; \
																				\
		/* run the texture pipeline on TMU1 to produce a value in texel */      \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 2 && vd->tmu[1].lodmin < (8 << 8))                    {       \
			INT32 tmp; \
			const rgbaint_t texelZero(0);  \
			texel = genTexture(&vd->tmu[1], x, dither4, TEXMODE1, vd->tmu[1].lookup, extra->lodbase1, \
														iters1, itert1, iterw1, tmp); \
			texel = combineTexture(&vd->tmu[1], TEXMODE1, texel, texelZero, tmp); \
		} \
		/* run the texture pipeline on TMU0 to produce a final */               \
		/* result in texel */                                                   \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 1 && vd->tmu[0].lodmin < (8 << 8))                           \
		{                                                                   \
			if (!vd->send_config)                                                \
			{                                                                   \
				INT32 lod0; \
				rgbaint_t texelT0;                                                \
				texelT0 = genTexture(&vd->tmu[0], x, dither4, TEXMODE0, vd->tmu[0].lookup, extra->lodbase0, \
																iters0, itert0, iterw0, lod0); \
				texel = combineTexture(&vd->tmu[0], TEXMODE0, texelT0, texel, lod0); \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel.set(vd->tmu_config);                                              \
			}                                                                   \
		}                                                                   \
																				\
		/* colorpath pipeline selects source colors and does blending */        \
		color = clampARGB(iterargb, FBZCOLORPATH);           \
		if (!combineColor(vd, stats, FBZCOLORPATH, FBZMODE, ALPHAMODE, texel, iterz, iterw, color)) \
			goto skipdrawdepth; \
																				\
		/* pixel pipeline part 2 handles fog, alpha, and final output */        \
		PIXEL_PIPELINE_END(vd, stats, dither, dither4, dither_lookup, x, dest, depth, \
							FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE,          \
							iterz, iterw, iterargb);                            \
																				\
		/* update the iterated parameters */                                    \
		iterargb += iterargbDelta;                                              \
		iterz += extra->dzdx;                                                   \
		iterw += extra->dwdx;                                                   \
		if (TMUS >= 1)                                                          \
		{                                                                       \
			iterw0 += extra->dw0dx;                                             \
			iters0 += extra->ds0dx;                                             \
			itert0 += extra->dt0dx;                                             \
		}                                                                       \
		if (TMUS >= 2)                                                          \
		{                                                                       \
			iterw1 += extra->dw1dx;                                             \
			iters1 += extra->ds1dx;                                             \
			itert1 += extra->dt1dx;                                             \
		}                                                                       \
	}                                                                           \
}


// ******************************************************************************************************************************
// Computes a log2 of a 16.32 value to 2 fractional bits of precision.
// The return value is coded as a 24.8 value.
// The maximum error using a 4 bit lookup from the mantissa is 0.0875, which is less than 1/2 lsb (0.125) for 2 bits of fraction.
// ******************************************************************************************************************************
static inline INT32 ATTR_FORCE_INLINE new_log2(double &value)
{
	static const INT32 new_log2_table[16] = {0, 22, 44, 63, 82, 100, 118, 134, 150, 165, 179, 193, 207, 220, 232, 244};
	UINT64 ival = *((UINT64 *)&value);
	// We zero the result if negative so don't worry about the sign bit
	INT32 exp = (ival>>52);
	exp -= 1023+32;
	exp <<= 8;
	UINT32 addr = (UINT64)(ival>>48) & 0xf;
	exp += new_log2_table[addr];
	// Return 0 if negative
	return (ival & ((UINT64)1<<63)) ? 0 : exp;
}

// Computes A/C and B/C and returns log2 of 1/C
// A, B and C are 16.32 values.  The results are 24.8.
static inline void ATTR_FORCE_INLINE multi_reciplog(INT64 valueA, INT64 valueB, INT64 valueC, INT32 &log, INT32 &resA, INT32 &resB)
{
	double recip = double(1ULL<<(47-39))/valueC;
	double resAD = valueA * recip;
	double resBD = valueB * recip;
	log = new_log2(recip);
	log += 56<<8;
	resA = resAD;
	resB = resBD;
}


static inline rgbaint_t ATTR_FORCE_INLINE genTexture(tmu_state *TT, INT32 x, const UINT8 *dither4, const UINT32 TEXMODE, rgb_t *LOOKUP, INT32 LODBASE, INT64 ITERS, INT64 ITERT, INT64 ITERW, INT32 &lod)
{
	rgbaint_t result;
	INT32 s, t, ilod;

	/* determine the S/T/LOD values for this texture */
	lod = (LODBASE);
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))
	{
		INT32 wLog;
		if (USE_FAST_RECIP) {
			const INT32 oow = fast_reciplog((ITERW), &wLog);
			s = ((INT64)oow * (ITERS)) >> (29+10);
			t = ((INT64)oow * (ITERT)) >> (29+10);
		} else {
			multi_reciplog(ITERS, ITERT, ITERW, wLog, s, t);
		}
		lod += wLog;
	}
	else
	{
		s = (ITERS) >> (14+10);
		t = (ITERT) >> (14+10);
	}

	/* clamp W */
	if (TEXMODE_CLAMP_NEG_W(TEXMODE) && (ITERW) < 0)
	{
		s = t = 0;
	}

	/* clamp the LOD */
	lod += (TT)->lodbias;
	if (TEXMODE_ENABLE_LOD_DITHER(TEXMODE))
		lod += dither4[x&3] << 4;
	if (lod < (TT)->lodmin)
		lod = (TT)->lodmin;
	else if (lod > (TT)->lodmax)
		lod = (TT)->lodmax;

	/* now the LOD is in range; if we don't own this LOD, take the next one */
	ilod = lod >> 8;
	if (!(((TT)->lodmask >> ilod) & 1))
		ilod++;

	/* fetch the texture base */
	UINT32 texbase = (TT)->lodoffset[ilod];

	/* compute the maximum s and t values at this LOD */
	INT32 smax = (TT)->wmask >> ilod;
	INT32 tmax = (TT)->hmask >> ilod;

	/* determine whether we are point-sampled or bilinear */
	if ((lod == (TT)->lodmin && !TEXMODE_MAGNIFICATION_FILTER(TEXMODE)) ||
		(lod != (TT)->lodmin && !TEXMODE_MINIFICATION_FILTER(TEXMODE)))
	{
		/* point sampled */

		UINT32 texel0;

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
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];
			result.set((LOOKUP)[texel0]);
		}
		else
		{
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask];
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12)
				result.set((LOOKUP)[texel0]);
			else
				result.set(((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16));
		}
	}
	else
	{
		/* bilinear filtered */

		UINT32 texel0, texel1, texel2, texel3;
		UINT32 sfrac, tfrac;
		INT32 s1, t1;

		/* adjust S/T for the LOD and strip off all but the low 8 bits of */
		/* the fraction */
		s >>= ilod; // + (10-10);
		t >>= ilod; // + (10-10);

		/* also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel */
		s -= 0x80;
		t -= 0x80;

		/* extract the fractions */
		sfrac = s & (TT)->bilinear_mask;
		tfrac = t & (TT)->bilinear_mask;

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
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s)];
			texel1 = *(UINT8 *)&(TT)->ram[(texbase + t + s1)];
			texel2 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s)];
			texel3 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s1)];
			texel0 = (LOOKUP)[texel0];
			texel1 = (LOOKUP)[texel1];
			texel2 = (LOOKUP)[texel2];
			texel3 = (LOOKUP)[texel3];
		}
		else
		{
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s))];
			texel1 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s1))];
			texel2 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s))];
			texel3 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s1))];
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

static inline rgbaint_t ATTR_FORCE_INLINE combineTexture(tmu_state *TT, const UINT32 TEXMODE, rgbaint_t c_local, rgbaint_t c_other, INT32 lod)
{
	INT32 a_other = c_other.get_a();
	INT32 a_local = c_local.get_a();
	rgbaint_t add_val = c_local;
	UINT8 tmp;

	/* select zero/other for RGB */
	if (TEXMODE_TC_ZERO_OTHER(TEXMODE))
		c_other.and_imm_rgba(-1, 0, 0, 0);

	/* select zero/other for alpha */
	if (TEXMODE_TCA_ZERO_OTHER(TEXMODE))
		c_other.set_a(0);

	if (TEXMODE_TC_SUB_CLOCAL(TEXMODE) || TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
	{
		rgbaint_t sub_val = c_local;

		/* potentially subtract c_local */
		if (!TEXMODE_TC_SUB_CLOCAL(TEXMODE))
			sub_val.and_imm_rgba(-1, 0, 0, 0);

		if (!TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
			sub_val.set_a(0);

		c_other.sub(sub_val);
	}

	/* blend RGB */
	switch (TEXMODE_TC_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			c_local.and_imm_rgba(-1, 0, 0, 0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set(a_local, a_other, a_other, a_other);
			break;

		case 3:     /* a_local */
			c_local.set(a_local, a_local, a_local, a_local);
			break;

		case 4:     /* LOD (detail factor) */
			if ((TT)->detailbias <= lod)
				c_local.and_imm_rgba(-1, 0, 0, 0);
			else
			{
				tmp = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);
				if (tmp > (TT)->detailmax)
					tmp = (TT)->detailmax;
				c_local.set(a_local, tmp, tmp, tmp);
			}
			break;

		case 5:     /* LOD fraction */
			tmp = lod & 0xff;
			c_local.set(a_local, tmp, tmp, tmp);
			break;
	}

	/* blend alpha */
	switch (TEXMODE_TCA_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			c_local.set_a(0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set_a(a_other);
			break;

		case 3:     /* a_local */
			break;

		case 4:     /* LOD (detail factor) */
			if ((TT)->detailbias <= lod)
				c_local.set_a(0);
			else
			{
				tmp = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);
				if (tmp > (TT)->detailmax)
					tmp = (TT)->detailmax;
				c_local.set_a(tmp);
			}
			break;

		case 5:     /* LOD fraction */
			c_local.set_a(lod & 0xff);
			break;
	}

	/* reverse the RGB blend */
	if (!TEXMODE_TC_REVERSE_BLEND(TEXMODE))
	{
		c_local.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	}

	/* reverse the alpha blend */
	if (!TEXMODE_TCA_REVERSE_BLEND(TEXMODE))
		c_local.xor_imm_rgba(0xff, 0, 0, 0);

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
			add_val.set(a_local, 0, 0, 0);
			break;

		case 1:     /* add c_local */
			break;

		case 2:     /* add_alocal */
			add_val.set(a_local, a_local , a_local , a_local);
			//tr += c_local.rgb.a;
			//tg += c_local.rgb.a;
			//tb += c_local.rgb.a;
			break;
	}

	/* add clocal or alocal to alpha */
	if (!TEXMODE_TCA_ADD_ACLOCAL(TEXMODE))
		add_val.set_a(0);
		//ta += c_local.rgb.a;

	/* clamp */
	//result.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;
	//result.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;
	//result.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;
	//result.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;
	c_local.add_imm(1);
	c_other.scale_add_and_clamp(c_local, add_val);
	rgbaint_t result(c_other);
	/* invert */
	if (TEXMODE_TC_INVERT_OUTPUT(TEXMODE))
		result.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	if (TEXMODE_TCA_INVERT_OUTPUT(TEXMODE))
		result.xor_imm_rgba(0xff, 0, 0, 0);
	return result;
}
