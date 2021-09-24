// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_render.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_RENDER_H
#define MAME_VIDEO_VOODOO_RENDER_H

#pragma once

#include "video/poly.h"
#include "video/rgbutil.h"


namespace voodoo
{

// forward declarations
struct rasterizer_info;
struct poly_data;
class dither_helper;

// base class for our renderer
using voodoo_poly_manager = poly_manager<float, poly_data, 0, POLY_FLAG_NO_CLIPPING>;



//**************************************************************************
//  DITHER HELPER
//**************************************************************************

// ======================> dither_helper

// this class provides common code for querying and managing dithering
// effects, which are very particular to the Voodoo
class dither_helper
{
public:
	// constructor to pre-cache based on mode and Y coordinate
	dither_helper(int y, reg_fbz_mode const fbzmode, reg_fog_mode const fogmode = reg_fog_mode(0)) :
		m_dither_lookup(nullptr),
		m_dither_raw(nullptr),
		m_dither_raw_4x4(&s_dither_matrix_4x4[(y & 3) * 4])
	{
		// still use a lookup for no dithering since it's rare and we
		// can avoid yet another conditional on the hot path
		if (!fbzmode.enable_dithering())
			m_dither_lookup = &s_nodither_lookup[0];
		else if (fbzmode.dither_type() == 0)
		{
			m_dither_lookup = &s_dither4_lookup[(y & 3) << 11];
			m_dither_raw = &s_dither_matrix_4x4[(y & 3) * 4];
		}
		else
		{
			m_dither_lookup = &s_dither2_lookup[(y & 3) << 11];
			m_dither_raw = &s_dither_matrix_2x2[(y & 3) * 4];
		}
	}

	// apply dithering to a pixel in separate R/G/B format and assemble as 5-6-5
	u16 pixel(s32 x, s32 r, s32 g, s32 b) const
	{
		u8 const *table = &m_dither_lookup[(x & 3) << 9];
		return (table[r] << 11) | (table[g + 256] << 5) | table[b];
	}

	// apply dithering to a pixel in separate R/G/B format and assemble as 5-6-5
	u16 pixel(s32 x, rgb_t color) const
	{
		u8 const *table = &m_dither_lookup[(x & 3) << 9];
		return (table[color.r()] << 11) | (table[color.g() + 256] << 5) | table[color.b()];
	}

	// apply dithering to an rgbint_t pixel and assemble as 5-6-5
	u16 pixel(s32 x, rgbaint_t const &color) const
	{
		u8 const *table = &m_dither_lookup[(x & 3) << 9];
		return (table[color.get_r()] << 11) | (table[color.get_g() + 256] << 5) | table[color.get_b()];
	}

	// return the raw 4x4 dither pattern
	u32 raw_4x4(s32 x) const
	{
		return m_dither_raw_4x4[x & 3];
	}

	// return the subtractive dither value for alpha blending
	u32 subtract(s32 x) const
	{
		return (m_dither_raw != nullptr) ? ((15 - m_dither_raw[x & 3]) >> 1) : 0;
	}

	// allocate and initialize static tables
	static void init_static();

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
	u8 const *m_dither_raw_4x4;

	// static tables
	static std::unique_ptr<u8[]> s_dither4_lookup;
	static std::unique_ptr<u8[]> s_dither2_lookup;
	static std::unique_ptr<u8[]> s_nodither_lookup;
	static u8 const s_dither_matrix_4x4[4*4];
	static u8 const s_dither_matrix_2x2[4*4];
};


// ======================> color_source

// color_source describes the alpha+RGB components of a color in an
// abstract way
class color_source
{
public:
	// flags
	static constexpr u8 FLAG_INVERTED = 0x80;
	static constexpr u8 FLAG_ALPHA_EXPANDED = 0x40;

	// constant values (0-3)
	static constexpr u8 ZERO = 0;
	static constexpr u8 ONE = 1;
	static constexpr u8 COLOR0 = 2;
	static constexpr u8 COLOR1 = 3;

	// iterated values (4-7)
	static constexpr u8 ITERATED_ARGB = 4;
	static constexpr u8 CLAMPZ = 5;
	static constexpr u8 CLAMPW = 6;

