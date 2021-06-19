// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_render.cpp

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#include "emu.h"
#include "voodoo.h"

namespace voodoo
{

static constexpr bool LOG_RASTERIZERS = false;


struct static_rasterizer_info
{
	voodoo_renderer::rasterizer_mfp mfp;
	rasterizer_params params;
};

extern static_rasterizer_info s_predef_raster_table[];


// fast dither lookup
std::unique_ptr<u8[]> dither_helper::s_dither4_lookup;
std::unique_ptr<u8[]> dither_helper::s_dither2_lookup;
std::unique_ptr<u8[]> dither_helper::s_nodither_lookup;

u8 const dither_helper::s_dither_matrix_4x4[16] =
{
	 0,  8,  2, 10,
	12,  4, 14,  6,
	 3, 11,  1,  9,
	15,  7, 13,  5
};
// Using this matrix allows iteagle video memory tests to pass
u8 const dither_helper::s_dither_matrix_2x2[16] =
{
	 8, 10,  8, 10,
	11,  9, 11,  9,
	 8, 10,  8, 10,
	11,  9, 11,  9
};

static const rectangle global_cliprect(-4096, 4095, -4096, 4095);



//**************************************************************************
//  STATIC HELPERS
//**************************************************************************

//-------------------------------------------------
//  compute_wfloat - compute the pseudo-floating
//  point version of the iterated W value
//-------------------------------------------------

static inline s32 ATTR_FORCE_INLINE compute_wfloat(s64 iterw)
{
	int exp = count_leading_zeros_64(iterw) - 16;
	if (exp < 0)
		return 0x0000;
	if (exp >= 16)
		return 0xffff;
	return ((exp << 12) | ((iterw >> (35 - exp)) ^ 0x1fff)) + 1;
}


//-------------------------------------------------
//  clamped_argb - clamp the incoming ARGB color
//  according to the clamping settings, or do the
//  fake pseudo-clamp described in the Voodoo
//  manual
//-------------------------------------------------

static inline rgbaint_t ATTR_FORCE_INLINE clamped_argb(const rgbaint_t &iterargb, reg_fbz_colorpath const fbzcp)
{
	rgbaint_t result(iterargb);
	result.shr_imm(20);

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

	// check against 0xfff and force to 0
	rgbaint_t temp1(result);
	rgbaint_t temp2(result);
	temp1.cmpeq_imm(0xfff);
	temp2.cmpeq_imm(0x100);
	result.andnot_reg(temp1);
	result.or_reg(temp2);
	result.and_imm(0xff);
	return result;
}


//-------------------------------------------------
//  clamped_z - clamp the Z value according to the
//  clamping settings, or do the fake pseudo-clamp
//  described in the Voodoo manual
//-------------------------------------------------

static inline s32 ATTR_FORCE_INLINE clamped_z(s32 iterz, reg_fbz_colorpath const fbzcp)
{
	// clamped case is easy
	if (fbzcp.rgbzw_clamp() != 0)
		return std::clamp(iterz >> 12, 0, 0xffff);

	// non-clamped case has specific behaviors
	u32 result = u32(iterz) >> 12;
	if (result == 0xfffff)
		return 0;
	if (result == 0x10000)
		return 0xffff;
	return result & 0xffff;
}


//-------------------------------------------------
//  clamped_w - clamp the W value according to the
//  clamping settings, or do the fake pseudo-clamp
//  described in the Voodoo manual
//-------------------------------------------------

static inline s32 ATTR_FORCE_INLINE clamped_w(s64 iterw, reg_fbz_colorpath const fbzcp)
{
	// clamped case is easy
	if (fbzcp.rgbzw_clamp() != 0)
		return std::clamp(s32(iterw >> 48), 0, 0xff);

	// non-clamped case has specific behaviors
	u32 result = u64(iterw) >> 48;
	if (result == 0xffff)
		return 0;
	if (result == 0x100)
		return 0xff;
	return result & 0xff;
}



//**************************************************************************
//  RASTERIZER PARAMS
//**************************************************************************

//-------------------------------------------------
//  operator== - compare all values
//-------------------------------------------------

bool rasterizer_params::operator==(rasterizer_params const &rhs) const
{
	return (m_fbzcp == rhs.m_fbzcp &&
			m_fbzmode == rhs.m_fbzmode &&
			m_alphamode == rhs.m_alphamode &&
			m_fogmode == rhs.m_fogmode &&
			m_texmode0 == rhs.m_texmode0 &&
			m_texmode1 == rhs.m_texmode1);
}


//-------------------------------------------------
//  compute - compute the normalized rasterizer
//  parameters based on the current register state
//-------------------------------------------------

void rasterizer_params::compute(voodoo_regs &regs, voodoo_regs *tmu0regs, voodoo_regs *tmu1regs)
{
	// these values are normalized to ignore irrelevant bits
	m_fbzcp = regs.fbz_colorpath().normalize();
	m_fbzmode = regs.fbz_mode().normalize();
	m_alphamode = regs.alpha_mode().normalize();
	m_fogmode = regs.fog_mode().normalize();

	// if the TMUs are disabled or no textures are enabled, the texture modes are invalid
	if (regs.fbi_init3().disable_tmus() || !regs.fbz_colorpath().texture_enable())
		m_texmode0 = m_texmode1 = 0xffffffff;
	else
	{
		m_texmode0 = (tmu0regs != nullptr && tmu0regs->texture_lod().lod_min() < 32) ? tmu0regs->texture_mode().normalize() : 0xffffffff;
		m_texmode1 = (tmu1regs != nullptr && tmu1regs->texture_lod().lod_min() < 32) ? tmu1regs->texture_mode().normalize() : 0xffffffff;

		// repurpose the top bit of texmode for the send TMU config
		if (m_texmode0 != 0xffffffff && tmu0regs->trexinit_send_tmu_config())
			m_texmode0 |= 1 << 31;
	}
}


//-------------------------------------------------
//  hash - return a hash of the current values
//-------------------------------------------------

u32 rasterizer_params::hash() const
{
	return rotate(m_alphamode, 0) ^
			rotate(m_fbzmode, 6) ^
			rotate(m_fbzcp, 12) ^
			rotate(m_fogmode, 18) ^
			rotate(m_texmode0, 24) ^
			rotate(m_texmode1, 30);
}



//**************************************************************************
//  DITHER HELPER
//**************************************************************************

//-------------------------------------------------
//  init_static - initialize static tables
//-------------------------------------------------

void dither_helper::init_static()
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



//**************************************************************************
//  STW HELPER
//**************************************************************************

// use SSE on 64-bit implementations, where it can be assumed
#if 1 && ((!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64))

#include <emmintrin.h>
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

class stw_helper
{
public:
	stw_helper() { }
	stw_helper(stw_helper const &other) = default;
	stw_helper &operator=(stw_helper const &other) = default;

	void set(s64 s, s64 t, s64 w)
	{
		m_st = _mm_set_pd(s << 8, t << 8);
		m_w = _mm_set1_pd(w);
	}

	bool is_w_neg() const
	{
		return _mm_comilt_sd(m_w, _mm_set1_pd(0.0));
	}

	void get_st_shiftr(s32 &s, s32 &t, s32 shift) const
	{
		shift += 8;
		s = _mm_cvtsd_si64(_mm_shuffle_pd(m_st, _mm_setzero_pd(), 1)) >> shift;
		t = _mm_cvtsd_si64(m_st) >> shift;
	}

	void add(stw_helper const &delta)
	{
		m_st = _mm_add_pd(m_st, delta.m_st);
		m_w = _mm_add_pd(m_w, delta.m_w);
	}

	void calc_stow(s32 &sow, s32 &tow, s32 &oowlog) const
	{
		__m128d tmp = _mm_div_pd(m_st, m_w);
		__m128i tmp2 = _mm_cvttpd_epi32(tmp);
#ifdef __SSE4_1__
		sow = _mm_extract_epi32(tmp2, 1);
		tow = _mm_extract_epi32(tmp2, 0);
#else
		sow = _mm_cvtsi128_si32(_mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 0, 1)));
		tow = _mm_cvtsi128_si32(tmp2);
#endif
		oowlog = -fast_log2(_mm_cvtsd_f64(m_w), 0);
	}

private:
	__m128d m_st;
	__m128d m_w;
};

#else

class stw_helper
{
public:
	stw_helper() {}
	stw_helper(stw_helper const &other) = default;
	stw_helper &operator=(stw_helper const &other) = default;

	void set(s64 s, s64 t, s64 w)
	{
		m_s = s;
		m_t = t;
		m_w = w;
	}

	bool is_w_neg() const
	{
		return (m_w < 0) ? true : false;
	}

	void get_st_shiftr(s32 &s, s32 &t, s32 shift) const
	{
		s = m_s >> shift;
		t = m_t >> shift;
	}

	inline void add(stw_helper const &other)
	{
		m_s += other.m_s;
		m_t += other.m_t;
		m_w += other.m_w;
	}

	// Computes s/w and t/w and returns log2 of 1/w
	// s, t and c are 16.32 values.  The results are 24.8.
	inline void calc_stow(s32 &sow, s32 &tow, s32 &oowlog) const
	{
		double recip = double(1ULL << (47 - 39)) / m_w;
		sow = s32(m_s * recip);
		tow = tow(m_t * recip);
		oowlog = fast_log2(recip, 56);
	}

private:
	s64 m_s, m_t, m_w;
};

#endif


//**************************************************************************
//  RASTER TEXTURE
//**************************************************************************

//-------------------------------------------------
//  recompute - recompute state based on parameters
//-------------------------------------------------

void rasterizer_texture::recompute(voodoo_regs const &regs, u8 *ram, u32 mask, rgb_t const *lookup)
{
	u32 const addrmask = regs.rev1_or_2() ? 0x0fffff : 0xfffff0;
	u32 const addrshift = regs.rev1_or_2() ? 3 : 0;

	m_ram = ram;
	m_mask = mask;
	m_lookup = lookup;

	// extract LOD parameters
	auto const texlod = regs.texture_lod();
	m_lodmin = texlod.lod_min() << 6;
	m_lodmax = texlod.lod_max() << 6;
	m_lodbias = s8(texlod.lod_bias() << 2) << 4;

	// determine which LODs are present
	m_lodmask = 0x1ff;
	if (texlod.lod_tsplit())
		m_lodmask = texlod.lod_odd() ? 0x0aa : 0x155;

	// determine base texture width/height
	m_wmask = m_hmask = 0xff;
	if (texlod.lod_s_is_wider())
		m_hmask >>= texlod.lod_aspect();
	else
		m_wmask >>= texlod.lod_aspect();

	// determine the bpp of the texture
	auto const texmode = regs.texture_mode();
	int bppscale = texmode.format() >> 3;

	// start with the base of LOD 0
	u32 base = regs.texture_baseaddr();
	if (addrshift == 0 && BIT(base, 0) != 0)
		osd_printf_debug("Tiled texture\n");
	base = (base & addrmask) << addrshift;
	m_lodoffset[0] = base & mask;

	// LODs 1-3 are different depending on whether we are in multitex mode
	// Several Voodoo 2 games leave the upper bits of TLOD == 0xff, meaning we think
	// they want multitex mode when they really don't -- disable for now
	// Enable for Voodoo 3 or Viper breaks - VL.
	// Add check for upper nibble not equal to zero to fix funkball -- TG
	if (texlod.tmultibaseaddr() && texlod.magic() == 0)
	{
		base = (regs.texture_baseaddr_1() & addrmask) << addrshift;
		m_lodoffset[1] = base & mask;
		base = (regs.texture_baseaddr_2() & addrmask) << addrshift;
		m_lodoffset[2] = base & mask;
		base = (regs.texture_baseaddr_3_8() & addrmask) << addrshift;
		m_lodoffset[3] = base & mask;
	}
	else
	{
		if (m_lodmask & (1 << 0))
			base += (((m_wmask >> 0) + 1) * ((m_hmask >> 0) + 1)) << bppscale;
		m_lodoffset[1] = base & mask;
		if (m_lodmask & (1 << 1))
			base += (((m_wmask >> 1) + 1) * ((m_hmask >> 1) + 1)) << bppscale;
		m_lodoffset[2] = base & mask;
		if (m_lodmask & (1 << 2))
			base += (((m_wmask >> 2) + 1) * ((m_hmask >> 2) + 1)) << bppscale;
		m_lodoffset[3] = base & mask;
	}

	// remaining LODs make sense
	for (int lod = 4; lod <= 8; lod++)
	{
		if (m_lodmask & (1 << (lod - 1)))
		{
			u32 size = ((m_wmask >> (lod - 1)) + 1) * ((m_hmask >> (lod - 1)) + 1);
			if (size < 4) size = 4;
			base += size << bppscale;
		}
		m_lodoffset[lod] = base & mask;
	}

	// compute the detail parameters
	auto const texdetail = regs.texture_detail();
	m_detailmax = texdetail.detail_max();
	m_detailbias = s8(texdetail.detail_bias() << 2) << 6;
	m_detailscale = texdetail.detail_scale();

	// check for separate RGBA filtering
	if (texdetail.separate_rgba_filter())
		fatalerror("Separate RGBA filters!\n");

	m_bilinear_mask = regs.rev2_or_3() ? 0xff : 0xf0;
}


