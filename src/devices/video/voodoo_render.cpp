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

inline s32 ATTR_FORCE_INLINE compute_wfloat(s64 iterw)
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

inline rgbaint_t ATTR_FORCE_INLINE clamped_argb(const rgbaint_t &iterargb, reg_fbz_colorpath const fbzcp)
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

inline s32 ATTR_FORCE_INLINE clamped_z(s32 iterz, reg_fbz_colorpath const fbzcp)
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

inline s32 ATTR_FORCE_INLINE clamped_w(s64 iterw, reg_fbz_colorpath const fbzcp)
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


//-------------------------------------------------
//  fast_log2 - computes the log2 of a double-
//  precision value as a 24.8 value; if the double
//  was converted from a fixed-point integer, the
//  number of fractional bits should be specified
//  by fracbits
//-------------------------------------------------

inline s32 ATTR_FORCE_INLINE fast_log2(double value, int fracbits = 0)
{
	// negative values return 0
	if (UNEXPECTED(value < 0))
		return 0;

	// convert the value to a raw integer
	union { double d; u64 i; } temp;
	temp.d = value;

	// we only care about the 11-bit exponent and top 7 bits of mantissa
	// (sign is already assured to be 0)
	u32 ival = temp.i >> 45;

	// extract exponent, unbias, and adjust for fixed-point fraction
	s32 exp = (ival >> 7) - 1023 - fracbits;

	// use top 7 bits of mantissa to look up fractional log
	static u8 const s_log2_table[128] =
	{
		  0,   2,   5,   8,  11,  14,  16,  19,  22,  25,  27,  30,  33,  35,  38,  40,
		 43,  46,  48,  51,  53,  56,  58,  61,  63,  65,  68,  70,  73,  75,  77,  80,
		 82,  84,  87,  89,  91,  93,  96,  98, 100, 102, 104, 106, 109, 111, 113, 115,
		117, 119, 121, 123, 125, 127, 129, 132, 134, 136, 138, 140, 141, 143, 145, 147,
		149, 151, 153, 155, 157, 159, 161, 162, 164, 166, 168, 170, 172, 173, 175, 177,
		179, 181, 182, 184, 186, 188, 189, 191, 193, 194, 196, 198, 200, 201, 203, 205,
		206, 208, 209, 211, 213, 214, 216, 218, 219, 221, 222, 224, 225, 227, 229, 230,
		232, 233, 235, 236, 238, 239, 241, 242, 244, 245, 247, 248, 250, 251, 253, 254
	};

	// combine the integral and fractional parts
	return (exp << 8) | s_log2_table[ival & 127];
}



//-------------------------------------------------
//  compute_lodbase - compute the base LOD value
//-------------------------------------------------

inline s32 ATTR_FORCE_INLINE compute_lodbase(s64 dsdx, s64 dsdy, s64 dtdx, s64 dtdy)
{
	// compute (ds^2 + dt^2) in both X and Y; note that these values are
	// each .32, so the square is a .64 fixed point value
	double fdsdx = double(dsdx);
	double fdsdy = double(dsdy);
	double fdtdx = double(dtdx);
	double fdtdy = double(dtdy);
	double texdx = fdsdx * fdsdx + fdtdx * fdtdx;
	double texdy = fdsdy * fdsdy + fdtdy * fdtdy;

	// pick whichever is larger
	double maxval = std::max(texdx, texdy);

	// use our fast reciprocal/log on this value; 64 to indicate how many
	// bits of fractional resolution in the source, and divide by 2 because
	// we really want the log of the square root
	return fast_log2(maxval, 64) / 2;
}



//**************************************************************************
//  RASTERIZER PARAMS
//**************************************************************************

//-------------------------------------------------
//  operator== - compare all values
//-------------------------------------------------

