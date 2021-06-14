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

#define COPY_TEXTURE_DATA (0)

//
// To do:
//   - incorporate type and send_config flags into constant flags
//   - use type for: fog delta mask, bilinear mask
//   - multiple rasterizer_textures to save stalls
//   - leverage poly clipping?
//

namespace voodoo
{

// forward declarations
struct rasterizer_info;
struct poly_data;
class dither_helper;
class stw_helper;

// base class for our renderer
using voodoo_poly_manager = poly_manager<float, poly_data, 1, 10000>;


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
		return (m_dither_raw != nullptr) ? (15 - (m_dither_raw[x & 3] >> 1)) : 0;
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


// ======================> rasterizer_params

// this class holds the representative parameters that characterize a
// specific rasterizer; these are used to index and discover one of
// the special hard-coded rasterizers in voodoo_render.cpp
class rasterizer_params
{
public:
	// construction
	constexpr rasterizer_params(u32 fbzcp = 0, u32 alphamode = 0, u32 fogmode = 0, u32 fbzmode = 0, u32 texmode0 = 0, u32 texmode1 = 0) :
		m_fbzcp(fbzcp),
		m_alphamode(alphamode),
		m_fogmode(fogmode),
		m_fbzmode(fbzmode),
		m_texmode0(texmode0),
		m_texmode1(texmode1) { }

	// compare everything directly
	bool operator==(rasterizer_params const &rhs) const;

	// compute the parameters given a set of registers
	void compute(u8 type, voodoo_regs &regs, voodoo_regs *tmu0regs = nullptr, voodoo_regs *tmu1regs = nullptr);

	// compute the hash of the settings
	u32 hash() const;

	// getters
	reg_fbz_colorpath fbzcp() const { return reg_fbz_colorpath(m_fbzcp); }
	reg_alpha_mode alphamode() const { return reg_alpha_mode(m_alphamode); }
	reg_fog_mode fogmode() const { return reg_fog_mode(m_fogmode); }
	reg_fbz_mode fbzmode() const { return reg_fbz_mode(m_fbzmode); }
	reg_texture_mode texmode0() const { return reg_texture_mode(m_texmode0); }
	reg_texture_mode texmode1() const { return reg_texture_mode(m_texmode1); }

private:
	// internal helpers
	static constexpr u32 rotate(u32 value, int count) { return (value << count) | (value >> (32 - count)); }

	// internal state
	u32 m_fbzcp;		// 30 bits
	u32 m_alphamode;	// 32 bits
	u32 m_fogmode;		// 8 bits
	u32 m_fbzmode;		// 22 bits
	u32 m_texmode0;		// 31 bits
	u32 m_texmode1;		// 31 bits
};


// ======================> rasterizer_texture

// this class holds TMU-specific decoded data regarding a texture; it is
// encapsulated here, with functions to derive it from register info and
// functions to process it as part of the rendering pipeline
class rasterizer_texture
{
public:
	// recompute internal values based on parameters
	void recompute(u8 type, voodoo_regs const &regs, u8 *ram, u32 mask, rgb_t const *lookup);

	// look up a texel at the given coordinate
	rgb_t lookup_single_texel(u32 format, u32 texbase, s32 s, s32 t);

	// fetch a texel given coordinates and LOD information
	rgbaint_t fetch_texel(voodoo::reg_texture_mode const texmode, voodoo::dither_helper const &dither, s32 x, const voodoo::stw_helper &iterstw, s32 lodbase, s32 &lod);

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
	u8 m_bilinear_mask;         // mask for bilinear resolution (0xf0 for V1, 0xff for V2)
	s16 m_lodmin;               // minimum LOD value
	s16 m_lodmax;               // maximum LOD value
	s16 m_lodbias;              // LOD bias
	u16 m_lodmask;              // mask of available LODs
	u32 m_mask;                 // mask to apply to pointers
	s32 m_detailmax;            // detail clamp
	s32 m_detailbias;           // detail bias
	u32 m_lodoffset[9];         // offset of texture base for each LOD
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
	s32 lodbase0;               // used during rasterization

	s64 starts1, startt1;       // starting S,T (14.18)
	s64 startw1;                // starting W (2.30)
	s64 ds1dx, dt1dx;           // delta S,T per X
	s64 dw1dx;                  // delta W per X
	s64 ds1dy, dt1dy;           // delta S,T per Y
	s64 dw1dy;                  // delta W per Y
	s32 lodbase1;               // used during rasterization

	rgb_t color0, color1;       // colors consumed by the rasterizer
	u32 zacolor;                // depth/alpha value consumed by the rasterizer

	u16 dither[16];             // dither matrix, for fastfill

#if COPY_TEXTURE_DATA
	rasterizer_texture tex0copy;// texture 0 information
	rasterizer_texture tex1copy;// texture 1 information
#endif
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
	s32 filler[voodoo_poly_manager::CACHE_LINE_SIZE/4 - 7]; // pad this structure to cache line size
};


// ======================> rasterizer_info

class voodoo_renderer : public voodoo_poly_manager
{
	static constexpr u32 RASTER_HASH_SIZE = 97;	// size of the rasterizer hash table

public:
	using rasterizer_mfp = void (voodoo_renderer::*)(int32_t, const extent_t &, const poly_data &, int);

	// construction
	voodoo_renderer(running_machine &machine, u8 type, const rgb_t *rgb565, voodoo_regs &fbi_regs, voodoo_regs *tmu0_regs, voodoo_regs *tmu1_regs);

	// simple getters
	std::vector<thread_stats_block> &thread_stats() { return m_thread_stats; }

	// allocate a new poly_data and fill in the rasterizer_params
	poly_data &alloc_poly();