//-------------------------------------------------
//  lookup_single_texel - look up the texel at the
//  given S,T coordinate based on the format and
//  return a decoded RGB value
//-------------------------------------------------

inline rgb_t rasterizer_texture::lookup_single_texel(u32 format, u32 texbase, s32 s, s32 t)
{
	if (format < 8)
		return m_lookup[*(u8 *)&m_ram[(texbase + t + s) & m_mask]];
	else if (format >= 10 && format <= 12)
		return m_lookup[*(u16 *)&m_ram[(texbase + 2*(t + s)) & m_mask]];
	else
	{
		u32 texel = *(u16 *)&m_ram[(texbase + 2*(t + s)) & m_mask];
		return (m_lookup[texel & 0xff] & 0xffffff) | ((texel & 0xff00) << 16);
	}
}


//-------------------------------------------------
//  fetch_texel - fetch the texel value based on
//  the S,T coordinates and LOD
//-------------------------------------------------

inline rgbaint_t ATTR_FORCE_INLINE rasterizer_texture::fetch_texel(reg_texture_mode const texmode, dither_helper const &dither, s32 x, const stw_helper &iterstw, s32 lodbase, s32 &lod)
{
	lod = lodbase;

	// determine the S/T/LOD values for this texture
	s32 s, t;
	if (texmode.enable_perspective())
	{
		s32 wlog;
		iterstw.calc_stow(s, t, wlog);
		lod += wlog;
	}
	else
		iterstw.get_st_shiftr(s, t, 14 + 10);

	// clamp W
	if (texmode.clamp_neg_w() && iterstw.is_w_neg())
		s = t = 0;

	// clamp the LOD
	lod += m_lodbias;
	if (texmode.enable_lod_dither())
		lod += dither.raw_4x4(x) << 4;
	lod = std::clamp<s32>(lod, m_lodmin, m_lodmax);

	// now the LOD is in range; if we don't own this LOD, take the next one
	s32 ilod = lod >> 8;
	ilod += (~m_lodmask >> ilod) & 1;

	// fetch the texture base
	u32 texbase = m_lodoffset[ilod];

	// compute the maximum s and t values at this LOD
	s32 smax = m_wmask >> ilod;
	s32 tmax = m_hmask >> ilod;

	// determine whether we are point-sampled or bilinear
	rgbaint_t result;
	if ((lod == m_lodmin && !texmode.magnification_filter()) || (lod != m_lodmin && !texmode.minification_filter()))
	{
		// adjust S/T for the LOD and strip off the fractions
		ilod += 18 - 10;
		s >>= ilod;
		t >>= ilod;

		// clamp/wrap S/T if necessary
		if (texmode.clamp_s())
			s = std::clamp(s, 0, smax);
		if (texmode.clamp_t())
			t = std::clamp(t, 0, tmax);
		s &= smax;
		t &= tmax;
		t *= smax + 1;

		// fetch texel data
		result.set(lookup_single_texel(texmode.format(), texbase, s, t));
	}
	else
	{
		// adjust S/T for the LOD and strip off all but the low 8 bits of the fraction
		s >>= ilod;
		t >>= ilod;

		// also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel
		s -= 0x80;
		t -= 0x80;

		// extract the fractions
		u32 sfrac = s & m_bilinear_mask;
		u32 tfrac = t & m_bilinear_mask;

		// now toss the rest
		s >>= 8;
		t >>= 8;
		s32 s1 = s + 1;
		s32 t1 = t + 1;

		// clamp/wrap S/T if necessary
		if (texmode.clamp_s())
		{
			if (s < 0)
				s = s1 = 0;
			else if (s >= smax)
				s = s1 = smax;
		}
		s &= smax;
		s1 &= smax;

		if (texmode.clamp_t())
		{
			if (t < 0)
				t = t1 = 0;
			else if (t >= tmax)
				t = t1 = tmax;
		}
		t &= tmax;
		t1 &= tmax;
		t *= smax + 1;
		t1 *= smax + 1;

		// fetch texel data
		u32 texel0 = lookup_single_texel(texmode.format(), texbase, s, t);
		u32 texel1 = lookup_single_texel(texmode.format(), texbase, s1, t);
		u32 texel2 = lookup_single_texel(texmode.format(), texbase, s, t1);
		u32 texel3 = lookup_single_texel(texmode.format(), texbase, s1, t1);
		result.bilinear_filter_rgbaint(texel0, texel1, texel2, texel3, sfrac, tfrac);
	}
	return result;
}


//-------------------------------------------------
//  combine_texture - color combine unit for
//  texture pixels
//-------------------------------------------------

inline rgbaint_t ATTR_FORCE_INLINE rasterizer_texture::combine_texture(reg_texture_mode const texmode, rgbaint_t const &c_local, rgbaint_t const &c_other, s32 lod)
{
	// select zero/other for RGB
	rgbaint_t blend_color;
	if (texmode.tc_zero_other())
		blend_color.zero();
	else
		blend_color.set(c_other);

	// select zero/other for alpha
	if (texmode.tca_zero_other())
		blend_color.zero_alpha();
	else
		blend_color.merge_alpha16(c_other);

	// subtract local color
	if (texmode.tc_sub_clocal() || texmode.tca_sub_clocal())
	{
		rgbaint_t sub_val;

		// potentially subtract c_local
		if (!texmode.tc_sub_clocal())
			sub_val.zero();
		else
			sub_val.set(c_local);

		if (!texmode.tca_sub_clocal())
			sub_val.zero_alpha();
		else
			sub_val.merge_alpha16(c_local);

		blend_color.sub(sub_val);
	}

	// blend RGB
	rgbaint_t blend_factor;
	switch (texmode.tc_mselect())
	{
		default:    // reserved
		case 0:     // zero
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

		case 4:     // LOD (detail factor)
			if (m_detailbias <= lod)
				blend_factor.zero();
			else
			{
				u8 tmp;
				tmp = (((m_detailbias - lod) << m_detailscale) >> 8);
				if (tmp > m_detailmax)
					tmp = m_detailmax;
				blend_factor.set_all(tmp);
			}
			break;

		case 5:     // LOD fraction
			blend_factor.set_all(lod & 0xff);
			break;
	}

	// blend alpha
	switch (texmode.tca_mselect())
	{
		default:    // reserved
		case 0:     // zero
			blend_factor.zero_alpha();
			break;

		case 1:     // c_local
			blend_factor.merge_alpha16(c_local);
			break;

		case 2:     // a_other
			blend_factor.merge_alpha16(c_other);
			break;

		case 3:     // a_local
			blend_factor.merge_alpha16(c_local);
			break;

		case 4:     // LOD (detail factor)
			if (m_detailbias <= lod)
				blend_factor.zero_alpha();
			else
			{
				u8 tmp;
				tmp = (((m_detailbias - lod) << m_detailscale) >> 8);
				if (tmp > m_detailmax)
					tmp = m_detailmax;
				blend_factor.set_a16(tmp);
			}
			break;

		case 5:     // LOD fraction
			blend_factor.set_a16(lod & 0xff);
			break;
	}

	// reverse the RGB blend
	if (!texmode.tc_reverse_blend())
		blend_factor.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	// reverse the alpha blend
	if (!texmode.tca_reverse_blend())
		blend_factor.xor_imm_rgba(0xff, 0, 0, 0);

	blend_factor.add_imm(1);

	// add clocal or alocal to RGB
	rgbaint_t add_val;
	switch (texmode.tc_add_aclocal())
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
	if (!texmode.tca_add_aclocal())
		add_val.zero_alpha();
	else
		add_val.merge_alpha16(c_local);

	// scale add and clamp
	blend_color.scale_add_and_clamp(blend_factor, add_val);

	// invert
	if (texmode.tc_invert_output())
		blend_color.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	if (texmode.tca_invert_output())
		blend_color.xor_imm_rgba(0xff, 0, 0, 0);
	return blend_color;
}



//**************************************************************************
//  RASTERIZER NCC TEXELS
//**************************************************************************

//-------------------------------------------------
//  recompute - constructor
//-------------------------------------------------

void rasterizer_palette::compute_ncc(u32 const *regs)
{
	// generate all 256 possibilities
	u8 const *ybase = reinterpret_cast<u8 const *>(regs);
	for (int index = 0; index < 256; index++)
	{
		// start with the intensity
		s32 y = ybase[BYTE4_XOR_LE(BIT(index, 4, 4))];

		// look up the I/Q values
		s32 i = regs[4 + BIT(index, 2, 2)];
		s32 q = regs[8 + BIT(index, 0, 2)];

		// add the coloring
		s32 r = std::clamp(y + (s32(i <<  5) >> 23) + (s32(q <<  5) >> 23), 0, 255);
		s32 g = std::clamp(y + (s32(i << 14) >> 23) + (s32(q << 14) >> 23), 0, 255);
		s32 b = std::clamp(y + (s32(i << 23) >> 23) + (s32(q << 23) >> 23), 0, 255);

		// fill in the table
		m_texel[index] = rgb_t(0xff, r, g, b);
	}
}



//**************************************************************************
//  VOODOO RENDERER
//**************************************************************************

//-------------------------------------------------
//  voodoo_renderer - constructor
//-------------------------------------------------

voodoo_renderer::voodoo_renderer(running_machine &machine, u8 type, u16 tmu_config, const rgb_t *rgb565, voodoo_regs &fbi_regs, voodoo_regs *tmu0_regs, voodoo_regs *tmu1_regs) :
	poly_manager(machine),
	m_type(type),
	m_tmu_config(tmu_config),
	m_rowpixels(0),
	m_yorigin(0),
	m_fbi_reg(fbi_regs),
	m_tmu0_reg(tmu0_regs),
	m_tmu1_reg(tmu1_regs),
	m_rgb565(rgb565),
	m_fogdelta_mask((type < TYPE_VOODOO_2) ? 0xff : 0xfc),
	m_thread_stats(WORK_MAX_THREADS)
{
	// empty the hash table
	std::fill(std::begin(m_raster_hash), std::end(m_raster_hash), nullptr);

	// add all predefined rasterizers
	for (static_rasterizer_info const *info = s_predef_raster_table; info->params.fbzcp().raw() != 0xffffffff; info++)
		add_rasterizer(info->params, info->mfp, false);

	// create entries for the generic rasterizers as well
	rasterizer_params dummy_params;
	for (int index = 0; index < 4; index++)
		m_generic_rasterizer[index] = add_rasterizer(dummy_params, generic_rasterizer(index), true);
}


//-------------------------------------------------
//  alloc_poly - allocate a new poly_data object
//  and compute the raster parameters
//-------------------------------------------------

poly_data &voodoo_renderer::alloc_poly()
{
	// allocate poly data and compute the rasterization parameters
	poly_data &poly = object_data_alloc();
	poly.raster.compute(m_fbi_reg, m_tmu0_reg, m_tmu1_reg);
	return poly;
}


//-------------------------------------------------
//  enqueue_fastfill - enqueue a fastfill operation
//  using a custom renderer
//-------------------------------------------------

u32 voodoo_renderer::enqueue_fastfill(poly_data &poly)
{
	// if we're not clearing anything, take no time
	auto const fbzmode = poly.raster.fbzmode();
	if (!fbzmode.rgb_buffer_mask() && !fbzmode.aux_buffer_mask())
		return 0;

	// generate a dither pattern if clearing the RGB buffer
	if (fbzmode.rgb_buffer_mask())
	{
		for (int y = 0; y < 4; y++)
		{
			dither_helper dither(y, fbzmode);
			for (int x = 0; x < 4; x++)
				poly.dither[y * 4 + x] = dither.pixel(x, poly.color1);
		}
	}

	// create a block of 64 identical extents
	voodoo_renderer::extent_t extents[64];
	extents[0].startx = poly.clipleft;
	extents[0].stopx = poly.clipright;
	std::fill(std::begin(extents) + 1, std::end(extents), extents[0]);

	// iterate over the full area
	voodoo_renderer::render_delegate fastfill_cb(&voodoo_renderer::rasterizer_fastfill, this);
	u32 pixels = 0;
	for (int y = poly.cliptop; y < poly.clipbottom; y += std::size(extents))
	{
		int numextents = std::min(poly.clipbottom - y, int(std::size(extents)));
		pixels += render_triangle_custom(global_cliprect, fastfill_cb, y, numextents, extents);
	}
	return pixels;
}