	// dynamic values (8+)
	static constexpr u8 TEXEL0 = 8;
	static constexpr u8 TEXEL1 = 9;
	static constexpr u8 DETAIL_FACTOR = 10;
	static constexpr u8 LOD_FRACTION = 11;
	static constexpr u8 COLOR0_OR_ITERATED_VIA_TEXEL_ALPHA = 12;

	// constructor
	constexpr color_source(u8 alpha = ZERO, u8 rgb = ZERO) : m_alpha(alpha), m_rgb(rgb) { }

	// exact comparisons
	bool operator==(color_source const &rhs) const { return m_rgb == rhs.m_rgb && m_alpha == rhs.m_alpha; }
	bool operator!=(color_source const &rhs) const { return m_rgb != rhs.m_rgb || m_alpha != rhs.m_alpha; }

	// return the full alpha/RGB value
	u8 alpha() const { return m_alpha; }
	u8 rgb() const { return m_rgb; }

	// return the base (flag-free) alpha/RGB value
	u8 alpha_base() const { return m_alpha & 15; }
	u8 rgb_base() const { return m_rgb & 15; }

	// return the alpha/RGB value flags
	u8 alpha_flags() const { return m_alpha >> 6; }
	u8 rgb_flags() const { return m_rgb >> 6; }

	// helpers
	bool is_rgb_zero() const { return m_rgb == ZERO; }
	bool is_alpha_zero() const { return m_alpha == ZERO; }
	bool is_zero() const { return is_rgb_zero() && is_alpha_zero(); }
	bool is_rgb_one() const { return m_rgb == ONE; }
	bool is_alpha_one() const { return m_alpha == ONE; }
	bool is_one() const { return is_rgb_one() && is_alpha_one(); }

	// uniform is true if RGB and alpha come from the same source
	bool is_uniform() const { return (m_alpha == m_rgb); }

	// uniform_alpha is true if RGB and alpha are replication of the same alpha value
	bool is_uniform_alpha() const { return ((m_alpha | FLAG_ALPHA_EXPANDED) == m_rgb); }

	// constant is true if both values are constant across a scanline
	bool is_rgb_constant() const { return ((m_rgb & 0x0c) == 0x00); }
	bool is_alpha_constant() const { return ((m_alpha & 0x0c) == 0x00); }
	bool is_constant() const { return is_rgb_constant() && is_alpha_constant(); }

	// partial_constant is true if at least one value is constant across a scanline
	bool is_partial_constant() const { return is_rgb_constant() || is_alpha_constant(); }

	// constant_or_iterated is true if values are constant or simply iterated across a scanline
	bool is_rgb_constant_or_iterated() const { return ((m_rgb & 0x08) == 0x00); }
	bool is_alpha_constant_or_iterated() const { return ((m_alpha & 0x08) == 0x00); }
	bool is_constant_or_iterated() const { return is_rgb_constant_or_iterated() && is_alpha_constant_or_iterated(); }

	// uses_any is true if either the RGB or alpha referenecs the given source
	bool uses_any(color_source const &src) const { return rgb_base() == src.rgb_base() || alpha_base() == src.alpha_base(); }

	// directly set the alpha/RGB component
	void set_alpha(u8 alpha) { m_alpha = alpha; }
	void set_rgb(u8 rgb) { m_rgb = rgb; }

	// set the RGB as an expanded single component
	void set_rgb_from_alpha(u8 rgb) { m_rgb = rgb | FLAG_ALPHA_EXPANDED; }

	// mark the RGB/alpha component as inverted
	void invert_rgb() { m_rgb ^= FLAG_INVERTED; }
	void invert_alpha() { m_alpha ^= FLAG_INVERTED; }

	// perform internal simplification
	void simplify();

	// return a string version of the component
	std::string as_string() const;

	// constants
	static color_source const zero;
	static color_source const one;
	static color_source const iterated_argb;
	static color_source const color0;
	static color_source const color1;
	static color_source const texel0;
	static color_source const texel1;

private:
	// internal state
	u8 m_alpha, m_rgb;
};


// ======================> color_equation

// color_equation describes a set of 4 color sources, intended to be computed
// as clamp((color - sub) * multiply + add)
class color_equation
{
public:
	// construction
	constexpr color_equation() { }

