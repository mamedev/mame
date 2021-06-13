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



//**************************************************************************
//  CONSTANTS
//**************************************************************************

/* maximum number of rasterizers */
#define MAX_RASTERIZERS         1024

/* size of the rasterizer hash table */
#define RASTER_HASH_SIZE        97


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

namespace voodoo
{

class dither_helper;
class stw_helper;

struct raster_params
{
	static constexpr u32 rotate(u32 value, int count)
	{
		return (value << count) | (value >> (32 - count));
	}

	void compute(u8 type, int texcount, voodoo_regs &regs, voodoo_regs &tmu0regs, voodoo_regs &tmu1regs)
	{
		m_fbzcp = regs.fbz_colorpath().normalize();
		m_fbzmode = regs.fbz_mode().normalize();
		m_alphamode = regs.alpha_mode().normalize();
		m_fogmode = regs.fog_mode().normalize() | (int(type >= TYPE_VOODOO_2) << 31);
		m_texmode0 = (texcount >= 1 && tmu0regs.texture_lod().lod_min() < 32) ? tmu0regs.texture_mode().normalize() : 0xffffffff;
		m_texmode1 = (texcount >= 2 && tmu1regs.texture_lod().lod_min() < 32) ? tmu1regs.texture_mode().normalize() : 0xffffffff;
	}

	u32 hash() const
	{
		u32 result = (m_alphamode << 0) ^
					 (m_fbzmode << 1) ^
					 (m_fbzcp << 2) ^
					 (m_fogmode << 24) ^
					 rotate(m_texmode0, 8) ^
					 rotate(m_texmode1, 16);
		return result % RASTER_HASH_SIZE;
	}

	bool operator==(raster_params const &rhs) const
	{
		return (m_fbzcp == rhs.m_fbzcp &&
				m_fbzmode == rhs.m_fbzmode &&
				m_alphamode == rhs.m_alphamode &&
				m_fogmode == rhs.m_fogmode &&
				m_texmode0 == rhs.m_texmode0 &&
				m_texmode1 == rhs.m_texmode1);
	}

	u32 m_fbzcp;		// 30 bits
	u32 m_alphamode;	// 32 bits
	u32 m_fogmode;		// 8 bits
	u32 m_fbzmode;		// 22 bits
	u32 m_texmode0;		// 31 bits
	u32 m_texmode1;		// 31 bits
};

class raster_texture
{
public:
	void recompute(u32 type, voodoo_regs const &regs, u8 *ram, u32 mask, rgb_t const *lookup);

	rgb_t lookup_single_texel(voodoo::reg_texture_mode const texmode, u32 texbase, s32 s, s32 t);
	rgbaint_t fetch_texel(voodoo::reg_texture_mode const texmode, voodoo::dither_helper const &dither, s32 x, const voodoo::stw_helper &iterstw, s32 lodbase, s32 &lod);
	rgbaint_t combine_texture(voodoo::reg_texture_mode const texmode, rgbaint_t const &c_local, rgbaint_t const &c_other, s32 lod);

	u8 *m_ram = nullptr;          // pointer to our RAM
	u32 m_mask = 0;               // mask to apply to pointers
	s32 m_lodmin = 0, m_lodmax = 0; // min, max LOD values
	s32 m_lodbias = 0;            // LOD bias
	u32 m_lodmask = 0;            // mask of available LODs
	u32 m_lodoffset[9];           // offset of texture base for each LOD
	s32 m_detailmax = 0;          // detail clamp
	s32 m_detailbias = 0;         // detail bias
	u8 m_detailscale = 0;        // detail scale

	u32 m_wmask = 0;              // mask for the current texture width
	u32 m_hmask = 0;              // mask for the current texture height

	u32 m_bilinear_mask = 0;      // mask for bilinear resolution (0xf0 for V1, 0xff for V2)
	rgb_t const *m_lookup = nullptr;       // currently selected lookup
};

/* note that this structure is an even 64 bytes long */
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
	s32 filler[64/4 - 7];       // pad this structure to 64 bytes
};

struct raster_info;
struct poly_extra_data
{
	raster_info *info;          // pointer to rasterizer information
	u16 *destbase;              // destination to write
	u16 *depthbase;             // destination to write
	raster_texture *tex0;
	raster_texture *tex1;

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

	union
	{
		raster_params raster;   // normalized rasterizer parameters, for triangles
		u16 dither[16];         // dither matrix, for fastfill
	} u;
};

class voodoo_renderer : public poly_manager<float, poly_extra_data, 1, 10000>
{
public:
	voodoo_renderer(running_machine &machine, u8 type, voodoo_regs &regs, const rgb_t *rgb565) :
		poly_manager(machine),
		m_reg(regs),
		m_type(type),
		m_rowpixels(0),
		m_yorigin(0),
		m_rgb565(rgb565),
		m_fogdelta_mask((type < TYPE_VOODOO_2) ? 0xff : 0xfc),
		m_thread_stats(WORK_MAX_THREADS) { }