bool rasterizer_params::operator==(rasterizer_params const &rhs) const
{
	return (m_generic == rhs.m_generic &&
			m_fbzcp == rhs.m_fbzcp &&
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
	m_generic = 0;
	m_fbzcp = regs.fbz_colorpath().normalize();
	m_fbzmode = regs.fbz_mode().normalize();
	m_alphamode = regs.alpha_mode().normalize();
	m_fogmode = regs.fog_mode().normalize();
	m_texmode0 = reg_texture_mode::NONE;
	m_texmode1 = reg_texture_mode::NONE;
	if (!regs.fbi_init3().disable_tmus())
	{
		if (tmu0regs != nullptr && tmu0regs->texture_lod().lod_min() < 32)
		{
			m_texmode0 = tmu0regs->texture_mode().normalize();
			m_generic |= GENERIC_TEX0;
			if (tmu0regs->trexinit_send_tmu_config())
				m_texmode0 |= reg_texture_mode::TMU_CONFIG_MASK;
		}
		if (tmu1regs != nullptr && tmu1regs->texture_lod().lod_min() < 32)
		{
			m_texmode1 = tmu1regs->texture_mode().normalize();
			m_generic |= GENERIC_TEX1;
		}
	}
	compute_equations();
}


//-------------------------------------------------
//  compute_equations - compute equations based
//  on the captured state
//-------------------------------------------------

void rasterizer_params::compute_equations()
{
	// start with the color equation
	m_color_equation = color_equation::from_fbzcp(m_fbzcp);

	// if it doesn't consume any texels, zap the textures; we're done
	if (!m_color_equation.uses_any(color_source::texel0))
	{
		m_texmode0 = m_texmode1 = reg_texture_mode::NONE;
		m_generic = 0;
		return;
	}

	// check the TMU1 equation
	if (m_generic & GENERIC_TEX1)
	{
		m_tex1_equation = color_equation::from_texmode(m_texmode1, color_source::texel1, color_source::zero);

		// if it's identity, mark it and clear out the equation mask
		if (m_tex1_equation.is_identity(color_source::texel1))
		{
			m_generic |= GENERIC_TEX1_IDENTITY;
			m_texmode1 &= ~reg_texture_mode::EQUATION_MASK;
		}
	}

	// check the TMU0 equation
	if (m_generic & GENERIC_TEX0)
	{
		m_tex0_equation = color_equation::from_texmode(m_texmode0, color_source::texel0, (m_generic & GENERIC_TEX1) ? color_source::texel1 : color_source::zero);

		// special case the send configuration
		if (!(m_texmode0 & reg_texture_mode::TMU_CONFIG_MASK))
		{
			if (m_tex0_equation.is_identity(color_source::texel0))
			{
				// if identity returning ourself, the downstream TMU doesn't matter
				m_generic = (m_generic & ~(GENERIC_TEX1 | GENERIC_TEX1_IDENTITY)) | GENERIC_TEX0_IDENTITY;
				m_texmode0 &= ~reg_texture_mode::EQUATION_MASK;
				m_texmode1 = reg_texture_mode::NONE;
			}
			else if (m_tex0_equation.is_identity(color_source::texel1))
			{
				// if identity returning the downstream value, we're just a passthrough and we don't matter
				m_generic &= ~GENERIC_TEX0;
				m_texmode0 = reg_texture_mode::NONE;
			}
		}
	}
}


//-------------------------------------------------
//  hash - return a hash of the current values
//-------------------------------------------------

u32 rasterizer_params::hash() const
{
	return m_generic ^
			rotate(m_alphamode, 0) ^
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
//  COLOR SOURCE
//**************************************************************************

//-------------------------------------------------
//  simplify - simplify the individual values
//-------------------------------------------------

void color_source::simplify()
{
	// convert inverted zero to one
	if (m_rgb == (ZERO | FLAG_INVERTED))
		m_rgb = ONE;
	if (m_alpha == (ZERO | FLAG_INVERTED))
		m_alpha = ONE;
}


//-------------------------------------------------
//  string - return a string version of the
//  components
//-------------------------------------------------

std::string color_source::as_string() const
{
	static char const * const s_names[] =
	{
		"zero",
		"one",
		"color0",
		"color1",
		"iterargb",
		"clampz",
		"clipw",
		"",
		"texel0",
		"texel1",
		"detailfactor",
		"lodfrac",
		"color/iterargb via texel0 alpha"
	};
	static char const * const s_flags[] =
	{
		"",
		".alphaexpanded",
		".inverted",
		".alphaexpanded.inverted",
	};
	std::string result;

	// determine global tags
	char const *tag = "";
	if (is_constant())
		tag = "[const]";
	else if (is_constant_or_iterated())
		tag = "[iter]";

	// simplest case: RGB and alpha are the same and non-ambiguous
	if (m_alpha == m_rgb || (is_uniform_alpha() && (alpha_base() == DETAIL_FACTOR || alpha_base() == LOD_FRACTION)))
		return string_format("{ %s%s }%s", s_names[alpha_base()], s_flags[alpha_flags()], tag);

	// slightly less simple: RGB and alpha are both smeared alpha values
	if (is_uniform_alpha())
		return string_format("{ %s.alpha%s }%s", s_names[alpha_base()], s_flags[alpha_flags()], tag);

	// full complexity case
	char const *tag_alpha = "";
	char const *tag_rgb = "";
	if (tag[0] == 0)
	{
		if (is_alpha_constant())
			tag_alpha = "[const]";
		else if (is_alpha_constant_or_iterated())
			tag_alpha = "[iter]";
		if (is_rgb_constant())
			tag_rgb = "[const]";
		else if (is_rgb_constant_or_iterated())
			tag_rgb = "[iter]";
	}
	return string_format("{ %s%s%s, %s%s%s }%s", s_names[alpha_base()], s_flags[alpha_flags()], tag_alpha, s_names[rgb_base()], s_flags[rgb_flags()], tag_rgb, tag);
}

// constant values
color_source const color_source::zero(color_source::ZERO, color_source::ZERO);
color_source const color_source::one(color_source::ONE, color_source::ONE);
color_source const color_source::iterated_argb(color_source::ITERATED_ARGB, color_source::ITERATED_ARGB);
color_source const color_source::color0(color_source::COLOR0, color_source::COLOR0);
color_source const color_source::color1(color_source::COLOR1, color_source::COLOR1);
color_source const color_source::texel0(color_source::TEXEL0, color_source::TEXEL0);
color_source const color_source::texel1(color_source::TEXEL1, color_source::TEXEL1);


//**************************************************************************
//  COLOR EQUATION
//**************************************************************************

//-------------------------------------------------
//  simplify - simplify an equation for optimal
//  SIMD operations
//-------------------------------------------------

void color_equation::simplify()
{
	// simplify each internally
	m_color.simplify();
	m_subtract.simplify();
	m_multiply.simplify();
	m_add.simplify();

	// if subtract is zero, we can maybe swap things between color and multiply to
	// make things neater
	if (m_subtract.is_zero())
	{
		bool swap = false;

		// if both are split types and swapping will line one up, do it for sure
		if (!m_color.is_uniform() && !m_multiply.is_uniform())
			if (m_multiply.alpha_base() == m_color.rgb_base() || m_multiply.rgb_base() == m_color.alpha_base())
				swap = true;

		// if neither is a constant/iterated and swapping will make one of them into
		// a constant/iterated, do it
		if (!m_color.is_constant_or_iterated() && !m_multiply.is_constant_or_iterated())
			if (m_color.is_rgb_constant_or_iterated() != m_color.is_alpha_constant_or_iterated() && m_multiply.is_rgb_constant_or_iterated() != m_multiply.is_alpha_constant_or_iterated())
				swap = true;

		// do the swap
		if (swap)
		{
			auto temp = m_color.rgb();
			m_color.set_rgb(m_multiply.rgb());
			m_multiply.set_rgb(temp);
		}

		// always prefer the multiply to be constant
		if (m_color.is_constant_or_iterated() && !m_multiply.is_constant_or_iterated())
		{
			color_source temp = m_color;
			m_color = m_multiply;
			m_multiply = temp;
		}

		// if we're just combining rgb/alpha with constant 0/1, consolidate
		if (m_multiply.alpha() == color_source::ONE && m_multiply.is_rgb_zero() && m_add.is_alpha_zero())
		{
			m_add.set_alpha(m_color.alpha());
			m_multiply = color_source::zero;
		}
		if (m_multiply.is_alpha_zero() && m_multiply.is_rgb_one() && m_add.is_rgb_zero())
		{
			m_add.set_rgb(m_color.rgb());
			m_multiply = color_source::zero;
		}

		if (!m_add.is_zero() && !m_multiply.is_zero())
		{
			// if the multiply is RGB and the add is alpha, combine
			if (m_multiply.is_alpha_zero() && m_add.is_rgb_zero())
			{
				if (m_multiply.rgb_base() == m_add.alpha_base())
				{
					m_multiply.set_alpha(m_add.alpha());
					m_add.set_alpha(color_source::ZERO);
					m_color.set_alpha(color_source::ONE);
				}
				else
				{
					m_color.set_alpha(m_add.alpha());
					m_add.set_alpha(color_source::ZERO);
					m_multiply.set_alpha(color_source::ONE);
				}
			}

			// if the multiply is alpha and the add is RGB, combine
			if (m_multiply.is_rgb_zero() && m_add.is_alpha_zero())
			{
				if (m_multiply.alpha() == m_add.rgb())
				{
					m_multiply.set_rgb(m_add.rgb());
					m_add.set_rgb(color_source::ZERO);
					m_color.set_rgb(color_source::ONE);
				}
				else
				{
					m_color.set_rgb(m_add.rgb());
					m_add.set_rgb(color_source::ZERO);
					m_multiply.set_rgb(color_source::ONE);
				}
			}
		}
	}

	// if the alpha multiply is zero, it doesn't matter what the color/subtract alpha is
	if (m_multiply.is_alpha_zero())
	{
		m_color.set_alpha(m_color.rgb());
		m_subtract.set_alpha(m_subtract.rgb());
	}
	if (m_multiply.is_rgb_zero())
	{
		m_color.set_rgb(m_color.alpha());
		m_subtract.set_rgb(m_subtract.alpha());
	}

	// if color is zero and subtract is zero, that means the multiply is irrelevant
	if (m_color.is_zero() && m_subtract.is_zero())
		m_multiply = color_source::zero;
}


//-------------------------------------------------
//  as_string - return a string version of the
//  equation
//-------------------------------------------------

std::string color_equation::as_string() const
{
	// attempt to simplify
	// blend factor of zero ignores earlier stuff
	if (m_multiply.is_zero())
		return m_add.as_string();

	// blend multiply of one is an add
	if (m_multiply.is_one())
	{
		std::string result = m_color.as_string();
		if (!m_subtract.is_zero())
		{
			result += " - ";
			result += m_subtract.as_string();
		}
		if (!m_add.is_zero())
		{
			result += " + ";
			result += m_add.as_string();
		}
		return result;
	}

	// otherwise a proper blend
	std::string result;
	if (!m_subtract.is_zero())
		result += "(";
	result += m_color.as_string();
	if (!m_subtract.is_zero())
	{
		result += " - ";
		result += m_subtract.as_string();
		result += ")";
	}
	result += " * ";
	result += m_multiply.as_string();
	if (!m_add.is_zero())
	{
		result += " + ";
		result += m_add.as_string();
	}
	return result;
}


//-------------------------------------------------
//  from_fbzcp - compute the color equation based
//  on the fbs colorpath
//-------------------------------------------------

color_equation color_equation::from_fbzcp(reg_fbz_colorpath const fbzcp)
{
	// determine other color
	color_source other_color;
	switch (fbzcp.cc_rgbselect())
	{
		case 0: other_color.set_rgb(color_source::iterated_argb.rgb()); break;
		case 1: other_color.set_rgb(color_source::texel0.rgb()); break;
		case 2: other_color.set_rgb(color_source::color1.rgb()); break;
	}
	switch (fbzcp.cc_aselect())
	{
		case 0: other_color.set_alpha(color_source::iterated_argb.alpha()); break;
		case 1: other_color.set_alpha(color_source::texel0.alpha()); break;
		case 2: other_color.set_alpha(color_source::color1.alpha()); break;
	}

	// determine local color
	color_source local_color;
	if (fbzcp.cc_localselect_override() == 0)
		local_color.set_rgb((fbzcp.cc_localselect() == 0) ? color_source::iterated_argb.rgb() : color_source::color0.rgb());
	else
		local_color.set_rgb(color_source::COLOR0_OR_ITERATED_VIA_TEXEL_ALPHA);
	switch (fbzcp.cca_localselect())
	{
		case 0: local_color.set_alpha(color_source::iterated_argb.alpha()); break;
		case 1: local_color.set_alpha(color_source::color0.alpha()); break;
		case 2: local_color.set_alpha(color_source::CLAMPZ); break;
		case 3: local_color.set_alpha(color_source::CLAMPW); break;
	}

	// select blend color
	color_equation equation;
	if (!fbzcp.cc_zero_other())
		equation.color().set_rgb(other_color.rgb());
	if (!fbzcp.cca_zero_other())
		equation.color().set_alpha(other_color.alpha());

	// determine subtraction color
	if (fbzcp.cc_sub_clocal())
		equation.subtract().set_rgb(local_color.rgb());
	if (fbzcp.cca_sub_clocal())
		equation.subtract().set_alpha(local_color.alpha());

	// determine blend factor
	switch (fbzcp.cc_mselect())
	{
		case 1: equation.multiply().set_rgb(local_color.rgb()); break;
		case 2: equation.multiply().set_rgb_from_alpha(other_color.alpha()); break;
		case 3: equation.multiply().set_rgb_from_alpha(local_color.alpha()); break;
		case 4: equation.multiply().set_rgb_from_alpha(color_source::texel0.alpha()); break;
		case 5: /*if (type >= VOODOO_2)*/ equation.multiply().set_rgb(color_source::texel0.rgb()); break;
	}
	switch (fbzcp.cca_mselect())
	{
		case 1:
		case 3: equation.multiply().set_alpha(local_color.alpha()); break;
		case 2: equation.multiply().set_alpha(other_color.alpha()); break;
		case 4: equation.multiply().set_alpha(color_source::texel0.alpha()); break;
	}
	if (!fbzcp.cc_reverse_blend()) equation.multiply().invert_rgb();
	if (!fbzcp.cca_reverse_blend()) equation.multiply().invert_alpha();

	// determine add color
	switch (fbzcp.cc_add_aclocal())
	{
		case 1: equation.add().set_rgb(local_color.rgb()); break;
		case 2: equation.add().set_rgb_from_alpha(local_color.alpha()); break;
	}
	if (fbzcp.cca_add_aclocal())
		equation.add().set_alpha(local_color.alpha());

	equation.simplify();
	return equation;
}


//-------------------------------------------------
//  from_texmode - compute the color equation based
//  on the fbs colorpath
//-------------------------------------------------

color_equation color_equation::from_texmode(reg_texture_mode const texmode, color_source texel_color, color_source input_color)
{
	color_source other_color = input_color;
	color_source local_color = texel_color;
	color_equation equation;

	if (!texmode.tc_zero_other())
		equation.color().set_rgb(other_color.rgb());
	if (!texmode.tca_zero_other())
		equation.color().set_alpha(other_color.alpha());

	if (texmode.tc_sub_clocal())
		equation.subtract().set_rgb(local_color.rgb());
	if (texmode.tca_sub_clocal())
		equation.subtract().set_alpha(local_color.alpha());

	switch (texmode.tc_mselect())
	{
		case 1: equation.multiply().set_rgb(local_color.rgb()); break;
		case 2: equation.multiply().set_rgb_from_alpha(other_color.alpha()); break;
		case 3: equation.multiply().set_rgb_from_alpha(local_color.alpha()); break;
		case 4: equation.multiply().set_rgb_from_alpha(color_source::DETAIL_FACTOR); break;
		case 5: equation.multiply().set_rgb_from_alpha(color_source::LOD_FRACTION); break;
	}
	switch (texmode.tca_mselect())
	{
		case 1:
		case 3: equation.multiply().set_alpha(local_color.alpha()); break;
		case 2: equation.multiply().set_alpha(other_color.alpha()); break;
		case 4: equation.multiply().set_alpha(color_source::DETAIL_FACTOR); break;
		case 5: equation.multiply().set_alpha(color_source::LOD_FRACTION); break;
	}
	if (!texmode.tc_reverse_blend()) equation.multiply().invert_rgb();
	if (!texmode.tca_reverse_blend()) equation.multiply().invert_alpha();

	switch (texmode.tc_add_aclocal())
	{
		case 1: equation.add().set_rgb(local_color.rgb()); break;
		case 2: equation.add().set_rgb_from_alpha(local_color.alpha()); break;
	}
	if (texmode.tca_add_aclocal())
		equation.add().set_alpha(local_color.alpha());

	equation.simplify();
	return equation;
}



//**************************************************************************
//  RASTER TEXTURE
//**************************************************************************

//-------------------------------------------------
//  recompute - recompute state based on parameters
//-------------------------------------------------

void rasterizer_texture::recompute(voodoo_regs const &regs, u8 *ram, u32 mask, rgb_t const *lookup, u32 addrmask, u8 addrshift)
{
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
		fatalerror("Unsupported tiled texture in Voodoo device");
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

inline rgbaint_t ATTR_FORCE_INLINE rasterizer_texture::fetch_texel(reg_texture_mode const texmode, dither_helper const &dither, s32 x, double iters, double itert, double iterw, s32 &lod, u8 bilinear_mask)
{
	// determine the S/T/LOD values for this texture; iterated S/T are
	// in 32.32 format and we want final S/T in 24.8 format
	s32 s, t;
	if (texmode.enable_perspective())
	{
		// iterws is also 32.32, so division would leave no fractional bits;
		// the 256 factor is there to produce 8 bits of fraction
		double recip = 256.0 / iterw;
		s = s32(iters * recip);
		t = s32(itert * recip);

		// compute the log2 of the non-reciprocal W value; negating it gives
		// the log2 of the reciprocal, so we subtract instead of add it
		lod -= fast_log2(iterw, 32);
	}
	else
	{
		// scale the .32 values down to .8 values as doubles to avoid 64-bit
		// integers
		s = s32(iters * (1.0 / (1 << 24)));
		t = s32(itert * (1.0 / (1 << 24)));
	}

	// clamp S/T if the iterated W is negative
	if (texmode.clamp_neg_w() && iterw < 0)
		s = t = 0;

	// clamp the LOD after applying bias and dither
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
		// incorporate the fraction shift into ilod
		ilod += 8;
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
		u32 sfrac = s & bilinear_mask;
		u32 tfrac = t & bilinear_mask;

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

voodoo_renderer::voodoo_renderer(running_machine &machine, u16 tmu_config, const rgb_t *rgb565, voodoo_regs &fbi_regs, voodoo_regs *tmu0_regs, voodoo_regs *tmu1_regs) :
	poly_manager(machine),
	m_bilinear_mask(0xf0),
	m_tmu_config(tmu_config),
	m_rowpixels(0),
	m_yorigin(0),
	m_fbi_reg(fbi_regs),
	m_tmu0_reg(tmu0_regs),
	m_tmu1_reg(tmu1_regs),
	m_rgb565(rgb565),
	m_fogdelta_mask(0xff),
	m_thread_stats(WORK_MAX_THREADS)
{
	// empty the hash table
	std::fill(std::begin(m_raster_hash), std::end(m_raster_hash), nullptr);

	// register our arrays
	register_poly_array(m_textures);
	register_poly_array(m_palettes);

	// add all predefined rasterizers
	for (static_rasterizer_info const *info = s_predef_raster_table; info->params.generic() != 0xffffffff; info++)
		add_rasterizer(info->params, info->mfp, false);

	// create entries for the generic rasterizers as well
	rasterizer_params dummy_params;
	for (int index = 0; index < std::size(m_generic_rasterizer); index++)
		m_generic_rasterizer[index] = add_rasterizer(dummy_params, generic_rasterizer(index), true);
}


//-------------------------------------------------
//  register_save - register for saving states
//-------------------------------------------------

void voodoo_renderer::register_save(save_proxy &save)
{
	save.save_item(NAME(m_rowpixels));
	save.save_item(NAME(m_yorigin));
	save.save_item(NAME(m_fogblend));
	save.save_item(NAME(m_fogdelta));
}


//-------------------------------------------------
//  alloc_poly - allocate a new poly_data object
//  and compute the raster parameters
//-------------------------------------------------

poly_data &voodoo_renderer::alloc_poly()
{
	// allocate poly data and compute the rasterization parameters
	poly_data &poly = object_data().next();
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
	vertex_t v1(poly.clipleft, poly.cliptop);
	vertex_t v2(poly.clipright, poly.clipbottom);
	return render_tile<0>(global_cliprect, render_delegate(&voodoo_renderer::rasterizer_fastfill, this), v1, v2);
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
		// add a new one if we're logging usage
		if (LOG_RASTERIZERS)
			info = add_rasterizer(poly.raster, generic_rasterizer(poly.raster.generic()), true);
		else
			info = m_generic_rasterizer[poly.raster.generic()];
	}

	// set the info and render the triangle
	info->polys++;
	poly.info = info;
	return render_triangle<0>(global_cliprect, poly.info->callback, vert[0], vert[1], vert[2]);
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
			int exp = count_leading_zeros_32(iterz) - 4;
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

inline bool ATTR_FORCE_INLINE voodoo_renderer::depth_test(thread_stats_block &threadstats, reg_fbz_mode const fbzmode, s32 depthdest, s32 depthsource)
{
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

inline void ATTR_FORCE_INLINE voodoo_renderer::apply_fogging(rgbaint_t &color, rgb_t fogcolor, u32 depthbias, reg_fbz_mode const fbzmode, reg_fog_mode const fogmode, reg_fbz_colorpath const fbzcp, s32 x, dither_helper const &dither, s32 wfloat, s32 iterz, s64 iterw, rgbaint_t const &iterargb)
{
	// constant fog bypasses everything else
	rgbaint_t fog_color_local(fogcolor);
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
					fog_depth = std::clamp(fog_depth + s16(depthbias), 0, 0xffff);

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

void voodoo_renderer::pixel_pipeline(thread_stats_block &threadstats, u16 *dest, u16 *depth, s32 x, s32 scry, rgb_t src_color, u16 sz)
{
	threadstats.pixels_in++;

	// apply clipping
	auto const fbzmode = m_fbi_reg.fbz_mode();
	if (fbzmode.enable_clipping())
	{
		if (x < m_fbi_reg.clip_left() || x >= m_fbi_reg.clip_right() || scry < m_fbi_reg.clip_top() || scry >= m_fbi_reg.clip_bottom())
		{
			threadstats.clip_fail++;
			return;
		}
	}

	// handle stippling
	if (fbzmode.enable_stipple())
	{
		u32 stipple = m_fbi_reg.stipple();
		if (!stipple_test(threadstats, fbzmode, x, scry, stipple))
			return;
	}

	// Depth testing value for lfb pipeline writes is directly from write data, no biasing is used
	s32 depthval = u32(sz);

	// Perform depth testing
	if (fbzmode.enable_depthbuf() && !depth_test(threadstats, fbzmode, depth[x], (fbzmode.depth_source_compare() == 0) ? depthval : u16(m_fbi_reg.za_color())))
		return;

	// use the RGBA we stashed above
	rgbaint_t color(src_color);

	// handle chroma key
	if (fbzmode.enable_chromakey() && !chroma_key_test(threadstats, color, m_fbi_reg.chroma_key().argb()))
		return;

	// handle alpha mask
	if (fbzmode.enable_alpha_mask() && !alpha_mask_test(threadstats, color.get_a()))
		return;

	// handle alpha test
	auto const alphamode = m_fbi_reg.alpha_mode();
	if (alphamode.alphatest() && !alpha_test(threadstats, alphamode, color.get_a(), alphamode.alpharef()))
		return;

	// perform fogging
	auto const fogmode = m_fbi_reg.fog_mode();
	dither_helper dither(scry, fbzmode, fogmode);
	rgbaint_t prefog(color);
	if (fogmode.enable_fog())
	{
		s32 iterz = sz << 12;
		s64 iterw = m_fbi_reg.lfb_mode().write_w_select() ? u32(m_fbi_reg.za_color() << 16) : u32(sz << 16);
		apply_fogging(color, m_fbi_reg.fog_color().argb(), m_fbi_reg.za_color(), fbzmode, fogmode, m_fbi_reg.fbz_colorpath(), x, dither, depthval, iterz, iterw, rgbaint_t(0));
	}

	// wait for any outstanding work to finish
	wait("pixel_pipeline");

	// perform alpha blending
	if (alphamode.alphablend())
		alpha_blend(color, fbzmode, alphamode, x, dither, dest[x], depth, prefog);

	// pixel pipeline part 2 handles final output
	write_pixel(threadstats, fbzmode, dither, dest, depth, x, color, depthval);
}


//-------------------------------------------------
//  rasterizer - core scanline rasterizer
//-------------------------------------------------

template<u32 GenericFlags, u32 FbzCp, u32 FbzMode, u32 AlphaMode, u32 FogMode, u32 TexMode0, u32 TexMode1>
void voodoo_renderer::rasterizer(s32 y, const voodoo_renderer::extent_t &extent, const poly_data &poly, int threadid)
{
	thread_stats_block &threadstats = m_thread_stats[threadid];
	reg_texture_mode const texmode0(TexMode0, (GenericFlags & rasterizer_params::GENERIC_TEX0) ? poly.raster.texmode0() : 0);
	reg_texture_mode const texmode1(TexMode1, (GenericFlags & rasterizer_params::GENERIC_TEX1) ? poly.raster.texmode1() : 0);
	reg_fbz_colorpath const fbzcp(FbzCp, poly.raster.fbzcp());
	reg_alpha_mode const alphamode(AlphaMode, poly.raster.alphamode());
	reg_fbz_mode const fbzmode(FbzMode, poly.raster.fbzmode());
	reg_fog_mode const fogmode(FogMode, poly.raster.fogmode());
	double iters0, itert0, iterw0, iters1, itert1, iterw1;
	double deltas0, deltat0, deltaw0, deltas1, deltat1, deltaw1;
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
	s32 lodbase0 = 0;
	if (GenericFlags & rasterizer_params::GENERIC_TEX0)
	{
		deltas0 = double(poly.ds0dx);
		deltat0 = double(poly.dt0dx);
		deltaw0 = double(poly.dw0dx);
		iters0 = double(poly.starts0 + dy * poly.ds0dy + dx * poly.ds0dx);
		itert0 = double(poly.startt0 + dy * poly.dt0dy + dx * poly.dt0dx);
		iterw0 = double(poly.startw0 + dy * poly.dw0dy + dx * poly.dw0dx);
		lodbase0 = compute_lodbase(poly.ds0dx, poly.ds0dy, poly.dt0dx, poly.dt0dy);
	}
	s32 lodbase1 = 0;
	if (GenericFlags & rasterizer_params::GENERIC_TEX1)
	{
		deltas1 = double(poly.ds1dx);
		deltat1 = double(poly.dt1dx);
		deltaw1 = double(poly.dw1dx);
		iters1 = double(poly.starts1 + dy * poly.ds1dy + dx * poly.ds1dx);
		itert1 = double(poly.startt1 + dy * poly.dt1dy + dx * poly.dt1dx);
		iterw1 = double(poly.startw1 + dy * poly.dw1dy + dx * poly.dw1dx);
		lodbase1 = compute_lodbase(poly.ds1dx, poly.ds1dy, poly.dt1dx, poly.dt1dy);
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
			if (fbzmode.enable_depthbuf() && !depth_test(threadstats, fbzmode, depth[x], (fbzmode.depth_source_compare() == 0) ? depthval : u16(poly.zacolor)))
				break;

			// run the texture pipeline on TMU1 to produce a value in texel
			rgbaint_t texel(0);
			if (GenericFlags & rasterizer_params::GENERIC_TEX1)
			{
				s32 lod1 = lodbase1;
				rgbaint_t texel_t1 = poly.tex1->fetch_texel(texmode1, dither, x, iters1, itert1, iterw1, lod1, m_bilinear_mask);
				if (GenericFlags & rasterizer_params::GENERIC_TEX1_IDENTITY)
					texel = texel_t1;
				else
					texel = poly.tex1->combine_texture(texmode1, texel_t1, texel, lod1);
			}

			// run the texture pipeline on TMU0 to produce a final result in texel
			if (GenericFlags & rasterizer_params::GENERIC_TEX0)
			{
				// the seq_8_downld flag is repurposed in the rasterizer to indicate
				// we should send the configuration byte
				if (!texmode0.seq_8_downld())
				{
					s32 lod0 = lodbase0;
					rgbaint_t texel_t0 = poly.tex0->fetch_texel(texmode0, dither, x, iters0, itert0, iterw0, lod0, m_bilinear_mask);
					if (GenericFlags & rasterizer_params::GENERIC_TEX0_IDENTITY)
						texel = texel_t0;
					else
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
			if (alphamode.alphatest() && !alpha_test(threadstats, alphamode, color.get_a(), poly.alpharef))
				break;

			// perform fogging
			rgbaint_t prefog(color);
			if (fogmode.enable_fog())
				apply_fogging(color, poly.fogcolor, poly.zacolor, fbzmode, fogmode, fbzcp, x, dither, wfloat, iterz, iterw, iterargb);

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
		if (GenericFlags & rasterizer_params::GENERIC_TEX0)
		{
			iters0 += deltas0;
			itert0 += deltat0;
			iterw0 += deltaw0;
		}
		if (GenericFlags & rasterizer_params::GENERIC_TEX1)
		{
			iters1 += deltas1;
			itert1 += deltat1;
			iterw1 += deltaw1;
		}
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
	switch (texmask & 15)
	{
	default:
	case 0:
		return &voodoo_renderer::rasterizer<0, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::NONE>;
	case 1:
		return &voodoo_renderer::rasterizer<1, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::NONE>;
	case 2:
		return &voodoo_renderer::rasterizer<2, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::DECODE_LIVE>;
	case 3:
		return &voodoo_renderer::rasterizer<3, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>;
	case 4:
		return &voodoo_renderer::rasterizer<4, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::NONE>;
	case 5:
		return &voodoo_renderer::rasterizer<5, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::NONE>;
	case 6:
		return &voodoo_renderer::rasterizer<6, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::DECODE_LIVE>;
	case 7:
		return &voodoo_renderer::rasterizer<7, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>;
	case 8:
		return &voodoo_renderer::rasterizer<8, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::NONE>;
	case 9:
		return &voodoo_renderer::rasterizer<9, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::NONE>;
	case 10:
		return &voodoo_renderer::rasterizer<10, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::DECODE_LIVE>;
	case 11:
		return &voodoo_renderer::rasterizer<11, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>;
	case 12:
		return &voodoo_renderer::rasterizer<12, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::NONE>;
	case 13:
		return &voodoo_renderer::rasterizer<13, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::NONE>;
	case 14:
		return &voodoo_renderer::rasterizer<14, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::NONE, reg_texture_mode::DECODE_LIVE>;
	case 15:
		return &voodoo_renderer::rasterizer<15, reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>;
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
	if (!is_generic || LOG_RASTERIZERS)
	{
		info.next = m_raster_hash[hash];
		m_raster_hash[hash] = &info;
	}

	if (LOG_RASTERIZERS && params.fbzcp().raw() != 0)
	{
		osd_printf_info("Adding rasterizer : gen=%02X cp=%08X am=%08X fog=%08X fbz=%08X tm0=%08X tm1=%08X (hash=%d)\n",
				params.generic(), params.fbzcp().raw(), params.alphamode().raw(), params.fogmode().raw(), params.fbzmode().raw(),
				params.texmode0().raw(), params.texmode1().raw(), hash);

		// explicitly recompute the equations since static rasterizers don't have them
		rasterizer_params computed(params);
		computed.compute_equations();
		osd_printf_info("   Color: %s\n", computed.colorpath_equation().as_string().c_str());
		if (computed.generic() & rasterizer_params::GENERIC_TEX0)
		{
			if (computed.generic() & rasterizer_params::GENERIC_TEX0_IDENTITY)
				osd_printf_info("    Tex0: Identity\n");
			else
				osd_printf_info("    Tex0: %s\n", computed.tex0_equation().as_string().c_str());
		}
		if (computed.generic() & rasterizer_params::GENERIC_TEX1)
		{
			if (computed.generic() & rasterizer_params::GENERIC_TEX1_IDENTITY)
				osd_printf_info("    Tex1: Identity\n");
			else
				osd_printf_info("    Tex1: %s\n", computed.tex1_equation().as_string().c_str());
		}
	}
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

	osd_printf_info("----\n");
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
		osd_printf_info("%s RASTERIZER( 0x%02X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X ) // %8d %10d\n",
			best->is_generic ? "   " : "// ",
			best->params.generic(),
			best->params.fbzcp().raw(),
			best->params.alphamode().raw(),
			best->params.fogmode().raw(),
			best->params.fbzmode().raw(),
			best->params.texmode0().raw(),
			best->params.texmode1().raw(),
			best->polys,
			best->scanlines);

		// reset
		best->display = display_index;
	}
}


//**************************************************************************
//  GAME-SPECIFIC RASTERIZERS
//**************************************************************************

#define RASTERIZER(generic, fbzcp, alpha, fog, fbz, tex0, tex1) \
	{ &voodoo_renderer::rasterizer<generic, fbzcp, fbz, alpha, fog, tex0, tex1>, rasterizer_params(generic, fbzcp, alpha, fog, fbz, tex0, tex1) },

static_rasterizer_info s_predef_raster_table[] =
{
	// wg3dh
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x000000DF, 0xFFFFFFFF ) //  3286099   20381549
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0000009F, 0xFFFFFFFF ) //  2489030   17200373
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0000001F, 0xFFFFFFFF ) //  3014599   13488668
//  RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //   698012   11525474
//  RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x00000ADF, 0xFFFFFFFF ) //   119292    6768046
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0000081F, 0xFFFFFFFF ) //   218912    6316948
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0000089F, 0xFFFFFFFF ) //   176840    6033269
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x0000005F, 0xFFFFFFFF ) //   272493    1528924
	RASTERIZER( 0x05, 0x00480035, 0x00045119, 0x00000000, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //    11590    1026235

	// mace
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  6375980   96752762
	RASTERIZER( 0x05, 0x00602401, 0x00045119, 0x00000000, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //   877666   12853963
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //   537316   12467938
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //   674113   11249705
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0xFFFFFFFF ) //   371317    9571618
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000008F, 0xFFFFFFFF ) //   682233    9124733
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x000000CF, 0xFFFFFFFF ) //   498471    8420845
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //    90032    5376303
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0000080F, 0xFFFFFFFF ) //   107465    5176593
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x00000ADF, 0xFFFFFFFF ) //    13184    5007304

	// sfrush
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000001F ) //  1355672   19357520
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000001F, 0xFFFFFFFF ) //  1297788   14089416
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000081F ) //   607380   12402671
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0000001F ) //   665621   10647858
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x000000DF, 0xFFFFFFFF ) //   714734   10358977
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0000001F, 0xFFFFFFFF ) //   708263    9010666
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0000081F ) //   187026    4932870
	RASTERIZER( 0x0A, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0xFFFFFFFF, 0x000000DF ) //    45383    4310253
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000081F, 0xFFFFFFFF ) //   308972    3302296
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0379, 0x0000001F, 0xFFFFFFFF ) //   285502    2946170
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x082708DF, 0x0000001F ) //   443667    2634580
//  RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //   112446    2477427
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0000081F, 0xFFFFFFFF ) //   195197    2339499
	RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x082708DF, 0x0000001F ) //   365543    2303916

	// sfrushrk