	// simple getters
	color_source &color() { return m_color; }
	color_source &subtract() { return m_subtract; }
	color_source &multiply() { return m_multiply; }
	color_source &add() { return m_add; }

	// helpers
	bool uses_any(color_source color) const { return m_color.uses_any(color) || m_subtract.uses_any(color) || m_multiply.uses_any(color) || m_add.uses_any(color); }
	bool is_identity(color_source color) const { return (m_multiply.is_zero() && m_add == color); }
	bool is_zero() const { return m_multiply.is_zero() && m_add.is_zero(); }

	// operations
	void simplify();
	std::string as_string() const;

	// computation
	static color_equation from_fbzcp(reg_fbz_colorpath const fbzcp);
	static color_equation from_texmode(reg_texture_mode const texmode, color_source texel_color, color_source input_color);

private:
	// internal state
	color_source m_color;
	color_source m_subtract;
	color_source m_multiply;
	color_source m_add;
};


// ======================> rasterizer_params

// this class holds the representative parameters that characterize a
// specific rasterizer; these are used to index and discover one of
// the special hard-coded rasterizers in voodoo_render.cpp
class rasterizer_params
{
public:
	// generic flags
	static constexpr u32 GENERIC_TEX0 = 0x01;
	static constexpr u32 GENERIC_TEX1 = 0x02;
	static constexpr u32 GENERIC_TEX0_IDENTITY = 0x04;
	static constexpr u32 GENERIC_TEX1_IDENTITY = 0x08;

	// construction
	constexpr rasterizer_params(u32 generic = 0, u32 fbzcp = 0, u32 alphamode = 0, u32 fogmode = 0, u32 fbzmode = 0, u32 texmode0 = 0, u32 texmode1 = 0) :
		m_generic(generic),
		m_fbzcp(fbzcp),
		m_alphamode(alphamode),
		m_fogmode(fogmode),
		m_fbzmode(fbzmode),
		m_texmode0(texmode0),
		m_texmode1(texmode1) { }

	// compare everything directly
	bool operator==(rasterizer_params const &rhs) const;

	// compute the parameters given a set of registers
	void compute(voodoo_regs &regs, voodoo_regs *tmu0regs = nullptr, voodoo_regs *tmu1regs = nullptr);
	void compute_equations();

	// compute the hash of the settings
	u32 hash() const;

	// getters
	u32 generic() const { return m_generic; }
	reg_fbz_colorpath fbzcp() const { return reg_fbz_colorpath(m_fbzcp); }
	reg_alpha_mode alphamode() const { return reg_alpha_mode(m_alphamode); }
	reg_fog_mode fogmode() const { return reg_fog_mode(m_fogmode); }
	reg_fbz_mode fbzmode() const { return reg_fbz_mode(m_fbzmode); }
	reg_texture_mode texmode0() const { return reg_texture_mode(m_texmode0); }
	reg_texture_mode texmode1() const { return reg_texture_mode(m_texmode1); }
	color_equation const &colorpath_equation() const { return m_color_equation; }
	color_equation const &tex0_equation() const { return m_tex0_equation; }
	color_equation const &tex1_equation() const { return m_tex1_equation; }

private:
	// internal helpers
	static constexpr u32 rotate(u32 value, int count) { return (value << count) | (value >> (32 - count)); }

	// internal state
	u32 m_generic;      // 4 bits
	u32 m_fbzcp;        // 30 bits
	u32 m_alphamode;    // 32 bits
	u32 m_fogmode;      // 8 bits
	u32 m_fbzmode;      // 22 bits
	u32 m_texmode0;     // 31 bits
	u32 m_texmode1;     // 31 bits
	color_equation m_color_equation;
	color_equation m_tex0_equation;
	color_equation m_tex1_equation;
};


// ======================> rasterizer_texture

// this class holds TMU-specific decoded data regarding a texture; it is
// encapsulated here, with functions to derive it from register info and
// functions to process it as part of the rendering pipeline
class rasterizer_texture
{
public:
	// recompute internal values based on parameters
	void recompute(voodoo_regs const &regs, u8 *ram, u32 mask, rgb_t const *lookup, u32 addrmask, u8 addrshift);