//-------------------------------------------------
//  enqueue_triangle - enqueue a triangle
//-------------------------------------------------

u32 voodoo_renderer::enqueue_triangle(poly_data &poly, vertex_t const *vert)
{
	// compute the hash of the raster parameters
	u32 fullhash = poly.raster.hash();
	u32 hash = fullhash % RASTER_HASH_SIZE;

	// find the appropriate hash entry
	rasterizer_info *prev = nullptr;
	rasterizer_info *info;
	for (info = m_raster_hash[hash]; info != nullptr; prev = info, info = info->next)
		if (info->fullhash == fullhash && info->params == poly.raster)
		{
			// got it, move us to the head of the list
			if (prev != nullptr)
			{
				prev->next = info->next;
				info->next = m_raster_hash[hash];
				m_raster_hash[hash] = info;
			}
			break;
		}

	// determine the index of the generic rasterizer
	if (info == nullptr)
	{
		u32 index = 0;
		if (poly.raster.texmode0().raw() != 0xffffffff)
			index |= 1;
		if (poly.raster.texmode1().raw() != 0xffffffff)
			index |= 2;

		// add a new one if we're logging usage
		if (LOG_RASTERIZERS)
			info = add_rasterizer(poly.raster, generic_rasterizer(index), true);
		else
			info = m_generic_rasterizer[index];
	}

	// set the info and render the triangle
	poly.info = info;
	return render_triangle(global_cliprect, poly.info->callback, 0, vert[0], vert[1], vert[2]);
}


//-------------------------------------------------
//  stipple_test - test against the stipple
//  pattern; the enable flag is not checked here,
//  so must be checked by the caller
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::stipple_test(thread_stats_block &threadstats, reg_fbz_mode const fbzmode, s32 x, s32 scry, u32 &stipple)
{
	// rotate mode
	if (fbzmode.stipple_pattern() == 0)
	{
		stipple = (stipple >> 1) | (stipple << 31);
		if (s32(stipple) >= 0)
		{
			threadstats.stipple_count++;
			return false;
		}
	}

	// pattern mode
	else
	{
		int stipple_index = ((scry & 3) << 3) | (~x & 7);
		if (BIT(stipple, stipple_index) == 0)
		{
			threadstats.stipple_count++;
			return false;
		}
	}
	return true;
}


//-------------------------------------------------
//  compute_depthval - compute the value for depth
//  according to the configured flags
//-------------------------------------------------

inline s32 ATTR_FORCE_INLINE voodoo_renderer::compute_depthval(poly_data const &poly, reg_fbz_mode const fbzmode, reg_fbz_colorpath const fbzcp, s32 wfloat, s32 iterz)
{
	s32 result;
	if (fbzmode.wbuffer_select())
	{
		if (!fbzmode.depth_float_select())
			result = wfloat;
		else if ((iterz & 0xf0000000) != 0)
			result = 0x0000;
		else if ((iterz & 0x0ffff000) == 0)
			result = 0xffff;
		else
		{
			int exp = count_leading_zeros(iterz) - 4;
			return ((exp << 12) | ((iterz >> (15 - exp)) ^ 0x1fff)) + 1;
		}
	}
	else
		result = clamped_z(iterz, fbzcp);

	if (fbzmode.enable_depth_bias())
		result = std::clamp(result + s16(poly.zacolor), 0, 0xffff);

	return result;
}