//  RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0000001F ) //  1419527   23908786
//  RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0000001F, 0xFFFFFFFF ) //   963906   14481970
	RASTERIZER( 0x0A, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0xFFFFFFFF, 0x0000001F ) //   174421   10184608
//  RASTERIZER( 0x0B, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x00000000, 0x0000081F ) //   384547    7885615
//  RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x000000DF, 0xFFFFFFFF ) //   244858    4208409
	RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //   206798    3960712
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000001, 0x000B0779, 0x000008DF, 0xFFFFFFFF ) //   153642    3621111
//  RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000001F, 0xFFFFFFFF ) //   108089    3590760
//  RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0x0000081F, 0xFFFFFFFF ) //   219585    2909829
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000001, 0x000B0379, 0x0000001F, 0xFFFFFFFF ) //   187042    2805524
	RASTERIZER( 0x0A, 0x00600C09, 0x00045119, 0x00000001, 0x000B0779, 0xFFFFFFFF, 0x000000DF ) //    25554    2515514

	// calspeed
	RASTERIZER( 0x05, 0x01022819, 0x00000009, 0x00000001, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //  4701502   50735847
	RASTERIZER( 0x05, 0x01022819, 0x00000009, 0x00000001, 0x000B073B, 0x0000000F, 0xFFFFFFFF ) //  1634191   24047354
	RASTERIZER( 0x05, 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0000080F, 0xFFFFFFFF ) //   742857   14056518
	RASTERIZER( 0x05, 0x00002815, 0x00045119, 0x00000001, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //  1299926   12893822
	RASTERIZER( 0x05, 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0000080F, 0xFFFFFFFF ) //   543774   11318818
	RASTERIZER( 0x05, 0x00002815, 0x00045119, 0x00000001, 0x000B07F9, 0x0000000F, 0xFFFFFFFF ) //    40320    5118555
	RASTERIZER( 0x05, 0x01022C19, 0x00000009, 0x00000001, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //   227824    3146670
	RASTERIZER( 0x05, 0x01022C19, 0x00000009, 0x00000001, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    35644    2681027
	RASTERIZER( 0x00, 0x0102001A, 0x00045119, 0x00000001, 0x000A0321, 0xFFFFFFFF, 0xFFFFFFFF ) //    24976    2363490
	RASTERIZER( 0x05, 0x00602819, 0x00045119, 0x00000001, 0x000B07F9, 0x0000080F, 0xFFFFFFFF ) //    28316    2004570

	// vaportrx
	RASTERIZER( 0x05, 0x00482405, 0x00000000, 0x00000000, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //  1446198   21480806
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0739, 0x000000CF, 0xFFFFFFFF ) //  1707856    9876920
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x000000C9, 0xFFFFFFFF ) //   739894    9863144
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0339, 0x00000ACF, 0xFFFFFFFF ) //   460944    9848421
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B07F9, 0x00000ACF, 0xFFFFFFFF ) //   219658    4982710
	RASTERIZER( 0x05, 0x00482405, 0x00000009, 0x00000000, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //   268313    4748332
	RASTERIZER( 0x05, 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x0000000F, 0xFFFFFFFF ) //   283627    3457853
	RASTERIZER( 0x05, 0x00482435, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //    34206    2494986
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x00000000, 0x000B0339, 0x0000000F, 0xFFFFFFFF ) //   153391    2048929

	// blitz
	RASTERIZER( 0x05, 0x00002C35, 0x00515119, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //  5465143   29568854
	RASTERIZER( 0x05, 0x00002C35, 0x00515110, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //   671263   19501211
	RASTERIZER( 0x05, 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   239790   17412073
	RASTERIZER( 0x05, 0x01422439, 0x00000000, 0x00000000, 0x000B073B, 0x000000C9, 0xFFFFFFFF ) //  1174048   13394869
	RASTERIZER( 0x05, 0x00002C35, 0x00515119, 0x00000000, 0x000B0799, 0x00000A0F, 0xFFFFFFFF ) //   781255    6391702
	RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //    47802    3412779
	RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //    79490    3088491
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    15232    2566323
	RASTERIZER( 0x05, 0x00002C35, 0x00515119, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //   177716    2320638
	RASTERIZER( 0x05, 0x00006136, 0x00515119, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    18686    1741744

	// blitz99
	RASTERIZER( 0x05, 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //  5757356   35675229
//  RASTERIZER( 0x05, 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   270977   16139833
//  RASTERIZER( 0x05, 0x00002C35, 0x64515119, 0x00000000, 0x000B0799, 0x00000A0F, 0xFFFFFFFF ) //   869068    7819874
//  RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //    57874    5309214
//  RASTERIZER( 0x05, 0x01422439, 0x00000000, 0x00000000, 0x000B073B, 0x000000C9, 0xFFFFFFFF ) //   380688    4597915
//  RASTERIZER( 0x05, 0x00006136, 0x40515119, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    18141    2099641
//  RASTERIZER( 0x05, 0x00002C35, 0x40515119, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //    92655    1928223
//  RASTERIZER( 0x05, 0x00002C35, 0x40515119, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //   131380    1908198
//  RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //    86764    1760469
	RASTERIZER( 0x05, 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //   189197    1504639

	// blitz2k
//  RASTERIZER( 0x05, 0x00000035, 0x00000009, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //  5486749   33668808
//  RASTERIZER( 0x05, 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   275360   15899691
//  RASTERIZER( 0x05, 0x00002C35, 0x00515119, 0x00000000, 0x000B0799, 0x00000A0F, 0xFFFFFFFF ) //   801050    7476260
//  RASTERIZER( 0x05, 0x01422439, 0x00000000, 0x00000000, 0x000B073B, 0x000000C9, 0xFFFFFFFF ) //   461364    5981963
//  RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //    93786    5079967
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000000, 0x000B073B, 0x00000ACF, 0xFFFFFFFF ) //    16252    2943314
	RASTERIZER( 0x05, 0x00002C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   200589    2563939
//  RASTERIZER( 0x05, 0x00006136, 0x00515119, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    18221    2335412
//  RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //    90486    2212467

	// carnevil
	RASTERIZER( 0x05, 0x00002425, 0x00045119, 0x00000000, 0x00030679, 0x00000A0F, 0xFFFFFFFF ) //   420627    4298755
	RASTERIZER( 0x05, 0x00002435, 0x00045119, 0x00000000, 0x000302F9, 0x0000080F, 0xFFFFFFFF ) //   112660    2916676
	RASTERIZER( 0x05, 0x00002435, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //    36468     735846
	RASTERIZER( 0x05, 0x00000035, 0x08045119, 0x00000000, 0x000306F9, 0x00000AC9, 0xFFFFFFFF ) //     6996     212717
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x00000000, 0x00030679, 0x00000A0F, 0xFFFFFFFF ) //      754     190433

	// hyprdriv
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000001, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //   146568   18142019
	RASTERIZER( 0x05, 0x01422C19, 0x00000000, 0x00000001, 0x000B073B, 0x00000A0F, 0xFFFFFFFF ) //   771521   12744098
	RASTERIZER( 0x05, 0x00582435, 0x00515110, 0x00000001, 0x000B0739, 0x00000AC9, 0xFFFFFFFF ) //   997666   12173827
	RASTERIZER( 0x00, 0x0142612A, 0x00000000, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) //  2064812   11818083
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000001, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    67518    8476957
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x00000A0F, 0xFFFFFFFF ) //  3404038    8059738
	RASTERIZER( 0x00, 0x0142611A, 0x00045110, 0x00000001, 0x000B0739, 0xFFFFFFFF, 0xFFFFFFFF ) //   797160    3625519
	RASTERIZER( 0x05, 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x00000A1F, 0xFFFFFFFF ) //   154937    3337816
	RASTERIZER( 0x05, 0x00582C35, 0x00515110, 0x00000001, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //    19770    2887063
	RASTERIZER( 0x05, 0x01420039, 0x00000000, 0x00000001, 0x000B073B, 0x0000000F, 0xFFFFFFFF ) //   392772    2654749
	RASTERIZER( 0x05, 0x01422429, 0x00000000, 0x00000001, 0x000B073B, 0x0000001F, 0xFFFFFFFF ) //   101694    2333476
	RASTERIZER( 0x05, 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x00000A1F, 0xFFFFFFFF ) //   152748    2258208
	RASTERIZER( 0x05, 0x00580035, 0x00000000, 0x00000001, 0x000B073B, 0x0000001F, 0xFFFFFFFF ) //   164822    2100196

	// gauntleg
	RASTERIZER( 0x0B, 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22400F, 0x00000ACF ) //  1846512   77235960
	RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  1733863   14067084
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x0000004F, 0xFFFFFFFF ) //  1517970   13213493
//  RASTERIZER( 0x05, 0x00600C09, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  2449825   12337057
	RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000000F ) //   193539   10327282
	RASTERIZER( 0x0B, 0x00600039, 0x00045119, 0x00000000, 0x000B0779, 0x0C22480F, 0x00000ACF ) //   173388    5900312
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000004F ) //   272048    5557244
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x00000A4F ) //    38452    4404158
	RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000080F ) //    93156    4365283
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x00000A4F, 0xFFFFFFFF ) //    51416    3857019
	RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //   220103    2783489

	// gauntdl
	RASTERIZER( 0x0B, 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x00000ACF ) //  4613022  100488623
	RASTERIZER( 0x0B, 0x0060743A, 0x00045110, 0x000000C1, 0x000B0779, 0x0C22400F, 0x00000ACF ) //  2989550   50643553
	RASTERIZER( 0x0B, 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22480F, 0x00000ACF ) //   741940   22885885
	RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  2843876   16209403
	RASTERIZER( 0x0B, 0x0060743A, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22488F, 0x00000ACF ) //   349018   11700233
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x000000CF, 0xFFFFFFFF ) //   798144    7819799
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x000008CF ) //   217622    7362228
	RASTERIZER( 0x0B, 0x00602439, 0x00044110, 0x00000000, 0x000B0379, 0x00000009, 0x0000000F ) //   111605    6859677
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x00000ACF ) //    42994    6006533
	RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //  1174759    5414174
	RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000009, 0x0000000F ) //  1358847    5366960
	RASTERIZER( 0x05, 0x00602C19, 0x00045110, 0x000000C1, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  1317912    5103516

	// warfa
	RASTERIZER( 0x0B, 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x00000ACF ) //  5549270   83260187
//  RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  2450037   34142048
	RASTERIZER( 0x0B, 0x00602419, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22400F, 0x00000A0F ) //  1406309   22020895
	RASTERIZER( 0x05, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  3121611   13535796
	RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000000, 0x0000000F ) //  3821464   12671381
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x104008CF, 0x000008CF ) //  1045140   10170316
	RASTERIZER( 0x0B, 0x00602439, 0x00044119, 0x000000C1, 0x000B0779, 0x0582480F, 0x0000080F ) //    43674    7349020