	// look up a texel at the given coordinate
	rgb_t lookup_single_texel(u32 format, u32 texbase, s32 s, s32 t);

	// fetch a texel given coordinates and LOD information
	rgbaint_t fetch_texel(voodoo::reg_texture_mode const texmode, voodoo::dither_helper const &dither, s32 x, double iters, double itert, double iterw, s32 &lod, u8 bilinear_mask);

	// texture-specific color combination unit
	rgbaint_t combine_texture(voodoo::reg_texture_mode const texmode, rgbaint_t const &c_local, rgbaint_t const &c_other, s32 lod);

	// return a write pointer based on the LOD, s/t coordinates, and format
	u8 *write_ptr(u32 lod, u32 s, u32 t, u32 scale) const
	{
		u32 offs = t * ((m_wmask >> lod) + 1) + s;
		return m_ram + ((m_lodoffset[lod] + ((scale * offs) & ~3)) & m_mask);
	}

private:
	// internal state
	rgb_t const *m_lookup;      // currently selected lookup
	u8 *m_ram;                  // pointer to base of TMU RAM
	u8 m_wmask;                 // mask for the current texture width
	u8 m_hmask;                 // mask for the current texture height
	u8 m_detailscale;           // detail scale
	s16 m_lodmin;               // minimum LOD value
	s16 m_lodmax;               // maximum LOD value
	s16 m_lodbias;              // LOD bias
	u16 m_lodmask;              // mask of available LODs
	u32 m_mask;                 // mask to apply to pointers
	s32 m_detailmax;            // detail clamp
	s32 m_detailbias;           // detail bias
	u32 m_lodoffset[9];         // offset of texture base for each LOD
};


// ======================> rasterizer_palette

class rasterizer_palette
{
public:
	// compute from an NCC table
	void compute_ncc(u32 const *regs);

	// copy from a table
	void copy(rgb_t *texels) { memcpy(&m_texel, texels, sizeof(m_texel)); }

	// simple getters
	rgb_t const *texels() const { return &m_texel[0]; }

private:
	// internal state
	rgb_t m_texel[256];
};


// ======================> poly_data

// this struct contains the polygon-wide shared data used during rendering;
// it is captured here so that further changed can be made to the registers
// without affecting pending operations
struct poly_data
{
	rasterizer_params raster;   // normalized rasterizer parameters, for triangles
	rasterizer_info *info;      // pointer to rasterizer information
	u16 *destbase;              // destination to write
	u16 *depthbase;             // depth/aux buffer to write
	rasterizer_texture *tex0;   // texture 0 information
	rasterizer_texture *tex1;   // texture 1 information
	u16 clipleft, clipright;    // x clipping
	u16 cliptop, clipbottom;    // y clipping

	s16 ax, ay;                 // vertex A x,y (12.4)
	s32 startr, startg, startb, starta; // starting R,G,B,A (12.12)
	s32 startz;                 // starting Z (20.12)
	s64 startw;                 // starting W (16.32)
	s32 drdx, dgdx, dbdx, dadx; // delta R,G,B,A per X
	s32 dzdx;                   // delta Z per X
	s64 dwdx;                   // delta W per X
	s32 drdy, dgdy, dbdy, dady; // delta R,G,B,A per Y
	s32 dzdy;                   // delta Z per Y
	s64 dwdy;                   // delta W per Y

	s64 starts0, startt0;       // starting S,T (14.18)
	s64 startw0;                // starting W (2.30)
	s64 ds0dx, dt0dx;           // delta S,T per X
	s64 dw0dx;                  // delta W per X
	s64 ds0dy, dt0dy;           // delta S,T per Y
	s64 dw0dy;                  // delta W per Y

	s64 starts1, startt1;       // starting S,T (14.18)
	s64 startw1;                // starting W (2.30)
	s64 ds1dx, dt1dx;           // delta S,T per X
	s64 dw1dx;                  // delta W per X
	s64 ds1dy, dt1dy;           // delta S,T per Y
	s64 dw1dy;                  // delta W per Y

	rgb_t color0, color1;       // colors consumed by the rasterizer
	rgb_t chromakey;            // chromakey
	rgb_t fogcolor;             // fogcolor
	u32 zacolor;                // depth/alpha value consumed by the rasterizer
	u32 stipple;                // stipple pattern
	u32 alpharef;               // reference alpha value