//-------------------------------------------------
//  depth_test - perform depth testing; the enable
//  flag is not checked here, so must be checked by
//  the caller
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::depth_test(thread_stats_block &threadstats, poly_data const &poly, reg_fbz_mode const fbzmode, s32 depthdest, s32 depthval)
{
	// the source depth is either the iterated W/Z+bias or a constant value
	s32 depthsource = (fbzmode.depth_source_compare() == 0) ? depthval : u16(poly.zacolor);

	// test against the depth buffer
	switch (fbzmode.depth_function())
	{
		case 0:     // depthOP = never
			threadstats.zfunc_fail++;
			return false;

		case 1:     // depthOP = less than
			if (depthsource >= depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 2:     // depthOP = equal
			if (depthsource != depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 3:     // depthOP = less than or equal
			if (depthsource > depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 4:     // depthOP = greater than
			if (depthsource <= depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 5:     // depthOP = not equal
			if (depthsource == depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 6:     // depthOP = greater than or equal
			if (depthsource < depthdest)
			{
				threadstats.zfunc_fail++;
				return false;
			}
			break;

		case 7:     // depthOP = always
			break;
	}
	return true;
}


//-------------------------------------------------
//  combine_color - core color combining logic
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::combine_color(rgbaint_t &color, thread_stats_block &threadstats, poly_data const &poly, reg_fbz_colorpath const fbzcp, reg_fbz_mode const fbzmode, rgbaint_t texel, s32 iterz, s64 iterw, rgb_t chromakey)
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
			c_other.set(poly.color1);
			break;

		default:    // reserved - voodoo3 LFB RGB
			c_other.zero();
			break;
	}

	// handle chroma key
	if (fbzmode.enable_chromakey())
		if (!chroma_key_test(threadstats, c_other, chromakey))
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
			c_other.set_a16(poly.color1.a());
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
			c_local.set(poly.color0);
	}
	else
	{
		if (!(texel.get_a() & 0x80))        // iterated RGB
			c_local.set(color);
		else                                // color0 RGB
			c_local.set(poly.color0);
	}

	// compute a_local
	switch (fbzcp.cca_localselect())
	{
		default:
		case 0:     // iterated alpha
			c_local.merge_alpha16(color);
			break;

		case 1:     // color0 alpha
			c_local.set_a16(poly.color0.a());
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


//-------------------------------------------------
//  alpha_mask_test - perform alpha mask testing;
//  the enable flag is not checked here, so must
//  be checked by the caller
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::alpha_mask_test(thread_stats_block &threadstats, u32 alpha)
{
	if ((alpha & 1) == 0)
	{
		threadstats.afunc_fail++;
		return false;
	}
	return true;
}


//-------------------------------------------------
//  alpha_test - perform alpha testing; the enable
//  flag is not checked here, so must be checked
//  by the caller
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::alpha_test(thread_stats_block &threadstats, reg_alpha_mode const alphamode, u32 alpha, u32 alpharef)
{
	switch (alphamode.alphafunction())
	{
		case 0:     // alphaOP = never
			threadstats.afunc_fail++;
			return false;

		case 1:     // alphaOP = less than
			if (alpha >= alpharef)
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 2:     // alphaOP = equal
			if (alpha != alpharef)
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 3:     // alphaOP = less than or equal
			if (alpha > alpharef)
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 4:     // alphaOP = greater than
			if (alpha <= alpharef)
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 5:     // alphaOP = not equal
			if (alpha == alpharef)
			{
				threadstats.afunc_fail++;
				return false;
			}
			break;

		case 6:     // alphaOP = greater than or equal
			if (alpha < alpharef)
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


//-------------------------------------------------
//  chroma_key_test - perform chroma key testing;
//  the enable flag is not checked here, so must
//  be checked by the caller
//-------------------------------------------------

inline bool ATTR_FORCE_INLINE voodoo_renderer::chroma_key_test(thread_stats_block &threadstats, rgbaint_t const &colorin, rgb_t chromakey)
{
	rgb_t color = colorin.to_rgba();

	// non-range version
	auto const chromarange = m_fbi_reg.chroma_range();
	if (!chromarange.enable())
	{
		if (((color ^ chromakey) & 0xffffff) == 0)
		{
			threadstats.chroma_fail++;
			return false;
		}
	}

	// tricky range version
	else
	{
		s32 low, high, test;
		int results;

		// check blue
		low = chromakey.b();
		high = chromarange.blue();
		test = color.b();
		results = (test >= low && test <= high);
		results ^= chromarange.blue_exclusive();
		results <<= 1;

		// check green
		low = chromakey.g();
		high = chromarange.green();
		test = color.g();
		results |= (test >= low && test <= high);
		results ^= chromarange.green_exclusive();
		results <<= 1;

		// check red
		low = chromakey.r();
		high = chromarange.red();
		test = color.r();
		results |= (test >= low && test <= high);
		results ^= chromarange.red_exclusive();

		// final result
		if (chromarange.union_mode())
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
	return true;
}


//-------------------------------------------------
//  apply_fogging - perform fogging; the enable
//  flag is not checked here, so must be checked by
//  the caller
//-------------------------------------------------

inline void ATTR_FORCE_INLINE voodoo_renderer::apply_fogging(rgbaint_t &color, poly_data const &poly, reg_fbz_mode const fbzmode, reg_fog_mode const fogmode, reg_fbz_colorpath const fbzcp, s32 x, dither_helper const &dither, s32 wfloat, s32 iterz, s64 iterw, rgbaint_t const &iterargb)
{
	// constant fog bypasses everything else
	rgbaint_t fog_color_local(poly.fogcolor);
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
					fog_depth = std::clamp(fog_depth + s16(poly.zacolor), 0, 0xffff);

				// perform the multiply against lower 8 bits of wfloat
				s32 delta = m_fogdelta[fog_depth >> 10];
				s32 deltaval = (delta & m_fogdelta_mask) * ((fog_depth >> 2) & 0xff);

				// fog zones allow for negating this value
				if (fogmode.fog_zones() && (delta & 2))
					deltaval = -deltaval;
				deltaval >>= 6;

				// apply dither
				if (fogmode.fog_dither())
					deltaval += dither.raw_4x4(x);
				deltaval >>= 4;

				// add to the blending factor
				fogblend = m_fogblend[fog_depth >> 10] + deltaval;
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


//-------------------------------------------------
//  alpha_blend - perform alpha blending; the
//  enable flag is not checked here, so must be
//  checked by the caller
//-------------------------------------------------

inline void ATTR_FORCE_INLINE voodoo_renderer::alpha_blend(rgbaint_t &color, reg_fbz_mode const fbzmode, reg_alpha_mode const alphamode, s32 x, dither_helper const &dither, int dpix, u16 *depth, rgbaint_t const &prefog)
{
	// extract destination pixel
	rgbaint_t dst_color(m_rgb565[dpix]);
	int da = 0xff;
	if (fbzmode.enable_alpha_planes())
		dst_color.set_a16(da = depth[x]);

	// apply dither subtraction
	if (fbzmode.alpha_dither_subtract())
	{
		int dith = dither.subtract(x);
		dst_color.add_imm_rgba(0, dith, dith >> 1, dith);
	}

	// compute source portion
	int sa = color.get_a();
	int ta;
	rgbaint_t src_scale;
	switch (alphamode.srcrgbblend())
	{
		default:    // reserved
		case 0:     // AZERO
			src_scale.zero();
			break;

		case 1:     // ASRC_ALPHA
			ta = sa + 1;
			src_scale.set_all(ta);
			break;

		case 2:     // A_COLOR
			src_scale = dst_color;
			src_scale.add_imm(1);
			break;

		case 3:     // ADST_ALPHA
			ta = da + 1;
			src_scale.set_all(ta);
			break;

		case 4:     // AONE
			src_scale.set_all(256);
			break;

		case 5:     // AOMSRC_ALPHA
			ta = 0x100 - sa;
			src_scale.set_all(ta);
			break;

		case 6:     // AOM_COLOR
			src_scale.set_all(0x100);
			src_scale.sub(dst_color);
			break;

		case 7:     // AOMDST_ALPHA
			ta = 0x100 - da;
			src_scale.set_all(ta);
			break;

		case 15:    // ASATURATE
			ta = 0x100 - da;
			if (sa < ta)
				ta = sa;
			ta++;
			src_scale.set_all(ta);
			break;
	}

	// set src_scale alpha
	int src_alpha_scale = (alphamode.srcalphablend() == 4) ? 256 : 0;
	src_scale.set_a16(src_alpha_scale);

	// add in dest portion
	rgbaint_t dst_scale;
	switch (alphamode.dstrgbblend())
	{
		default:    // reserved
		case 0:     // AZERO
			dst_scale.zero();
			break;

		case 1:     // ASRC_ALPHA
			ta = sa + 1;
			dst_scale.set_all(ta);
			break;

		case 2:     // A_COLOR
			dst_scale.set(color);
			dst_scale.add_imm(1);
			break;

		case 3:     // ADST_ALPHA
			ta = da + 1;
			dst_scale.set_all(ta);
			break;

		case 4:     // AONE
			dst_scale.set_all(256);
			break;

		case 5:     // AOMSRC_ALPHA
			ta = 0x100 - sa;
			dst_scale.set_all(ta);
			break;

		case 6:     // AOM_COLOR
			dst_scale.set_all(0x100);
			dst_scale.sub(color);
			break;

		case 7:     // AOMDST_ALPHA
			ta = 0x100 - da;
			dst_scale.set_all(ta);
			break;

		case 15:    // A_COLORBEFOREFOG
			dst_scale.set(prefog);
			dst_scale.add_imm(1);
			break;
	}

	// set dst_scale alpha
	int dest_alpha_scale = (alphamode.dstalphablend() == 4) ? 256 : 0;
	dst_scale.set_a16(dest_alpha_scale);

	// main blend
	color.scale2_add_and_clamp(src_scale, dst_color, dst_scale);
}


//-------------------------------------------------
//  write_pixel - write the pixel to the
//  destination buffer, and the depth to the depth
//  buffer
//-------------------------------------------------

inline void ATTR_FORCE_INLINE voodoo_renderer::write_pixel(thread_stats_block &threadstats, reg_fbz_mode const fbzmode, dither_helper const &dither, u16 *destbase, u16 *depthbase, s32 x, rgbaint_t const &color, s32 depthval)
{
	// write to framebuffer
	if (fbzmode.rgb_buffer_mask())
		destbase[x] = dither.pixel(x, color);

	// write to aux buffer
	if (fbzmode.aux_buffer_mask())
	{
		if (fbzmode.enable_alpha_planes() == 0)
			depthbase[x] = depthval;
		else
			depthbase[x] = color.get_a();
	}

	// track pixel writes to the frame buffer regardless of mask
	threadstats.pixels_out++;
}


//-------------------------------------------------
//  pixel_pipeline - run a pixel through the
//  pipeline
//-------------------------------------------------

void voodoo_renderer::pixel_pipeline(thread_stats_block &threadstats, poly_data const &poly, reg_lfb_mode const lfbmode, s32 x, s32 scry, rgb_t src_color, u16 sz)
{
	auto const fbzcp = poly.raster.fbzcp();
	auto const alphamode = poly.raster.alphamode();
	auto const fbzmode = poly.raster.fbzmode();
	auto const fogmode = poly.raster.fogmode();
	dither_helper dither(scry, fbzmode, fogmode);
	u16 *depth = poly.depthbase;
	u16 *dest = poly.destbase;
	u32 stipple = poly.stipple;

	threadstats.pixels_in++;

	// apply clipping
	if (fbzmode.enable_clipping())
	{
		if (x < poly.clipleft || x >= poly.clipright || scry < poly.cliptop || scry >= poly.clipbottom)
		{
			threadstats.clip_fail++;
			return;
		}
	}

	// handle stippling
	if (fbzmode.enable_stipple() && !stipple_test(threadstats, fbzmode, x, scry, stipple))
		return;

	// Depth testing value for lfb pipeline writes is directly from write data, no biasing is used
	s32 depthval = u32(sz);

	// Perform depth testing
	if (fbzmode.enable_depthbuf() && !depth_test(threadstats, poly, fbzmode, depth[x], depthval))
		return;

	// use the RGBA we stashed above
	rgbaint_t color(src_color);

	// handle chroma key
	if (fbzmode.enable_chromakey() && !chroma_key_test(threadstats, color, poly.chromakey))
		return;

	// handle alpha mask
	if (fbzmode.enable_alpha_mask() && !alpha_mask_test(threadstats, color.get_a()))
		return;

	// handle alpha test
	if (alphamode.alphatest() && !alpha_test(threadstats, alphamode, color.get_a(), alphamode.alpharef()))
		return;

	// perform fogging
	rgbaint_t prefog(color);
	if (fogmode.enable_fog())
	{
		s32 iterz = sz << 12;
		s64 iterw = lfbmode.write_w_select() ? u32(poly.zacolor << 16) : u32(sz << 16);
		apply_fogging(color, poly, fbzmode, fogmode, fbzcp, x, dither, depthval, iterz, iterw, rgbaint_t(0));
	}

	// wait for any outstanding work to finish
	wait("LFB Write");

	// perform alpha blending
	if (alphamode.alphablend())
		alpha_blend(color, fbzmode, alphamode, x, dither, dest[x], depth, prefog);

	// pixel pipeline part 2 handles final output
	write_pixel(threadstats, fbzmode, dither, dest, depth, x, color, depthval);
}


//-------------------------------------------------
//  rasterizer - core scanline rasterizer
//-------------------------------------------------

template<u32 _FbzCp, u32 _FbzMode, u32 _AlphaMode, u32 _FogMode, u32 _TexMode0, u32 _TexMode1>
void voodoo_renderer::rasterizer(s32 y, const voodoo_renderer::extent_t &extent, const poly_data &poly, int threadid)
{
	thread_stats_block &threadstats = m_thread_stats[threadid];
	reg_texture_mode const texmode0(_TexMode0, (_TexMode0 == 0xffffffff) ? 0 : poly.raster.texmode0());
	reg_texture_mode const texmode1(_TexMode1, (_TexMode1 == 0xffffffff) ? 0 : poly.raster.texmode1());
	reg_fbz_colorpath const fbzcp(_FbzCp, poly.raster.fbzcp());
	reg_alpha_mode const alphamode(_AlphaMode, poly.raster.alphamode());
	reg_fbz_mode const fbzmode(_FbzMode, poly.raster.fbzmode());
	reg_fog_mode const fogmode(_FogMode, poly.raster.fogmode());
	stw_helper iterstw0, iterstw1;
	stw_helper deltastw0, deltastw1;
	u32 stipple = poly.stipple;

	// determine the screen Y
	s32 scry = y;
	if (fbzmode.y_origin())
		scry = m_yorigin - y;

	// pre-increment the pixels_in unconditionally
	s32 startx = extent.startx;
	s32 stopx = extent.stopx;
	threadstats.pixels_in += stopx - startx;

	// apply clipping
	if (fbzmode.enable_clipping())
	{
		// Y clipping buys us the whole scanline
		if (scry < poly.cliptop || scry >= poly.clipbottom)
		{
			threadstats.clip_fail += stopx - startx;
			return;
		}

		// X clipping
		s32 tempclip = poly.clipright;

		// check for start outsize of clipping boundary
		if (startx >= tempclip)
		{
			threadstats.clip_fail += stopx - startx;
			return;
		}

		// clip the right side
		if (stopx > tempclip)
		{
			threadstats.clip_fail += stopx - tempclip;
			stopx = tempclip;
		}

		// clip the left side
		tempclip = poly.clipleft;
		if (startx < tempclip)
		{
			threadstats.clip_fail += tempclip - startx;
			startx = tempclip;
		}
	}

	// get pointers to the target buffer and depth buffer
	u16 *dest = poly.destbase + scry * m_rowpixels;
	u16 *depth = poly.depthbase + scry * m_rowpixels;

	// compute the starting parameters
	s32 dx = startx - (poly.ax >> 4);
	s32 dy = y - (poly.ay >> 4);
	s32 iterr = (poly.startr + dy * poly.drdy + dx * poly.drdx) << 8;
	s32 iterg = (poly.startg + dy * poly.dgdy + dx * poly.dgdx) << 8;
	s32 iterb = (poly.startb + dy * poly.dbdy + dx * poly.dbdx) << 8;
	s32 itera = (poly.starta + dy * poly.dady + dx * poly.dadx) << 8;

	rgbaint_t iterargb, iterargb_delta;
	iterargb.set(itera, iterr, iterg, iterb);
	iterargb_delta.set(poly.dadx, poly.drdx, poly.dgdx, poly.dbdx);
	iterargb_delta.shl_imm(8);
	s32 iterz = poly.startz + dy * poly.dzdy + dx * poly.dzdx;
	s64 iterw = (poly.startw + dy * poly.dwdy + dx * poly.dwdx) << 16;
	s64 iterw_delta = poly.dwdx << 16;
	if (_TexMode0 != 0xffffffff)
	{
		iterstw0.set(
			poly.starts0 + dy * poly.ds0dy + dx * poly.ds0dx,
			poly.startt0 + dy * poly.dt0dy + dx * poly.dt0dx,
			poly.startw0 + dy * poly.dw0dy + dx * poly.dw0dx);
		deltastw0.set(poly.ds0dx, poly.dt0dx, poly.dw0dx);
	}
	if (_TexMode1 != 0xffffffff)
	{
		iterstw1.set(
			poly.starts1 + dy * poly.ds1dy + dx * poly.ds1dx,
			poly.startt1 + dy * poly.dt1dy + dx * poly.dt1dx,
			poly.startw1 + dy * poly.dw1dy + dx * poly.dw1dx);
		deltastw1.set(poly.ds1dx, poly.dt1dx, poly.dw1dx);
	}
	poly.info->scanlines++;

	// loop in X
	dither_helper dither(scry, fbzmode, fogmode);
	for (s32 x = startx; x < stopx; x++)
	{
		do
		{
			// handle stippling
			if (fbzmode.enable_stipple() && !stipple_test(threadstats, fbzmode, x, scry, stipple))
				break;

			// compute "floating point" W value (used for depth and fog)
			s32 wfloat = compute_wfloat(iterw);

			// compute depth value (W or Z) for this pixel
			s32 depthval = compute_depthval(poly, fbzmode, fbzcp, wfloat, iterz);

			// depth testing
			if (fbzmode.enable_depthbuf() && !depth_test(threadstats, poly, fbzmode, depth[x], depthval))
				break;

			// run the texture pipeline on TMU1 to produce a value in texel
			rgbaint_t texel(0);
			if (_TexMode1 != 0xffffffff)
			{
				s32 lod1;
				rgbaint_t texel_t1 = poly.tex1->fetch_texel(texmode1, dither, x, iterstw1, poly.lodbase1, lod1);
				texel = poly.tex1->combine_texture(texmode1, texel_t1, texel, lod1);
			}

			// run the texture pipeline on TMU0 to produce a final result in texel
			if (_TexMode0 != 0xffffffff)
			{
				// the seq_8_downld flag is repurposed in the rasterizer to indicate
				// we should send the configuration byte
				if (!texmode0.seq_8_downld())
				{
					s32 lod0;
					rgbaint_t texel_t0 = poly.tex0->fetch_texel(texmode0, dither, x, iterstw0, poly.lodbase0, lod0);
					texel = poly.tex0->combine_texture(texmode0, texel_t0, texel, lod0);
				}
				else
					texel.set(m_tmu_config);
			}

			// colorpath pipeline selects source colors and does blending
			rgbaint_t color = clamped_argb(iterargb, fbzcp);
			if (!combine_color(color, threadstats, poly, fbzcp, fbzmode, texel, iterz, iterw, poly.chromakey))
				break;

			// handle alpha test
			if (alphamode.alphatest() && !alpha_test(threadstats, alphamode, color.get_a(), alphamode.alpharef()))
				break;

			// perform fogging
			rgbaint_t prefog(color);
			if (fogmode.enable_fog())
				apply_fogging(color, poly, fbzmode, fogmode, fbzcp, x, dither, wfloat, iterz, iterw, iterargb);

			// perform alpha blending
			if (alphamode.alphablend())
				alpha_blend(color, fbzmode, alphamode, x, dither, dest[x], depth, prefog);

			// store the pixel and depth value
			write_pixel(threadstats, fbzmode, dither, dest, depth, x, color, depthval);
		} while (0);

		// update the iterated parameters
		iterargb += iterargb_delta;
		iterz += poly.dzdx;
		iterw += iterw_delta;
		if (_TexMode0 != 0xffffffff)
			iterstw0.add(deltastw0);
		if (_TexMode1 != 0xffffffff)
			iterstw1.add(deltastw1);
	}
}


//-------------------------------------------------
//  rasterizer_fastfill - custom scanline
//  rasterizer for fastfill operations
//-------------------------------------------------

void voodoo_renderer::rasterizer_fastfill(s32 y, const voodoo_renderer::extent_t &extent, const poly_data &poly, int threadid)
{
	thread_stats_block &threadstats = m_thread_stats[threadid];
	auto const fbzmode = poly.raster.fbzmode();
	s32 startx = extent.startx;
	s32 stopx = extent.stopx;
	int x;

	// determine the screen Y
	s32 scry = y;
	if (fbzmode.y_origin())
		scry = m_yorigin - y;

	// fill this RGB row
	if (fbzmode.rgb_buffer_mask())
	{
		const u16 *ditherow = &poly.dither[(y & 3) * 4];
		u64 expanded = *(u64 *)ditherow;
		u16 *dest = poly.destbase + scry * m_rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = ditherow[x & 3];
		for ( ; x < (stopx & ~3); x += 4)
			*(u64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = ditherow[x & 3];
		threadstats.pixels_out += stopx - startx;
	}

	// fill this dest buffer row
	if (fbzmode.aux_buffer_mask() && poly.depthbase != nullptr)
	{
		u16 depth = poly.zacolor;
		u64 expanded = (u64(depth) << 48) | (u64(depth) << 32) | (u64(depth) << 16) | u64(depth);
		u16 *dest = poly.depthbase + scry * m_rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = depth;
		for ( ; x < (stopx & ~3); x += 4)
			*(u64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = depth;
	}
}


//-------------------------------------------------
//  generic_rasterizer - return a pointer to a
//  generic rasterizer based on a texture enable
//  mask
//-------------------------------------------------

voodoo_renderer::rasterizer_mfp voodoo_renderer::generic_rasterizer(u8 texmask)
{
	switch (texmask & 3)
	{
	default:
	case 0:
		return &voodoo_renderer::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, 0xffffffff, 0xffffffff>;
	case 1:
		return &voodoo_renderer::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, 0xffffffff>;
	case 2:
		return &voodoo_renderer::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, 0xffffffff, reg_texture_mode::DECODE_LIVE>;
	case 3:
		return &voodoo_renderer::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>;
	}
}


//-------------------------------------------------
//  add_rasterizer - add a rasterizer to our
//  hash table
//-------------------------------------------------

rasterizer_info *voodoo_renderer::add_rasterizer(rasterizer_params const &params, rasterizer_mfp rasterizer, bool is_generic)
{
	rasterizer_info &info = m_rasterizer_list.emplace_back();

	// fill in the data
	info.next = nullptr;
	info.callback = voodoo_renderer::render_delegate(rasterizer, this);
	info.display = 0;
	info.is_generic = is_generic;
	info.scanlines = 0;
	info.polys = 0;
	info.fullhash = params.hash();
	info.params = params;

	// hook us into the hash table
	u32 hash = info.fullhash % RASTER_HASH_SIZE;
	if (LOG_RASTERIZERS || !is_generic)
	{
		info.next = m_raster_hash[hash];
		m_raster_hash[hash] = &info;
	}

	if (LOG_RASTERIZERS)
		printf("Adding rasterizer : cp=%08X am=%08X fog=%08X fbz=%08X tm0=%08X tm1=%08X (hash=%d)\n",
				params.fbzcp().raw(), params.alphamode().raw(), params.fogmode().raw(), params.fbzmode().raw(),
				params.texmode0().raw(), params.texmode1().raw(), hash);

	return &info;
}


//-------------------------------------------------
//  dump_rasterizer_stats - dump statistics on
//  the current rasterizer usage patterns
//-------------------------------------------------

void voodoo_renderer::dump_rasterizer_stats()
{
	if (!LOG_RASTERIZERS)
		return;

	static u8 display_index;
	rasterizer_info *cur, *best;
	int hash;

	printf("----\n");
	display_index++;

	// loop until we've displayed everything
	while (1)
	{
		best = nullptr;

		// find the highest entry
		for (hash = 0; hash < RASTER_HASH_SIZE; hash++)
			for (cur = m_raster_hash[hash]; cur != nullptr; cur = cur->next)
				if (cur->display != display_index && (best == nullptr || cur->scanlines > best->scanlines))
					best = cur;

		// if we're done, we're done
		if (best == nullptr || best->scanlines == 0)
			break;

		// print it
		printf("%s RASTERIZER( 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X ) // %2d %8d %10d\n",
			best->is_generic ? "   " : "// ",
			best->params.fbzcp().raw(),
			best->params.alphamode().raw(),
			best->params.fogmode().raw(),
			best->params.fbzmode().raw(),
			best->params.texmode0().raw(),
			best->params.texmode1().raw(),
			best->fullhash % RASTER_HASH_SIZE,
			best->polys,
			best->scanlines);

		// reset
		best->display = display_index;
	}
}


//-------------------------------------------------
//  reset_after_wait - handle a reset after a
//  wait operation by resetting our allocated
//  object queues
//-------------------------------------------------

void voodoo_renderer::reset_after_wait()
{
static int maxtex = 0;
	if (m_textures.count() > maxtex)
	{
		maxtex = m_textures.count();
		printf("Used %d textures\n", maxtex);
	}
	m_textures.reset();
static int maxpal = 0;
	if (m_palettes.count() > maxpal)
	{
		maxpal = m_palettes.count();
		printf("Used %d palettes\n", maxpal);
	}
	m_palettes.reset();
}


//**************************************************************************
//  GAME-SPECIFIC RASTERIZERS
//**************************************************************************

#define RASTERIZER(fbzcp, alpha, fog, fbz, tex0, tex1) \
	{ &voodoo_renderer::rasterizer<fbzcp, fbz, alpha, fog, tex0, tex1>, rasterizer_params(fbzcp, alpha, fog, fbz, tex0, tex1) },

static_rasterizer_info s_predef_raster_table[] =
{
	/* blitz ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*     284269  914846168 */
	RASTERIZER( 0x00002C35, 0x00515110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*     485421  440309121 */
	RASTERIZER( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*      31606  230753709 */
	RASTERIZER( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      76742  211701679 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /*       6188  152109056 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /*       1100  108134400 */
	RASTERIZER( 0x00002C35, 0x00515119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*    6229525  106197740 */
	RASTERIZER( 0x00002C35, 0x00515119, 0x00000000, 0x000B0799, 0x0C261A0F, 0xFFFFFFFF ) /*     905641   75886220 */
	RASTERIZER( 0x00002C35, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*     205236   53317253 */
	RASTERIZER( 0x01422439, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /*     817356   48881349 */
	RASTERIZER( 0x00000035, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*      37979   41687251 */
	RASTERIZER( 0x00002C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      26014   41183295 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*       2512   37911104 */
	RASTERIZER( 0x00006136, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*      28834   15527654 */
	RASTERIZER( 0x00582435, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*       9878    4979429 */
	RASTERIZER( 0x00002C35, 0x00515119, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /*     199952    4622064 */
	RASTERIZER( 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /*       8672    3676949 */
	RASTERIZER( 0x00582C35, 0x00515010, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /*        616    2743972 */
	RASTERIZER( 0x01422C39, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      81380    2494832 */
	//RASTERIZER( 0x00582435, 0x00515110, 0x00000000, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /*       7670    2235587 */
	//RASTERIZER( 0x00592136, 0x00515110, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /*        210    1639140 */
	//RASTERIZER( 0x00582C35, 0x00515110, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /*        108    1154736 */
	//RASTERIZER( 0x00002C35, 0x00515110, 0x00000000, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /*       2152    1150842 */
	//RASTERIZER( 0x00592136, 0x00515110, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /*        152     880560 */
	//RASTERIZER( 0x00008035, 0x00515119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /*      90848     805730 */
	//RASTERIZER( 0x00002C35, 0x00515119, 0x00000000, 0x000B07F9, 0x0C261AC9, 0xFFFFFFFF ) /*       2024     571406 */
	//RASTERIZER( 0x00012136, 0x00515110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /*       1792     494592 */
	//RASTERIZER( 0x00000002, 0x00000000, 0x00000000, 0x00000300, 0xFFFFFFFF, 0xFFFFFFFF ) /*        256     161280 */

	/* blitz99 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *  6297478  149465839 */
	RASTERIZER( 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *   210693    6285480 */
	RASTERIZER( 0x01422C39, 0x00045110, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /* *    20180    2718710 */
	RASTERIZER( 0x00582C35, 0x00515110, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /* *      360    2425416 */
	RASTERIZER( 0x00002C35, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    67059    1480978 */
	RASTERIZER( 0x00008035, 0x00000009, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    24811     400666 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610C9, 0xFFFFFFFF ) /* *    10304     324468 */
	RASTERIZER( 0x00002C35, 0x00515110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     1024     112665 */

	/* blitz2k ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     3880   95344128 */
	RASTERIZER( 0x00582C35, 0x00514110, 0x00000000, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *      148    1785480 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x0C2610CF, 0xFFFFFFFF ) /* *     9976     314244 */

	/* carnevil ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *      492   84128082 */
	RASTERIZER( 0x00002425, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *  1988398   36166780 */
	RASTERIZER( 0x00486116, 0x00045119, 0x00000000, 0x00030279, 0x0C26180F, 0xFFFFFFFF ) /* *    34424   28788847 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *      514   26316800 */
	RASTERIZER( 0x00480015, 0x00045119, 0x00000000, 0x000306F9, 0x0C261AC9, 0xFFFFFFFF ) /* *     7346   18805760 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x000302F9, 0x0C26180F, 0xFFFFFFFF ) /* *   130764   18678972 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x000306F9, 0x0C2618C9, 0xFFFFFFFF ) /* *     7244   12179040 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x000306F9, 0x0C26180F, 0xFFFFFFFF ) /* *    84520   12059721 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000306F9, 0x0C261AC9, 0xFFFFFFFF ) /* *    21926   11226112 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x00030679, 0x0C2618C9, 0xFFFFFFFF ) /* *    92115    8926536 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *     1730    7629334 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x0C26180F, 0xFFFFFFFF ) /* *    37408    5545956 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x00030679, 0x0C26180F, 0xFFFFFFFF ) /* *    26528    4225026 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x000306F9, 0x0C26180F, 0xFFFFFFFF ) /* *    35764    3230884 */
	RASTERIZER( 0x01422409, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *    96020    1226438 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x00030279, 0x0C2618C9, 0xFFFFFFFF ) /* *     1020     574649 */
	RASTERIZER( 0x00482415, 0x00045119, 0x00000000, 0x00030679, 0x0C261A0F, 0xFFFFFFFF ) /* *      360     370008 */
	RASTERIZER( 0x00480015, 0x00045119, 0x00000000, 0x000306F9, 0x0C261A0F, 0xFFFFFFFF ) /* *      576     334404 */

	/* calspeed ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26100F, 0xFFFFFFFF ) /* *    99120 1731923836 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  9955804 1526119944 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C26180F, 0xFFFFFFFF ) /* *  1898207 1124776864 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *  3487467 1101663125 */
	RASTERIZER( 0x01022C19, 0x00000009, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  1079277  609256033 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000A0723, 0x0C261ACF, 0xFFFFFFFF ) /* *    11880  583925760 */
	RASTERIZER( 0x00602819, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *    63644  582469888 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    22688  556797972 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *  1360254  417068457 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  3427489  405421272 */
	RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000B0739, 0x0C26180F, 0xFFFFFFFF ) /* *   286809  238944049 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *    28160  231084818 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *   183564  201014424 */
	RASTERIZER( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *    15275  168207109 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C26100F, 0xFFFFFFFF ) /* *     2856  134400000 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *    98551  110417974 */
	RASTERIZER( 0x01022819, 0x00000009, 0x00000001, 0x000B07F9, 0x0C2610CF, 0xFFFFFFFF ) /* *    47040  107360728 */
	RASTERIZER( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *    13128   86876789 */
	RASTERIZER( 0x01022C19, 0x00000009, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *   257515   76329054 */
	RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *     3934   64958208 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *    77400   63786236 */
	//RASTERIZER( 0x01022C19, 0x00000009, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    12500   63151200 */
	//RASTERIZER( 0x0102001A, 0x00045119, 0x00000001, 0x000A0321, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     8764   57629312 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *     3257   32708448 */
	//RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000A07E3, 0x0C2610CF, 0xFFFFFFFF ) /* *    28364   31195605 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   409001   30699647 */
	//RASTERIZER( 0x00482C35, 0x00045119, 0x00000001, 0x000A0321, 0x0C26100F, 0xFFFFFFFF ) /* *    17669   11214172 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000B0339, 0x0C26180F, 0xFFFFFFFF ) /* *     5844    6064373 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *      626    4651080 */
	//RASTERIZER( 0x00482C35, 0x00045119, 0x00000001, 0x000A0321, 0x0C26180F, 0xFFFFFFFF ) /* *     5887    2945500 */
	//RASTERIZER( 0x00480015, 0x00045119, 0x00000001, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *     1090    2945093 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000001, 0x000B07F9, 0x0C26180F, 0xFFFFFFFF ) /* *      228    1723908 */
	//RASTERIZER( 0x00002C15, 0x00045119, 0x00000001, 0x000A0321, 0x0C261A0F, 0xFFFFFFFF ) /* *      112    1433600 */
	//RASTERIZER( 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *     3091    1165805 */
	//RASTERIZER( 0x01022C19, 0x00000009, 0x00000001, 0x000B07FB, 0x0C26100F, 0xFFFFFFFF ) /* *      620     791202 */

	/* hyprdriv ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *    60860  498565120 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B07F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    28688  235012096 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /* *    11844  156499968 */
	RASTERIZER( 0x00580035, 0x00045119, 0x00000001, 0x00030279, 0x0C261A0F, 0xFFFFFFFF ) /* *   175990  146518715 */
	RASTERIZER( 0x00582C35, 0x00515110, 0x00000001, 0x000B0739, 0x0C261ACF, 0xFFFFFFFF ) /* *     2336  114819072 */
	RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   363325  100404294 */
	RASTERIZER( 0x00582C35, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    40918   96318738 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    54815   94990269 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *   123032   91652828 */
	RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *    82767   86431997 */
	RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *     9874   78101834 */
	RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   102146   72570879 */
	RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *   657804   67229658 */
	RASTERIZER( 0x00580035, 0x00045110, 0x00000001, 0x000B03F9, 0x0C261A0F, 0xFFFFFFFF ) /* *    10428   63173865 */
	RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   230145   57902926 */
	RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   769654   53992486 */
	RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    85365   51865697 */
	RASTERIZER( 0x00582435, 0x00515110, 0x00000001, 0x000B0739, 0x0C261AC9, 0xFFFFFFFF ) /* *   454674   46165536 */
	RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *   101889   33337987 */
	RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *   255952   29810993 */
	//RASTERIZER( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   106190   25430383 */
	//RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *   595001   23268601 */
	//RASTERIZER( 0x0142612A, 0x00000000, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   946410   22589110 */
	//RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *   330036   21323230 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A1F, 0xFFFFFFFF ) /* *    40089   13470498 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    90906   12850855 */
	//RASTERIZER( 0x00582C35, 0x00515110, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *     9492   12115280 */
	//RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *   453515   12013961 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A1F, 0xFFFFFFFF ) /* *    33829    8384312 */
	//RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    83986    7841206 */
	//RASTERIZER( 0x00580035, 0x00045110, 0x00000001, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *    42515    7242660 */
	//RASTERIZER( 0x00582425, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *      706    6158684 */
	//RASTERIZER( 0x00582425, 0x00000000, 0x00000001, 0x000B0739, 0x0C26101F, 0xFFFFFFFF ) /* *    62051    5819485 */
	//RASTERIZER( 0x0142612A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   135139    5063467 */
	//RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    10359    5135837 */
	//RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   170159    4449246 */
	//RASTERIZER( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *    19037    4371219 */
	//RASTERIZER( 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0C26101F, 0xFFFFFFFF ) /* *     8963    4352501 */
	//RASTERIZER( 0x01422C39, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    47712    4159994 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000000, 0x000B073B, 0x0C261ACF, 0xFFFFFFFF ) /* *    47525    4151435 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    34980    3794066 */
	//RASTERIZER( 0x0142613A, 0x00045110, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     6540    2358068 */
	//RASTERIZER( 0x0142611A, 0x00045110, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   703308    2096781 */
	//RASTERIZER( 0x00580035, 0x00045110, 0x00000001, 0x000B0339, 0x0C261A1F, 0xFFFFFFFF ) /* *     3963    2079440 */
	//RASTERIZER( 0x01422439, 0x00000000, 0x00000001, 0x000B073B, 0x0C261AC9, 0xFFFFFFFF ) /* *    22866    2008397 */
	//RASTERIZER( 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    69705    1673671 */
	//RASTERIZER( 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    13366    1575120 */
	//RASTERIZER( 0x0142613A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    50625    1408211 */
	//RASTERIZER( 0x0142613A, 0x00045110, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *  1244348    1244346 */
	//RASTERIZER( 0x00582425, 0x00000000, 0x00000001, 0x000B073B, 0x0C26100F, 0xFFFFFFFF ) /* *    13791    1222735 */
	//RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *    33064     943590 */
	//RASTERIZER( 0x0142610A, 0x00045110, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     2041     926507 */
	//RASTERIZER( 0x00480019, 0x00045110, 0x00000001, 0x000B073B, 0x0C261A0F, 0xFFFFFFFF ) /* *     2722     453924 */
	//RASTERIZER( 0x00580035, 0x00000000, 0x00000001, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *    68232     306869 */
	//RASTERIZER( 0x0142611A, 0x00045110, 0x00000001, 0x000B0379, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     7164     269002 */

	/* mace -------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824100F, 0xFFFFFFFF ) /* *  7204150 1340201579 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0xFFFFFFFF ) /* *    15332 1181663232 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *   104456  652582379 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824180F, 0xFFFFFFFF ) /* *   488613  368880164 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082418CF, 0xFFFFFFFF ) /* *   352924  312417405 */
	RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *    15024  291762384 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082410CF, 0xFFFFFFFF ) /* *   711824  279246170 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824100F, 0xFFFFFFFF ) /* *   735574  171881981 */
	RASTERIZER( 0x00602401, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *   943006  154374023 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082410CF, 0xFFFFFFFF ) /* *   103877  101077498 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824108F, 0xFFFFFFFF ) /* *   710125   87547221 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x08241ACF, 0xFFFFFFFF ) /* *     9834   79774966 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *    17644   70187036 */
	RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *    11324   56633925 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    96743   40820171 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *   166053   29100794 */
	RASTERIZER( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *   166053   29100697 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0379, 0x0824188F, 0xFFFFFFFF ) /* *     6723   29076516 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824188F, 0xFFFFFFFF ) /* *    53297   23928976 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824180F, 0xFFFFFFFF ) /* *    10309   19001776 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    22105   17473157 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824188F, 0xFFFFFFFF ) /* *    11304   17236698 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0xFFFFFFFF ) /* *     1664   17180883 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x08241A0F, 0xFFFFFFFF ) /* *   148606   12274278 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418CF, 0xFFFFFFFF ) /* *    80692    9248007 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000001, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    37819    8080994 */
	//RASTERIZER( 0x00482435, 0x00045117, 0x00000001, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    37819    8080969 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *      536    7930305 */
	//RASTERIZER( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    27601    7905364 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    27601    7905364 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    36314    7667917 */
	//RASTERIZER( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    36314    7667917 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    31109    6020110 */
	//RASTERIZER( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    31109    6020110 */
	//RASTERIZER( 0x00482435, 0x00045117, 0x00000000, 0x000B0339, 0x082418CF, 0xFFFFFFFF ) /* *    42689    5959231 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x082418CF, 0xFFFFFFFF ) /* *    42689    5959231 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824188F, 0xFFFFFFFF ) /* *    11965    5118044 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *    11923    4662909 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x082410CF, 0xFFFFFFFF ) /* *     4422    4624260 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0824100F, 0xFFFFFFFF ) /* *     3853    3596375 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B0379, 0x082418DF, 0xFFFFFFFF ) /* *      400    3555759 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0379, 0x0824180F, 0xFFFFFFFF ) /* *     3755    3453084 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *     4170    2425016 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824184F, 0xFFFFFFFF ) /* *      322    2220073 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x082418CF, 0xFFFFFFFF ) /* *     4008    1201335 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824108F, 0xFFFFFFFF ) /* *    13704     883585 */

	/* sfrush -----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0824101F ) /* *   590139  246714190 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x0824101F ) /* *   397774  153418144 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x082410DF ) /* *    22732  146975666 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0824101F ) /* *   306398  130393278 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0x0824101F ) /* *   437743  117403881 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0x0824101F ) /* *    66608  109289500 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x082410DF ) /* *    19101   92573085 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0824181F ) /* *   258287   78618228 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824181F, 0x0824101F ) /* *    61814   68788856 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x0824181F ) /* *   149792   61464124 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0x0824181F ) /* *   109988   55083276 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x00000000 ) /* *      478   46989312 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x0824181F ) /* *      468   46006272 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0824181F ) /* *   125204   39023396 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x082410DB ) /* *      394   38731776 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x082410DB ) /* *    12890   36333568 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0824101F, 0x0824101F ) /* *   147995   31086325 */
	RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B077B, 0x00000000, 0x082410DB ) /* *     3576   29294592 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824181F, 0x0824181F ) /* *    76059   29282981 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    12632   29173808 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x082418DF ) /* *    14040   24318118 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0824101F, 0x0824101F ) /* *    56586   17643207 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     9130   17277440 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    66302   17049921 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x0824101F ) /* *    64380   16463672 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x0824181F ) /* *      152   14942208 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824101F ) /* *     8748   13810176 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082708DF, 0x0824101F ) /* *   216634   10628656 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B077B, 0x00000000, 0x082410DB ) /* *     1282   10502144 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *    74636    9758030 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x082410DB ) /* *    58652    9353671 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x082410DB ) /* *     5242    8038747 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x082410DB ) /* *    11048    7538060 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0x0824181F ) /* *   121630    6906591 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x082418DF ) /* *    19553    6864245 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x082418DF ) /* *     1287    6648834 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082708DF, 0x0824101F ) /* *   197766    6617876 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082700DF, 0x0824101F ) /* *    75470    6231739 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x08241ADF, 0x0824101F ) /* *      180    5898240 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     7692    5743360 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *    20128    4980591 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     1144    4685824 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082700DF, 0x0824101F ) /* *    72299    4466336 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     3750    4018176 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DF ) /* *     7533    3692141 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824101F ) /* *     9484    3610674 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x0824181F ) /* *   128660    3216280 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x082410DB ) /* *    22214    3172813 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824181F ) /* *     5094    3099098 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F ) /* *     1954    2850924 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824181F ) /* *     1542    2434304 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x00000000 ) /* *      478    1957888 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0x0824181F ) /* *      468    1916928 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x0824101F ) /* *    11664    1729188 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x082410DB ) /* *     1282    1640960 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B077B, 0x082410DB, 0x0824101F ) /* *      388    1589248 */
	//RASTERIZER( 0x00480035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x082410DB ) /* *     1282    1312768 */
	//RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B077B, 0x082410DB, 0x0824181F ) /* *     3928    1046582 */

	/* sfrushrk---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER(0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082410DF, 0x0824101F) /* * 27   992960   15063136 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082708DF, 0x0824101F) /* * 81  1014993    6262343 */
	RASTERIZER(0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0824101F, 0x0824101F) /* *  7   283517    3673219 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824101F) /* * 15   272066    3479808 */
	RASTERIZER(0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241ADF, 0x042210C0) /* * 73    10072    2751593 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082700DF, 0x0824101F) /* * 59   399456    2293575 */
	RASTERIZER(0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x082410DB) /* * 12    94616    1697401 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x0824181F) /* * 83   197678    1694134 */
	RASTERIZER(0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F) /* * 38    47356    1655374 */
	RASTERIZER(0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0x042210C0) /* * 94     7526    1449675 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x082418DF) /* * 89    58657    1178470 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082418DF, 0x0824181F) /* *  4   117539    1114862 */
	RASTERIZER(0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0824101F, 0x042210C0) /* * 52    30451     905250 */

	/* vaportrx ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00482405, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *  2226138  592165102 */
	RASTERIZER( 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    53533  281405105 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x0C261ACF, 0xFFFFFFFF ) /* *   314131  219103141 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *   216329   95014510 */
	RASTERIZER( 0x00482405, 0x00000009, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   317128   92010096 */
	RASTERIZER( 0x0142613A, 0x00045119, 0x00000000, 0x000B07F9, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    13728   88595930 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261ACF, 0xFFFFFFFF ) /* *   649448   81449105 */
	RASTERIZER( 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   444231   60067944 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C26184F, 0xFFFFFFFF ) /* *    36057   58970468 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *    53147   48856709 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x0C2610C9, 0xFFFFFFFF ) /* *   447654   47171792 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A0F, 0xFFFFFFFF ) /* *   207392   38933691 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *  2015632   33364173 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C26100F, 0xFFFFFFFF ) /* *   196361   30395218 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *   110898   28973006 */
	RASTERIZER( 0x00482435, 0x00000009, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *   135107   16301589 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A8F, 0xFFFFFFFF ) /* *    22375   15797748 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x0C26184F, 0xFFFFFFFF ) /* *   141539    7513140 */
	RASTERIZER( 0x0142613A, 0x00045119, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *   621403    5369705 */
	RASTERIZER( 0x00482435, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    30443    4070277 */
	//RASTERIZER( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    22121    3129894 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     9187    1864599 */
	//RASTERIZER( 0x00482405, 0x00044110, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *    10390    1694950 */
	//RASTERIZER( 0x0142610A, 0x00000009, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    25366    1624563 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x0C261A0F, 0xFFFFFFFF ) /* *    69033    1607970 */
	//RASTERIZER( 0x0142610A, 0x00000000, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    36316    1084818 */
	//RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *     1813     816763 */
	//RASTERIZER( 0x0142613A, 0x00045119, 0x00000000, 0x000B0339, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     6602     767221 */
	//RASTERIZER( 0x00482435, 0x00045110, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     2547     646048 */
	//RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261A8F, 0xFFFFFFFF ) /* *     2394     501590 */
	//RASTERIZER( 0x0142613A, 0x00000009, 0x00000000, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) /* *    14078     440086 */
	//RASTERIZER( 0x0142610A, 0x00045119, 0x00000000, 0x000B0339, 0xFFFFFFFF, 0xFFFFFFFF ) /* *     9877     429160 */
	//RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0C261ACF, 0xFFFFFFFF ) /* *     3222     366052 */
	//RASTERIZER( 0x00482435, 0x00000009, 0x00000000, 0x000B0739, 0x0C2610CF, 0xFFFFFFFF ) /* *     5942     285657 */
	//RASTERIZER( 0x00482405, 0x00044119, 0x00000000, 0x000B0339, 0x0C2610CF, 0xFFFFFFFF ) /* *     2328     239688 */
	//RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x000B0739, 0x0C26100F, 0xFFFFFFFF ) /* *     1129     208448 */

	/* wg3dh ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824181F, 0xFFFFFFFF ) /* *   127676  116109477 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824189F, 0xFFFFFFFF ) /* *    96310  112016758 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824109F, 0xFFFFFFFF ) /* *  1412831  108682642 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824101F, 0xFFFFFFFF ) /* *  1612798   45952714 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x08241AD9, 0xFFFFFFFF ) /* *     5960    6103040 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x082418DF, 0xFFFFFFFF ) /* *    56512    4856542 */
	RASTERIZER( 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x0824109F, 0xFFFFFFFF ) /* *     8480    2045940 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0379, 0x0824181F, 0xFFFFFFFF ) /* *     2779    1994317 */
	RASTERIZER( 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0824105F, 0xFFFFFFFF ) /* *   154691    1922774 */
	RASTERIZER( 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x082410DF, 0xFFFFFFFF ) /* *    18114     776139 */

	/* gauntleg ---> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24100F ) /* *   157050  668626339 */
	RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  1079126  580272490 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A4F, 0x0C24100F ) /* *    49686  232178144 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C24104F, 0x0C24100F ) /* *  1048560  206304396 */
	RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C2240CF, 0x0C241ACF ) /* *    59176  182444375 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A4F ) /* *    66342  179689728 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *    72264  109413344 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   281243   75399210 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24104F ) /* *   126384   68412120 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A0F, 0x0C24100F ) /* *    26864   43754988 */
	RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241ACF ) /* *    30510   32759936 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *    44783   31884168 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *    34946   31359362 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241ACF ) /* *     8006   28367999 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *    15430   27908213 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A0F ) /* *    29306   25166802 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    27737   24517949 */
	RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     6783   21292092 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *     9591   17815763 */
	RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   343966   13864759 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *    11842   12126208 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A8F, 0x0C24100F ) /* *     6648    9788508 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C2418CF ) /* *     8444    8646656 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     9677    8365606 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   844920    8289326 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24184F ) /* *     3108    8010176 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x00000000, 0x0C24180F ) /* *     1435    6209238 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     5754    5617499 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *     1608    5557253 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   105127    5133321 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     3460    4689138 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *     7025    4629550 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *     7164    4407683 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24188F ) /* *     1922    3924179 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C24180F ) /* *     4116    3733777 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0C241A8F ) /* *     2626    3732809 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C24180F ) /* *      778    3202973 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C24184F, 0x0C24100F ) /* *     1525    2997446 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C241A0F ) /* *      645    2975266 */
	//RASTERIZER( 0x00600039, 0x00044119, 0x00000000, 0x000B0379, 0x00000000, 0x0C241A0F ) /* *     5212    2491361 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *      825    1996513 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C241A0F ) /* *      466    1967163 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0580000F, 0x0C24180F ) /* *    77400    1883434 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *      472    1698177 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *     2476    1678760 */
	//RASTERIZER( 0x00600C09, 0x00045119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24180F ) /* *     4054    1541748 */
	//RASTERIZER( 0x00600039, 0x00044119, 0x00000000, 0x000B0379, 0x0C241A0F, 0x0C24180F ) /* *     3132    1509438 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0580080F, 0x0C24180F ) /* *     8582    1324196 */
	//RASTERIZER( 0x00602C19, 0x00044119, 0x00000000, 0x000B0379, 0x00000000, 0x0C24100F ) /* *     1436    1239704 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B03F9, 0x0C24180F, 0x0C24100F ) /* *      253    1220316 */
	//RASTERIZER( 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *     2433    1014668 */

	/* gauntdl ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C241ACF ) /* *    30860 1128173568 */
	RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  2631692 1117011118 */
	RASTERIZER( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241ACF ) /* *  2429239  826969012 */
	RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *   454056  468285142 */
	RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C2418CF ) /* *   257586  355634672 */
	RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0379, 0x00000009, 0x0C24180F ) /* *    10898  134362122 */
	RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C241A0F ) /* *    32195  126327049 */
	RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2410CF, 0x0C24100F ) /* *   855240  123899880 */
	RASTERIZER( 0x00602439, 0x00045110, 0x000000C1, 0x000B0379, 0x00000009, 0x0C24180F ) /* *     1718  120629204 */
	RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22488F, 0x0C241ACF ) /* *   186839  120281357 */
	RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0379, 0x0C22480F, 0x0C241ACF ) /* *    14102  115428820 */
	RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C2410CF ) /* *    88530   98271949 */
	RASTERIZER( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0379, 0x0C22480F, 0x0C241ACF ) /* *    12994   68053222 */
	RASTERIZER( 0x00602439, 0x00044110, 0x00000000, 0x000B0379, 0x00000009, 0x0C24100F ) /* *    68273   67454880 */
	RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *   100026   62271618 */
	RASTERIZER( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22480F, 0x0C241ACF ) /* *   153285   44411342 */
	RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   157545   40702131 */
	RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     7800   31948800 */
	RASTERIZER( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22408F, 0x0C241ACF ) /* *    47623   20321183 */
	RASTERIZER( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24188F ) /* *    21570   19324892 */
	//RASTERIZER( 0x00482435, 0x00045110, 0x000000C1, 0x000B0779, 0x0C241ACF, 0x0C24100F ) /* *     3698   15147008 */
	//RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22408F, 0x0C241ACF ) /* *    19765   12383722 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   662274   10563855 */
	//RASTERIZER( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    27909   10462997 */
	//RASTERIZER( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *    78671   10286957 */
	//RASTERIZER( 0x00602439, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *    52038    9928244 */
	//RASTERIZER( 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C224A0F, 0x0C241ACF ) /* *    27469    9239782 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   757116    8072783 */
	//RASTERIZER( 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22488F, 0x0C241ACF ) /* *    18018    7035833 */
	//RASTERIZER( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C241A0F ) /* *    50339    5976564 */
	//RASTERIZER( 0x00603430, 0x00040219, 0x00000000, 0x000B0379, 0x00000009, 0x0C2410CE ) /* *    29385    5466384 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   423347    5355017 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *   162620    4709092 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24100F ) /* *   463705    4642480 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *   280337    4425529 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24180F ) /* *   212646    3432265 */
	//RASTERIZER( 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *     5788    2963456 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *   460800    2609198 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   251108    2392362 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   297219    2352862 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0584180F, 0x0C2410CF ) /* *     9913    2097069 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *   142722    2091569 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     8820    2053325 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *    10346    2033427 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24188F, 0x0C241ACF ) /* *     2136    2017241 */
	//RASTERIZER( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24100F ) /* *     1505    1928490 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   176734    1842440 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24180F ) /* *   262577    1799080 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24180F ) /* *    83179    1534171 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0C24188F ) /* *     3863    1527077 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C24180F ) /* *     8021    1472661 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C241A0F, 0x0C241ACF ) /* *    85416    1342195 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C24100F ) /* *   261360    1335048 */
	//RASTERIZER( 0x00602C19, 0x00000009, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *    74811    1320900 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24100F ) /* *   239331    1268661 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C241ACF ) /* *   107769    1244175 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C241ACF ) /* *     3706    1216182 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x0C24188F ) /* *    49608    1206129 */
	//RASTERIZER( 0x00602C19, 0x00000009, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C241ACF ) /* *    42440    1204109 */
	//RASTERIZER( 0x00482435, 0x00045110, 0x000000C1, 0x000B0779, 0x0C2410CF, 0x0C24100F ) /* *    29584    1168568 */
	//RASTERIZER( 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24180F, 0x0C241ACF ) /* *    17729    1152869 */
	//RASTERIZER( 0x00602C19, 0x00045110, 0x000000C1, 0x000B0379, 0x0C24180F, 0x0C24100F ) /* *     4052    1108726 */
	//RASTERIZER( 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2418CF, 0x0C24100F ) /* *     7082    1079348 */
	//RASTERIZER( 0x00602439, 0x00044119, 0x00000000, 0x000B0379, 0x00000009, 0x0C24180F ) /* *     7761    1023855 */

	/* gradius4 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	//RASTERIZER( 0x02420002,  0x00000009, 0x00000000, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* intro */
	//RASTERIZER( 0x01420021,  0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF )   /* intro */
	//RASTERIZER( 0x00000005,  0x00005119, 0x00000000, 0x00030F7B, 0x14261A87, 0xFFFFFFFF )   /* in-game */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030BFB, 0x14261AC7, 0xFFFFFFFF ) /*   35  1239092  118514052 */
	RASTERIZER( 0x0000303A, 0x00004119, 0x00000000, 0x00030BFB, 0x142610C7, 0xFFFFFFFF ) /* * 72   400122   93801372 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030F7B, 0x14261A87, 0xFFFFFFFF ) /* *  2  1715230   16465427 */
	RASTERIZER( 0x02422E12, 0x00005119, 0x00000000, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF ) /*   81   404825   14369443 */
	RASTERIZER( 0x00582435, 0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /* * 69   505796   13187254 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /*   33   460278   12366856 */
	RASTERIZER( 0x00000015, 0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /*   60   341915    7357317 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030FFB, 0x08241AC7, 0xFFFFFFFF ) /*   70   444582    7071742 */
	RASTERIZER( 0x00580021, 0x00005119, 0x00000000, 0x00030FFB, 0x14261AC7, 0xFFFFFFFF ) /*   51   242000    6018798 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030B7B, 0x14261A07, 0xFFFFFFFF ) /*   28    26700    4497995 */
	RASTERIZER( 0x02420002, 0x00000009, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /*    5  3817984    3777348 */
	RASTERIZER( 0x01424A11, 0x00000009, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /*   31  1140930    3724657 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030BFB, 0x14261A47, 0xFFFFFFFF ) /*   70   165464    3646194 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030BFB, 0x14261A07, 0xFFFFFFFF ) /*   39    25812    3115146 */
	RASTERIZER( 0x00000035, 0x00005119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /* *  6    72291    2961233 */
	RASTERIZER( 0x00000015, 0x00005119, 0x00000000, 0x00030F7B, 0x14261A87, 0xFFFFFFFF ) /*   29    43584    2752299 */
	RASTERIZER( 0x00000005, 0x00001419, 0x00000000, 0x00030B7B, 0x14261A07, 0xFFFFFFFF ) /*   20    15210    2402045 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030B7B, 0x14261AC7, 0xFFFFFFFF ) /*   24    58447    1844641 */
	RASTERIZER( 0x00000005, 0x00005119, 0x00000000, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF ) /*   59   177334    1792616 */
	RASTERIZER( 0x01420021, 0x00000119, 0x00000000, 0x00030F7B, 0x14261AC7, 0xFFFFFFFF ) /*   72    27090    1632226 */

	/* nbapbp ------> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1  */
	//RASTERIZER( 0x00424219,  0x00000000, 0x00000001, 0x00030B7B, 0x08241AC7, 0xFFFFFFFF )   /* intro */
	//RASTERIZER( 0x00002809,  0x00004110, 0x00000001, 0x00030FFB, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
	//RASTERIZER( 0x00424219,  0x00000000, 0x00000001, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
	//RASTERIZER( 0x0200421A,  0x00001510, 0x00000001, 0x00030F7B, 0x08241AC7, 0xFFFFFFFF )   /* in-game */
	/* gtfore06 ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x0C261ACD, 0x0C261ACD ) /*   18  1064626   69362127 */
	RASTERIZER( 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x0C261ACD ) /*   47  3272483   31242799 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x00000ACD, 0x0C261ACD ) /*    9   221917   12348555 */
	RASTERIZER( 0x00002425, 0x00045110, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /*   26    57291    9357989 */
	RASTERIZER( 0x00002429, 0x00000000, 0x000000C1, 0x00010FF9, 0x00000A09, 0x0C261A0F ) /*   12    97156    8530607 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x000000C4, 0x0C261ACD ) /*   55   110144    5265532 */
	RASTERIZER( 0x00002425, 0x00045110, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /*   61    16644    1079382 */
	RASTERIZER( 0x00002425, 0x00045119, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /*    5     8332    1065229 */
	RASTERIZER( 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x0C261A0D ) /*   45     8148     505013 */
	RASTERIZER( 0x00002425, 0x00045119, 0x00000000, 0x00010F79, 0x0C224A0D, 0x0C261A0D ) /*   84    45233     248267 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x0C261ACD, 0x0C2610C4 ) /*   90    10235     193036 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x0C261ACD, 0x0C261ACD ) /* * 29     3777      83777 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x0C261ACD, 0x042210C0 ) /*    2    24952      66761 */
	RASTERIZER( 0x00002429, 0x00000000, 0x00000000, 0x00010FF9, 0x00000A09, 0x0C261A0F ) /*   24      661      50222 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x0C261ACD, 0x04221AC9 ) /*   92    12504      43720 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x0C261ACD, 0x0C2610C4 ) /*   79     2160      43650 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x000000C4, 0x04221AC9 ) /*   19     2796      30377 */
	RASTERIZER( 0x00002425, 0x00045119, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /*   67     1962      14755 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x000000C4, 0x0C261ACD ) /* * 66       74       3951 */
	RASTERIZER( 0x00482405, 0x00045119, 0x00000000, 0x00010FF9, 0x00000ACD, 0x04221AC9 ) /*   70      374       3691 */
	RASTERIZER( 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x0C261ACD ) /* * 20      350       7928 */

	/* virtpool ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A0F, 0x042210C0 ) /* * 78  2182388   74854175 */
	RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B07F9, 0x0C261A0F, 0x042210C0 ) /* * 46   114830    6776826 */
	RASTERIZER( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A0F, 0x042210C0 ) /* * 58  1273673    4513463 */
	RASTERIZER( 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x0C261A09, 0x042210C0 ) /* * 46   634995    2275612 */
	RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A0F, 0x042210C0 ) /* * 46    26651    1883507 */
	RASTERIZER( 0x00482405, 0x00045110, 0x00000000, 0x000B073B, 0x0C261A0F, 0x042210C0 ) /* * 26   220644     751241 */
	//RASTERIZER( 0x00002421, 0x00445110, 0x00000000, 0x000B073B, 0x0C261A09, 0x042210C0 ) /* * 79    14846    3499120 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C261A09, 0x042210C0 ) /* * 66    26665    1583363 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C26100F, 0x042210C0 ) /* * 78    33096     957935 */
	//RASTERIZER( 0x00002425, 0x00445110, 0x00000000, 0x000B07F9, 0x0C261A0F, 0x042210C0 ) /* * 38    12494     678029 */
	//RASTERIZER( 0x00800000, 0x00000000, 0x00000000, 0x00000200, 0x00000000, 0x00000000 ) /* * 28    25348     316181 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x0C26100F, 0x042210C0 ) /* * 13    11344     267903 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B073B, 0x0C261A09, 0x042210C0 ) /* * 34     1548     112168 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B07FB, 0x0C26100F, 0x042210C0 ) /* * 35      664      25222 */
	//RASTERIZER( 0x00000002, 0x00000000, 0x00000000, 0x00000300, 0xFFFFFFFF, 0xFFFFFFFF ) /* * 33      512      18393 */
	//RASTERIZER( 0x00002421, 0x00000000, 0x00000000, 0x000B07FB, 0x0C261A0F, 0x042210C0 ) /* * 14      216      16842 */
	//RASTERIZER( 0x00000001, 0x00000000, 0x00000000, 0x00000300, 0x00000800, 0x00000800 ) /* * 87        2         72 */
	//RASTERIZER( 0x00000001, 0x00000000, 0x00000000, 0x00000200, 0x08241A00, 0x08241A00 ) /* * 92        2          8 */
	//RASTERIZER( 0x00000001, 0x00000000, 0x00000000, 0x00000200, 0x00000000, 0x08241A00 ) /* * 93        2          8 */

	/* roadburn ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24100F) /* * 88  6599666   64554420 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A0F, 0x0C2418CF) /* * 12   763617   35323533 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C2418CF) /* * 44  1930013   33746169 */
	RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0C2418CF) /* * 21  1439267   29935941 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0x0C24100F) /* * 32  6915356   28830506 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0C24180F) /* * 39  4057572   17696631 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0C24100F) /* * 50  4955570   14335742 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0C2418CF) /* * 41   766085    9520801 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C2418CF) /* *  0   534929    7695839 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C24100F) /* *  9  1078501    7419628 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24180F) /* * 77   413387    7228312 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C24180F) /* * 95   353176    7192165 */
	RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0x0C2418CF) /* * 52   315430    5154802 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0x0C24100F) /* * 54  1704858    5008909 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24100F) /* * 13   899639    4953916 */
	RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C24188F) /* * 64   277509    4254016 */
	//RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0000088F, 0x0C2418CF) /* * 87   295785    4066338 */
	//RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C241ACF) /* *  7    28140    4056321 */
	//RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C241ACF, 0x0C2618CF) /* *  9     9494    3608108 */
	//RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0C2418CF, 0x0C2418CF) /* * 77   647531    3223654 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C2410CF, 0x0C24100F) /* * 84   142873    3131813 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C24180F) /* *  2   247541    2880853 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24100F, 0x0C24188F) /* * 11   143650    2430648 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0x0C24180F) /* * 43   686600    2266187 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000088F, 0x0C24180F) /* *  8   526279    1750008 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24188F, 0x0C24100F) /* * 75   158668    1533323 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C24180F, 0x0C2418CF) /* * 66   105179    1324448 */
	//RASTERIZER(0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0C241A0F, 0x0C24188F) /* * 76    19479    1150035 */

	/* cartfury --> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER(0x00000035, 0x00045119, 0x000000C1, 0x00030F39, 0x0C261A0F, 0x042210C0) /*   55  6897497   95045895 */
	RASTERIZER(0x00420039, 0x00000000, 0x000000C1, 0x00030F39, 0x0C26100F, 0x042210C0) /*   92  9680462   62502113 */
	RASTERIZER(0x0142A409, 0x00000000, 0x000000C1, 0x00030F3B, 0x0C261ACF, 0x042210C0) /*   51  3884086   40581793 */
	RASTERIZER(0x00000035, 0x00045119, 0x00000000, 0x00030F39, 0x0C261A0F, 0x042210C0) /* * 94  2263184   30556572 */
	RASTERIZER(0x00422439, 0x00000000, 0x000000C1, 0x00030F3B, 0x0C261A0F, 0x042210C0) /*   93  2520077   30036037 */
	RASTERIZER(0x00582435, 0x00045110, 0x00000000, 0x00030BF9, 0x0C2610C9, 0x042210C0) /*   36  2053030   28006572 */
	RASTERIZER(0x0142A409, 0x00000000, 0x00000000, 0x00030B39, 0x0C261A0F, 0x042210C0) /*   92  4894430   27815796 */
	RASTERIZER(0x00580035, 0x00045119, 0x000000C1, 0x00030B39, 0x0C261A0F, 0x042210C0) /*   69  1813831   25845118 */
	RASTERIZER(0x0142A409, 0x00000000, 0x00000000, 0x00030F3B, 0x0C261ACF, 0x042210C0) /*   47  1906963   20937897 */
	RASTERIZER(0x00420039, 0x00000000, 0x00000000, 0x00030FF9, 0x0C261A0F, 0x042210C0) /*   40   152832   19687732 */
	RASTERIZER(0x00420039, 0x00000000, 0x00000000, 0x00030F3B, 0x0C261A0F, 0x042210C0) /*   40  2896061   19553336 */
	RASTERIZER(0x00420039, 0x00000000, 0x000000C1, 0x00030F39, 0x0C261A0F, 0x042210C0) /*   60  1626437   16446065 */
	RASTERIZER(0x00000035, 0x00045110, 0x000000C1, 0x00030BF9, 0x0C261A0F, 0x042210C0) /*   82   156240   16358632 */
	RASTERIZER(0x00420039, 0x00000000, 0x00000000, 0x00030F3B, 0x0C261ACF, 0x042210C0) /*   36  3538654   15204923 */
	RASTERIZER(0x00420039, 0x00000000, 0x00000000, 0x00030F39, 0x0C26100F, 0x042210C0) /* *  7  1899232   14314750 */
	RASTERIZER(0x0142A409, 0x00000000, 0x000000C1, 0x00030F3B, 0x0C261A0F, 0x042210C0) /*   82  1097851   13461074 */
	//RASTERIZER(0x00422439, 0x00000000, 0x00000000, 0x00030F3B, 0x0C261A0F, 0x042210C0) /*    8  1024956   13120753 */
	//RASTERIZER(0x00420039, 0x00000000, 0x00000000, 0x00030F39, 0x0C261A0F, 0x042210C0) /*   72  1641456   11253842 */
	//RASTERIZER(0x0142A409, 0x00000000, 0x00000000, 0x00030F3B, 0x0C261A0F, 0x042210C0) /*   51   742740   10329945 */
	//RASTERIZER(0x00420039, 0x00000000, 0x000000C1, 0x00030F3B, 0x0C261A0F, 0x042210C0) /* * 28  1167364    8450582 */
	//RASTERIZER(0x0042613A, 0x00000000, 0x000000C1, 0x00030FF9, 0xFFFFFFFF, 0xFFFFFFFF) /*    6    29298    7216480 */

	/* warfa  ----> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER(0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241ACF) /*    5  6988136  185178764 */
	RASTERIZER(0x00602419, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x0C241A0F) /*   92  2423839   61801379 */
	RASTERIZER(0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0C24100F, 0x00000000) /*   74  5178601   36194132 */
	RASTERIZER(0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000000, 0x0C24180F) /*   57  1267243   26421492 */
	RASTERIZER(0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x104008CF, 0x0C2618CF) /*   13  1461060   22971624 */
	RASTERIZER(0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x10400ACF, 0x0C2618CF) /*   63  1089946   21852830 */
	RASTERIZER(0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22480F, 0x0C241ACF) /*   27   252592   20660816 */
	RASTERIZER(0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x104000CF, 0x0C2618CF) /*   84   796384   20070179 */
	RASTERIZER(0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000000, 0x0C24100F) /*   68  4256201   19630570 */
	RASTERIZER(0x00602439, 0x00044119, 0x000000C1, 0x000B0779, 0x0582480F, 0x0C26180F) /* * 69   137540   18243142 */
	RASTERIZER(0x00602C09, 0x00045119, 0x000000C1, 0x000B0779, 0x0482400F, 0x0C261ACF) /* * 16   377796   16889915 */

	/* sf2049se --> fbzColorPath alphaMode   fogMode,    fbzMode,    texMode0,   texMode1        hash */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C26100F, 0x0000000F) /* * 44  6143319  107636749 */
	RASTERIZER(0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x0C26180F, 0x0000080F) /* * 89  5079264   72208948 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C26100F, 0x0000080F) /* * 33  2115327   46427046 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0000000F, 0x0C26100F) /* * 46  1574119   21123488 */
	RASTERIZER(0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x0C2610CF, 0x0000080F) /* * 45  1681920   19266314 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0A452A0F, 0x0E47200F) /* * 85  2624894   12845924 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C26180F, 0x0000000F) /* * 22   312805    8560298 */
	RASTERIZER(0x00482435, 0x00045117, 0x000000C1, 0x000B0339, 0x0C26100F, 0x0000000F) /* * 69   567798    7255338 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C2618CF, 0x0000000F) /* * 88   201277    7213832 */
	RASTERIZER(0x00602401, 0x00045119, 0x000000C1, 0x00030279, 0x0C2618CF, 0x0000080F) /* * 25  1137620    7146799 */
	RASTERIZER(0x00482435, 0x00045119, 0x000000C1, 0x000B0339, 0x0C26100F, 0x0000000F) /* * 52   567798    7137832 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C261A0F, 0x0000080F) /* * 54   141585    6669637 */
	RASTERIZER(0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x0C2618CF, 0x0000080F) /* * 23   357022    6573647 */
	RASTERIZER(0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x0C26100F, 0x0000080F) /* * 14   462528    5358791 */
	RASTERIZER(0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0C26188F, 0x0000000F) /* * 57    98945    5300544 */
	RASTERIZER(0x00482405, 0x00045119, 0x00000000, 0x000B0379, 0x0C26180F, 0x0000080F) /* * 77   232584    4197249 */
	{ &voodoo_renderer::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, 0xffffffff, 0xffffffff>, { 0xffffffff } }
};

}