//  RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x00000000, 0x000B0779, 0x00000000, 0x0000000F ) //   864570    6795448
	RASTERIZER( 0x0B, 0x00602C19, 0x00045119, 0x000000C1, 0x000B0779, 0x00000000, 0x0000080F ) //   531132    6584618
	RASTERIZER( 0x0B, 0x00602439, 0x00045119, 0x000000C1, 0x000B0779, 0x0C22480F, 0x00000ACF ) //   169887    6529727
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x10400ACF, 0x000008CF ) //   420204    5602132
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x000000C1, 0x000B0779, 0x104000CF, 0x000008CF ) //   306432    4918886

	//
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  5627690   79723246
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x00000A0F, 0xFFFFFFFF ) //   250480   17957361
	RASTERIZER( 0x0B, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000000F, 0x0000000F ) //  4264955   17065131
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0xFFFFFFFF ) //   789463   11602094
	RASTERIZER( 0x0B, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x000008CF ) //   452695   11374291
	RASTERIZER( 0x0B, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0000080F ) //  2219901   10964372
	RASTERIZER( 0x0B, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x0000000F ) //  2985476   10576909
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0xFFFFFFFF ) //   430488    6729680
	RASTERIZER( 0x0B, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x000008CF, 0x000008CF ) //   317486    5192722
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //   551185    3395443
	RASTERIZER( 0x0B, 0x00602409, 0x00045119, 0x00000000, 0x000B0779, 0x0000088F, 0x0000080F ) //   276296    3024329

	// nbashowt
	RASTERIZER( 0x05, 0x00002C35, 0x00045119, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) // 12589303   89992094
	RASTERIZER( 0x05, 0x00002435, 0x00045119, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   528689   41431369
	RASTERIZER( 0x05, 0x00002C35, 0x00045119, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //  4346462   21676464
	RASTERIZER( 0x05, 0x00002C35, 0x00044110, 0x00000000, 0x000A0321, 0x00000A0F, 0xFFFFFFFF ) //  1311258   15990693
	RASTERIZER( 0x05, 0x00582435, 0x00045110, 0x00000000, 0x000B0739, 0x00000AC9, 0xFFFFFFFF ) //   498752    8603534
//  RASTERIZER( 0x05, 0x00000035, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   262625    8248367
	RASTERIZER( 0x05, 0x00582C35, 0x00045110, 0x00000000, 0x000B0739, 0x00000ACF, 0xFFFFFFFF ) //   146310    7881711
	RASTERIZER( 0x05, 0x01424039, 0x00045110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   353983    6698613
	RASTERIZER( 0x05, 0x01422439, 0x00044119, 0x00000000, 0x000A0321, 0x00000A0F, 0xFFFFFFFF ) //   105456    5867982

	// sf2049
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0000000F, 0xFFFFFFFF ) //  4535144   87108835
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x000008CF, 0xFFFFFFFF ) //  3460768   36622999
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x000000CF, 0xFFFFFFFF ) //  1167220   12527933
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x000008CF, 0xFFFFFFFF ) //   294802   12127575
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0000088F, 0xFFFFFFFF ) //   192929   11588013
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0000080F, 0xFFFFFFFF ) //   322059   10421128
	RASTERIZER( 0x03, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x0A452A0F, 0x0E47200F ) //  1449341    8868943
	RASTERIZER( 0x05, 0x00482435, 0x00045117, 0x000000C1, 0x000B0339, 0x0000000F, 0xFFFFFFFF ) //   874920    7250149
	RASTERIZER( 0x05, 0x00482435, 0x00045119, 0x000000C1, 0x000B0339, 0x0000000F, 0xFFFFFFFF ) //   874920    6973439
	RASTERIZER( 0x05, 0x00602401, 0x00045119, 0x000000C1, 0x00030279, 0x00000A0F, 0xFFFFFFFF ) //   453774    6485154
	RASTERIZER( 0x05, 0x00602409, 0x00045119, 0x000000C1, 0x000B0779, 0x00000A0F, 0xFFFFFFFF ) //   220733    5697617
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x000000C1, 0x000B0379, 0x0000000F, 0xFFFFFFFF ) //   348786    4416017

	// cartfury
	RASTERIZER( 0x05, 0x00000035, 0x00045119, 0x000000C1, 0x00030F39, 0x00000A0F, 0xFFFFFFFF ) //  3719502   47429534
	RASTERIZER( 0x05, 0x00420039, 0x00000000, 0x000000C1, 0x00030F39, 0x0000000F, 0xFFFFFFFF ) //  4821876   25433268
	RASTERIZER( 0x05, 0x0142A409, 0x00000000, 0x000000C1, 0x00030F3B, 0x00000ACF, 0xFFFFFFFF ) //  1704143   16287730
	RASTERIZER( 0x05, 0x00580035, 0x00045119, 0x000000C1, 0x00030B39, 0x00000A0F, 0xFFFFFFFF ) //   576134   13181152
	RASTERIZER( 0x05, 0x0142A409, 0x00000000, 0x00000000, 0x00030B39, 0x00000A0F, 0xFFFFFFFF ) //  2447036   13056499
	RASTERIZER( 0x05, 0x00420039, 0x00000000, 0x00000000, 0x00030F39, 0x00000A0F, 0xFFFFFFFF ) //  1597651   11909086
	RASTERIZER( 0x05, 0x00422439, 0x00000000, 0x000000C1, 0x00030F3B, 0x00000A0F, 0xFFFFFFFF ) //  1100912   11879195
	RASTERIZER( 0x05, 0x00420039, 0x00000000, 0x00000000, 0x00030F3B, 0x00000A0F, 0xFFFFFFFF ) //  1714391   11199323
	RASTERIZER( 0x05, 0x00582435, 0x00045110, 0x00000000, 0x00030BF9, 0x000000C9, 0xFFFFFFFF ) //   975518   11030280

	// gradius4
	RASTERIZER( 0x05, 0x00000005, 0x00005119, 0x00000000, 0x00030BFB, 0x00000AC7, 0xFFFFFFFF ) //  1261361   78858051
	RASTERIZER( 0x05, 0x0000303A, 0x00004119, 0x00000000, 0x00030BFB, 0x000000C7, 0xFFFFFFFF ) //   398143   72518712
	RASTERIZER( 0x05, 0x00000005, 0x00005119, 0x00000000, 0x00030F7B, 0x00000A87, 0xFFFFFFFF ) //  1716273   15992169
	RASTERIZER( 0x05, 0x00000005, 0x00005119, 0x00000000, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   812651   15723260
	RASTERIZER( 0x05, 0x00582435, 0x00005119, 0x00000000, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   595637   13777035
	RASTERIZER( 0x05, 0x00000015, 0x00005119, 0x00000000, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   675880   12288373
	RASTERIZER( 0x00, 0x02422E12, 0x00005119, 0x00000000, 0x00030F7B, 0xFFFFFFFF, 0xFFFFFFFF ) //   404825   10544497
	RASTERIZER( 0x05, 0x00000005, 0x00005119, 0x00000000, 0x00030FFB, 0x00000AC7, 0xFFFFFFFF ) //   444690    6872107
	RASTERIZER( 0x00, 0x02420002, 0x00000009, 0x00000000, 0x00030F7B, 0xFFFFFFFF, 0xFFFFFFFF ) //  5455064    5726069
	RASTERIZER( 0x05, 0x00580021, 0x00005119, 0x00000000, 0x00030FFB, 0x00000AC7, 0xFFFFFFFF ) //   242000    5057019

	// nbapbp
	RASTERIZER( 0x05, 0x00426E19, 0x00000000, 0x00000001, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //  2926955   28637513
	RASTERIZER( 0x05, 0x00424219, 0x00000000, 0x00000001, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   607076   17008880
	RASTERIZER( 0x05, 0x00422809, 0x00004610, 0x00000001, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   562460   12415476
	RASTERIZER( 0x05, 0x02004219, 0x00000000, 0x00000001, 0x00030F7B, 0x00000AC1, 0xFFFFFFFF ) //    79809    7045963
	RASTERIZER( 0x05, 0x02004219, 0x00000000, 0x00000001, 0x00030B7B, 0x00000AC7, 0xFFFFFFFF ) //    94254    6047743
	RASTERIZER( 0x05, 0x00006E19, 0x00000000, 0x00000001, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   352375    4465810
	RASTERIZER( 0x05, 0x00422A19, 0x00004610, 0x00000001, 0x00030BFB, 0x00000AC7, 0xFFFFFFFF ) //    58835    3500582
	RASTERIZER( 0x05, 0x00004219, 0x00000000, 0x00000001, 0x00030F7B, 0x00000AC7, 0xFFFFFFFF ) //   176291    2268659
	RASTERIZER( 0x05, 0x00424219, 0x00000000, 0x00000001, 0x00030B7B, 0x00000AC7, 0xFFFFFFFF ) //    13304    2250402

	// virtpool
	RASTERIZER( 0x05, 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   494481   13370166
	RASTERIZER( 0x05, 0x00002421, 0x00000000, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    45923    3905828
	RASTERIZER( 0x05, 0x00002425, 0x00445110, 0x00000000, 0x000B07F9, 0x00000A0F, 0xFFFFFFFF ) //    57946    3043998
	RASTERIZER( 0x05, 0x00482405, 0x00045110, 0x00000000, 0x000B0739, 0x00000A0F, 0xFFFFFFFF ) //   235221     962382
	RASTERIZER( 0x05, 0x00002421, 0x00000000, 0x00000000, 0x000B0739, 0x00000A09, 0xFFFFFFFF ) //    12297     930523

	// gtfore01
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x000000C1, 0x00010F79, 0x00000ACD, 0xFFFFFFFF ) //   807566   29151846
	RASTERIZER( 0x0B, 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x00000A0D ) //  2116043   18259224
	RASTERIZER( 0x0B, 0x00002429, 0x00000000, 0x000000C1, 0x00010FF9, 0x00000A09, 0x00000A0F ) //    43784    3594532
	RASTERIZER( 0x0B, 0x00002425, 0x00045110, 0x000000C1, 0x00010FF9, 0x00000ACD, 0x00000ACD ) //    14899    1391390

	// gtfore02
	RASTERIZER( 0x0B, 0x00002425, 0x00045119, 0x000000C1, 0x00010F79, 0x0C224A0D, 0x00000ACD ) //   776841   12879143

	// gtfore06
	RASTERIZER( 0x05, 0x00482405, 0x00045119, 0x000000C1, 0x00010FF9, 0x00000ACD, 0xFFFFFFFF ) //    51144    2582597

	{ nullptr, rasterizer_params(0xffffffff) }
};

}