	u16 dither[16];             // dither matrix, for fastfill
};


// ======================> rasterizer_info

// this struct describes a specific rasterizer
struct rasterizer_info
{
	rasterizer_info *next;      // pointer to next entry with the same hash
	voodoo_poly_manager::render_delegate callback; // callback pointer
	u8 is_generic;              // is this a generic rasterizer?
	u8 display;                 // display index, used for sorted printing
	u32 scanlines;              // how many scanlines we've used this for
	u32 polys;                  // how many polys we've used this for
	u32 fullhash;               // full 32-bit hash
	rasterizer_params params;   // full copy of the relevant parameters
};


// ======================> thread_stats_block

// this struct holds a thread-specific chunk of statistics that are combined
// on demand with other threads' data when requested
struct thread_stats_block
{
	void reset()
	{
		pixels_in = pixels_out = chroma_fail = zfunc_fail = afunc_fail = clip_fail = stipple_count = 0;
	}

	s32 pixels_in = 0;          // pixels in statistic
	s32 pixels_out = 0;         // pixels out statistic
	s32 chroma_fail = 0;        // chroma test fail statistic
	s32 zfunc_fail = 0;         // z function test fail statistic
	s32 afunc_fail = 0;         // alpha function test fail statistic
	s32 clip_fail = 0;          // clipping fail statistic
	s32 stipple_count = 0;      // stipple statistic
	s32 filler[poly_array<int,1>::CACHE_LINE_SIZE/4 - 7]; // pad this structure to cache line size
};


// ======================> voodoo_renderer

class voodoo_renderer : public voodoo_poly_manager
{
	static constexpr u32 RASTER_HASH_SIZE = 97; // size of the rasterizer hash table

public:
	using rasterizer_mfp = void (voodoo_renderer::*)(int32_t, const extent_t &, const poly_data &, int);

	// construction
	voodoo_renderer(running_machine &machine, u16 tmu_config, const rgb_t *rgb565, voodoo_regs &fbi_regs, voodoo_regs *tmu0_regs, voodoo_regs *tmu1_regs);

	// state saving
	void register_save(save_proxy &save);

	// simple getters
	s32 yorigin() const { return m_yorigin; }
	u32 rowpixels() const { return m_rowpixels; }
	u16 tmu_config() const { return m_tmu_config; }
	std::vector<thread_stats_block> &thread_stats() { return m_thread_stats; }

	// simple setters
	void set_tmu_config(u16 value) { m_tmu_config = value; }
	void set_fogdelta_mask(u8 value) { m_fogdelta_mask = value; }
	void set_bilinear_mask(u8 value) { m_bilinear_mask = value; }

	// allocate a new poly_data and fill in the rasterizer_params
	poly_data &alloc_poly();

	// enqueue operations
	u32 enqueue_fastfill(poly_data &poly);
	u32 enqueue_triangle(poly_data &poly, vertex_t const *vert);

	// core triangle rasterizer
	template<u32 GenericFlags, u32 FbzCp, u32 FbzMode, u32 AlphaMode, u32 FogMode, u32 TexMode0, u32 TexMode1>
	void rasterizer(s32 y, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_data &extra, int threadid);

	// run the pixel pipeline for LFB writes
	void pixel_pipeline(thread_stats_block &threadstats, u16 *dest, u16 *depth, s32 x, s32 scry, rgb_t color, u16 sz);

	// update the fog tables
	void write_fog(u32 base, u32 data)
	{
		u32 oldval = m_fogdelta[base + 0] | (m_fogblend[base + 0] << 8) | (m_fogdelta[base + 1] << 16) | (m_fogblend[base + 1] << 24);
		if (oldval != data)
		{
			wait("write_fog");
			m_fogdelta[base + 0] = BIT(data, 0, 8);
			m_fogblend[base + 0] = BIT(data, 8, 8);
			m_fogdelta[base + 1] = BIT(data, 16, 8);
			m_fogblend[base + 1] = BIT(data, 24, 8);
		}
	}

	// update the Y origin
	void set_yorigin(s32 yorigin)
	{
		if (m_yorigin != yorigin)
			wait("set_yorigin");
		m_yorigin = yorigin;
	}