	using mfp = void (voodoo_renderer::*)(s32, const extent_t &, const poly_extra_data &, int);

	void pixel_pipeline(thread_stats_block &threadstats, voodoo::poly_extra_data const &extra, voodoo::reg_lfb_mode const lfbmode, s32 x, s32 scry, rgb_t color, u16 sz);

	void raster_fastfill(s32 scanline, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_extra_data &extradata, int threadid);
	template<u32 _FbzCp, u32 _FbzMode, u32 _AlphaMode, u32 _FogMode, u32 _TexMode0, u32 _TexMode1>
	void rasterizer(s32 y, const voodoo::voodoo_renderer::extent_t &extent, const voodoo::poly_extra_data &extra, int threadid);

	bool stipple_test(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, s32 x, s32 y);
	s32 compute_wfloat(s64 iterw);
	s32 compute_depthval(voodoo::poly_extra_data const &extra, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fbz_colorpath const fbzcp, s32 wfloat, s32 iterz);
	bool depth_test(thread_stats_block &stats, voodoo::poly_extra_data const &extra, voodoo::reg_fbz_mode const fbzmode, s32 destDepth, s32 biasdepth);
	bool alpha_mask_test(thread_stats_block &stats, u8 alpha);
	bool chroma_key_test(thread_stats_block &stats, rgbaint_t const &rgaIntColor);
	bool combine_color(rgbaint_t &color, thread_stats_block &threadstats, const voodoo::poly_extra_data &extradata, voodoo::reg_fbz_colorpath const fbzcp, voodoo::reg_fbz_mode const fbzmode, rgbaint_t texel, s32 iterz, s64 iterw);
	bool alpha_test(thread_stats_block &stats, voodoo::reg_alpha_mode const alphamode, u8 alpha);
	void apply_fogging(rgbaint_t &color, voodoo::poly_extra_data const &extra, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_fog_mode const fogmode, voodoo::reg_fbz_colorpath const fbzcp, s32 x, voodoo::dither_helper const &dither, s32 wfloat, s32 iterz, s64 iterw, const rgbaint_t &iterargb);
	void alpha_blend(rgbaint_t &color, voodoo::reg_fbz_mode const fbzmode, voodoo::reg_alpha_mode const alphamode, s32 x, voodoo::dither_helper const &dither, int dpix, u16 *depth, rgbaint_t const &prefog);
	void write_pixel(thread_stats_block &threadstats, voodoo::reg_fbz_mode const fbzmode, voodoo::dither_helper const &dither, u16 *destbase, u16 *depthbase, s32 x, rgbaint_t const &color, s32 depthval);

	void write_fog(u32 base, u32 data)
	{
		wait("Fog write");
		m_fogdelta[base + 0] = (data >> 0) & 0xff;
		m_fogblend[base + 0] = (data >> 8) & 0xff;
		m_fogdelta[base + 1] = (data >> 16) & 0xff;
		m_fogblend[base + 1] = (data >> 24) & 0xff;
	}

	void set_yorigin(s32 yorigin)
	{
		wait("Y origin write");
		m_yorigin = yorigin;
	}

	void set_rowpixels(u32 rowpixels)
	{
		wait("Rowpixels write");
		m_rowpixels = rowpixels;
	}

	std::vector<thread_stats_block> &thread_stats() { return m_thread_stats; }

//private:
	voodoo_regs &m_reg;
	u8 m_type;
	u32 m_rowpixels;
	s32 m_yorigin;
	rgb_t const *m_rgb565;
	u8 m_fogblend[64];           // 64-entry fog table */
	u8 m_fogdelta[64];           // 64-entry fog table */
	u8 m_fogdelta_mask;          // mask for for delta (0xff for V1, 0xfc for V2) */
	std::vector<thread_stats_block> m_thread_stats;
};

struct static_raster_info
{
	voodoo_renderer::mfp callback_mfp;
	raster_params params;
};

struct raster_info
{
	raster_info *next = nullptr;         // pointer to next entry with the same hash
	voodoo_renderer::render_delegate callback; // callback pointer
	u8 display;                // display index
	u8 is_generic;                // display index
	u32 hits;                   // how many hits (pixels) we've used this for
	u32 polys;                  // how many polys we've used this for
	u32 hash = 0U;
	raster_params params;
};


//**************************************************************************
//  DITHER HELPER
//**************************************************************************

class dither_helper
{
public:
	// constructor to pre-cache based on mode and Y coordinate
	dither_helper(int y, reg_fbz_mode const fbzmode, reg_fog_mode const fogmode = reg_fog_mode(0)) :
		m_dither_lookup(nullptr),
		m_dither_raw_4x4(&s_dither_matrix_4x4[(y & 3) * 4]),
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
	u8 const *m_dither_raw_4x4;
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