	// enqueue operations
	u32 enqueue_fastfill(poly_data &poly);
	u32 enqueue_triangle(poly_data &poly, vertex_t const *vert);

	// core triangle rasterizer
	template<u32 _FbzCp, u32 _FbzMode, u32 _AlphaMode, u32 _FogMode, u32 _TexMode0, u32 _TexMode1>
	void rasterizer(s32 y, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_data &extra, int threadid);

	// run the pixel pipeline for LFB writes
	void pixel_pipeline(thread_stats_block &threadstats, voodoo::poly_data const &extra, voodoo::reg_lfb_mode const lfbmode, s32 x, s32 scry, rgb_t color, u16 sz);

	// update the fog tables
	void write_fog(u32 base, u32 data)
	{
		wait("Fog write");
		m_fogdelta[base + 0] = (data >> 0) & 0xff;
		m_fogblend[base + 0] = (data >> 8) & 0xff;
		m_fogdelta[base + 1] = (data >> 16) & 0xff;
		m_fogblend[base + 1] = (data >> 24) & 0xff;
	}

	// update the Y origin
	void set_yorigin(s32 yorigin)
	{
		wait("Y origin write");
		m_yorigin = yorigin;
	}

	// update the rowpixels
	void set_rowpixels(u32 rowpixels)
	{
		wait("Rowpixels write");
		m_rowpixels = rowpixels;
	}

	// dump rasterizer statistics if enabled
	void dump_rasterizer_stats();

private:
	// pipeline stages, in order
	bool stipple_test(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, s32 x, s32 y);
	s32 compute_depthval(voodoo::poly_data const &extra, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fbz_colorpath const fbzcp, s32 wfloat, s32 iterz);
	bool depth_test(thread_stats_block &stats, voodoo::poly_data const &extra, voodoo::reg_fbz_mode const fbzmode, s32 destDepth, s32 biasdepth);
	bool combine_color(rgbaint_t &color, thread_stats_block &threadstats, const voodoo::poly_data &extradata, voodoo::reg_fbz_colorpath const fbzcp, voodoo::reg_fbz_mode const fbzmode, rgbaint_t texel, s32 iterz, s64 iterw);
	bool alpha_mask_test(thread_stats_block &stats, u32 alpha);
	bool alpha_test(thread_stats_block &stats, voodoo::reg_alpha_mode const alphamode, u32 alpha, u32 alpharef);
	bool chroma_key_test(thread_stats_block &stats, rgbaint_t const &colorin);
	void apply_fogging(rgbaint_t &color, voodoo::poly_data const &extra, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fog_mode const fogmode, voodoo::reg_fbz_colorpath const fbzcp, s32 x, voodoo::dither_helper const &dither, s32 wfloat, s32 iterz, s64 iterw, const rgbaint_t &iterargb);
	void alpha_blend(rgbaint_t &color, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_alpha_mode const alphamode, s32 x, voodoo::dither_helper const &dither, int dpix, u16 *depth, rgbaint_t const &prefog);
	void write_pixel(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, voodoo::dither_helper const &dither, u16 *destbase, u16 *depthbase, s32 x, rgbaint_t const &color, s32 depthval);

	// fastfill rasterizer
	void rasterizer_fastfill(s32 scanline, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_data &extradata, int threadid);

	// helpers
	static rasterizer_mfp generic_rasterizer(u8 texmask);
	voodoo::rasterizer_info *add_rasterizer(voodoo::rasterizer_params const &params, rasterizer_mfp rasterizer, bool is_generic);

	// internal state
	u8 m_type;                  // type of chip
	u32 m_rowpixels;            // current pixels per row
	s32 m_yorigin;              // current Y origin
	voodoo_regs &m_fbi_reg;     // FBI registers
	voodoo_regs *m_tmu0_reg;    // TMU #0 registers
	voodoo_regs *m_tmu1_reg;    // TMU #1 register
	rgb_t const *m_rgb565;      // 5-6-5 to 8-8-8 lookup table
	u8 m_fogblend[64];          // 64-entry fog table
	u8 m_fogdelta[64];          // 64-entry fog table
	u8 m_fogdelta_mask;         // mask for for delta (0xff for V1, 0xfc for V2)
	voodoo::rasterizer_info *m_raster_hash[RASTER_HASH_SIZE]; // hash table of rasterizers
	voodoo::rasterizer_info *m_generic_rasterizer[4];
	std::list<voodoo::rasterizer_info> m_rasterizer_list;
	std::vector<thread_stats_block> m_thread_stats;
};



//**************************************************************************
//  MATH HELPERS
//**************************************************************************

//-------------------------------------------------
//  fast_log2 - computes the log2 of a double-
//  precision value as a 24.8 value
//-------------------------------------------------

inline s32 fast_log2(double value, int offset)
{
	// negative values return 0
	if (UNEXPECTED(value < 0))
		return 0;

	// convert the value to a raw integer
	union { double d; u64 i; } temp;
	temp.d = value;

	// we only care about the 11-bit exponent and top 4 bits of mantissa
	// (sign is already assured to be 0)
	u32 ival = temp.i >> 48;

	// exponent in the upper bits, plus an 8-bit log value from 4 bits of mantissa
	s32 exp = (ival >> 4) - 1023 + 32 - offset;

	// the maximum error using a 4 bit lookup from the mantissa is 0.0875, which is
	// less than 1/2 lsb (0.125) for 2 bits of fraction
	static u8 const s_log2_table[16] = { 0, 22, 44, 63, 82, 100, 118, 134, 150, 165, 179, 193, 207, 220, 232, 244 };
	return (exp << 8) | s_log2_table[ival & 15];
}

}

#endif // MAME_VIDEO_VOODOO_RENDER_H