	// update the rowpixels
	void set_rowpixels(u32 rowpixels)
	{
		if (m_rowpixels != rowpixels)
			wait("set_rowpixels");
		m_rowpixels = rowpixels;
	}

	// manage texture instances
	rasterizer_texture &alloc_texture(int tmu) { return m_textures.next(tmu); }
	rasterizer_texture &last_texture(int tmu) { return m_textures.last(tmu); }

	// manage ncc texel instnaces
	rasterizer_palette &alloc_palette(int which) { return m_palettes.next(which); }
	rasterizer_palette &last_palette(int which) { return m_palettes.last(which); }

	// dump rasterizer statistics if enabled
	void dump_rasterizer_stats();

private:
	// pipeline stages, in order
	bool stipple_test(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, s32 x, s32 y, u32 &stipple);
	s32 compute_depthval(voodoo::poly_data const &extra, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fbz_colorpath const fbzcp, s32 wfloat, s32 iterz);
	bool depth_test(thread_stats_block &stats, voodoo::reg_fbz_mode const fbzmode, s32 depth_dest, s32 depth_source);
	bool combine_color(rgbaint_t &color, thread_stats_block &threadstats, const voodoo::poly_data &extradata, voodoo::reg_fbz_colorpath const fbzcp, voodoo::reg_fbz_mode const fbzmode, rgbaint_t texel, s32 iterz, s64 iterw, rgb_t chromakey);
	bool alpha_mask_test(thread_stats_block &stats, u32 alpha);
	bool alpha_test(thread_stats_block &stats, voodoo::reg_alpha_mode const alphamode, u32 alpha, u32 alpharef);
	bool chroma_key_test(thread_stats_block &stats, rgbaint_t const &colorin, rgb_t chromakey);
	void apply_fogging(rgbaint_t &color, rgb_t fogcolor, u32 depthbias, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fog_mode const fogmode, voodoo::reg_fbz_colorpath const fbzcp, s32 x, voodoo::dither_helper const &dither, s32 wfloat, s32 iterz, s64 iterw, const rgbaint_t &iterargb);
	void alpha_blend(rgbaint_t &color, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_alpha_mode const alphamode, s32 x, voodoo::dither_helper const &dither, int dpix, u16 *depth, rgbaint_t const &prefog);
	void write_pixel(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, voodoo::dither_helper const &dither, u16 *destbase, u16 *depthbase, s32 x, rgbaint_t const &color, s32 depthval);

	// fastfill rasterizer
	void rasterizer_fastfill(s32 scanline, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_data &extradata, int threadid);

	// helpers
	static rasterizer_mfp generic_rasterizer(u8 texmask);
	voodoo::rasterizer_info *add_rasterizer(voodoo::rasterizer_params const &params, rasterizer_mfp rasterizer, bool is_generic);

	// internal state
	u8 m_bilinear_mask;         // mask for bilinear resolution (0xf0 for V1, 0xff for V2)
	u16 m_tmu_config;           // TMU configuration
	u32 m_rowpixels;            // current pixels per row
	s32 m_yorigin;              // current Y origin
	voodoo_regs &m_fbi_reg;     // FBI registers
	voodoo_regs *m_tmu0_reg;    // TMU #0 registers
	voodoo_regs *m_tmu1_reg;    // TMU #1 register
	rgb_t const *m_rgb565;      // 5-6-5 to 8-8-8 lookup table
	u8 m_fogblend[64];          // 64-entry fog table
	u8 m_fogdelta[64];          // 64-entry fog table
	u8 m_fogdelta_mask;         // mask for for delta (0xff for V1, 0xfc for V2)
	poly_array<voodoo::rasterizer_texture, 2> m_textures;
	poly_array<voodoo::rasterizer_palette, 8> m_palettes;
	voodoo::rasterizer_info *m_raster_hash[RASTER_HASH_SIZE]; // hash table of rasterizers
	voodoo::rasterizer_info *m_generic_rasterizer[16];
	std::list<voodoo::rasterizer_info> m_rasterizer_list;
	std::vector<thread_stats_block> m_thread_stats;
};

}

#endif // MAME_VIDEO_VOODOO_RENDER_H
