// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef _VIDEO_N64_H_
#define _VIDEO_N64_H_

#include "emu.h"
#include "includes/n64.h"
#include "video/poly.h"
#include "video/rdpblend.h"
#include "video/rdptpipe.h"

/*****************************************************************************/

#define PIXEL_SIZE_4BIT         0
#define PIXEL_SIZE_8BIT         1
#define PIXEL_SIZE_16BIT        2
#define PIXEL_SIZE_32BIT        3

#define CYCLE_TYPE_1            0
#define CYCLE_TYPE_2            1
#define CYCLE_TYPE_COPY         2
#define CYCLE_TYPE_FILL         3

#define SAMPLE_TYPE_1x1         0
#define SAMPLE_TYPE_2x2         1

#define BYTE_ADDR_XOR       BYTE4_XOR_BE(0)
#define WORD_ADDR_XOR       (WORD_XOR_BE(0) >> 1)

#define XOR_SWAP_BYTE_SHIFT     2
#define XOR_SWAP_WORD_SHIFT     1
#define XOR_SWAP_DWORD_SHIFT    0

#define XOR_SWAP_BYTE   4
#define XOR_SWAP_WORD   2
#define XOR_SWAP_DWORD  1

#define FORMAT_RGBA             0
#define FORMAT_YUV              1
#define FORMAT_CI               2
#define FORMAT_IA               3
#define FORMAT_I                4

#ifdef LSB_FIRST
#define BYTE_XOR_DWORD_SWAP 7
#define WORD_XOR_DWORD_SWAP 3
#else
#define BYTE_XOR_DWORD_SWAP 4
#define WORD_XOR_DWORD_SWAP 2
#endif
#define DWORD_XOR_DWORD_SWAP 1

#define GET_LOW_RGBA16_TMEM(x)  (m_rdp->m_replicated_rgba[((x) >> 1) & 0x1f])
#define GET_MED_RGBA16_TMEM(x)  (m_rdp->m_replicated_rgba[((x) >> 6) & 0x1f])
#define GET_HI_RGBA16_TMEM(x)   (m_rdp->m_replicated_rgba[((x) >> 11) & 0x1f])

#define MEM8_LIMIT  0x7fffff
#define MEM16_LIMIT 0x3fffff
#define MEM32_LIMIT 0x1fffff

#define RDP_RANGE_CHECK (0)

#if RDP_RANGE_CHECK
#define CHECK8(in) if(rdp_range_check((in))) { printf("Check8: Address %08x out of range!\n", (in)); fflush(stdout); fatalerror("Address %08x out of range!\n", (in)); }
#define CHECK16(in) if(rdp_range_check((in) << 1)) { printf("Check16: Address %08x out of range!\n", (in) << 1); fflush(stdout); fatalerror("Address %08x out of range!\n", (in) << 1); }
#define CHECK32(in) if(rdp_range_check((in) << 2)) { printf("Check32: Address %08x out of range!\n", (in) << 2); fflush(stdout); fatalerror("Address %08x out of range!\n", (in) << 2); }
#else
#define CHECK8(in) { }
#define CHECK16(in) { }
#define CHECK32(in) { }
#endif

#if RDP_RANGE_CHECK
#define RREADADDR8(in) ((rdp_range_check((in))) ? 0 : (((UINT8*)rdram)[(in) ^ BYTE_ADDR_XOR]))
#define RREADIDX16(in) ((rdp_range_check((in) << 1)) ? 0 : (((UINT16*)rdram)[(in) ^ WORD_ADDR_XOR]))
#define RREADIDX32(in) ((rdp_range_check((in) << 2)) ? 0 : rdram[(in)])

#define RWRITEADDR8(in, val)    if(rdp_range_check((in))) { printf("Write8: Address %08x out of range!\n", (in)); fflush(stdout); fatalerror("Address %08x out of range!\n", (in)); } else { ((UINT8*)rdram)[(in) ^ BYTE_ADDR_XOR] = val;}
#define RWRITEIDX16(in, val)    if(rdp_range_check((in) << 1)) { printf("Write16: Address %08x out of range!\n", ((object.m_misc_state.m_fb_address >> 1) + curpixel) << 1); fflush(stdout); fatalerror("Address out of range\n"); } else { ((UINT16*)rdram)[(in) ^ WORD_ADDR_XOR] = val;}
#define RWRITEIDX32(in, val)    if(rdp_range_check((in) << 2)) { printf("Write32: Address %08x out of range!\n", (in) << 2); fflush(stdout); fatalerror("Address %08x out of range!\n", (in) << 2); } else { rdram[(in)] = val;}
#else
#define RREADADDR8(in) (((UINT8*)rdram)[(in) ^ BYTE_ADDR_XOR])
#define RREADIDX16(in) (((UINT16*)rdram)[(in) ^ WORD_ADDR_XOR])
#define RREADIDX32(in) (rdram[(in)])

#define RWRITEADDR8(in, val)    ((UINT8*)rdram)[(in) ^ BYTE_ADDR_XOR] = val;
#define RWRITEIDX16(in, val)    ((UINT16*)rdram)[(in) ^ WORD_ADDR_XOR] = val;
#define RWRITEIDX32(in, val)    rdram[(in)] = val
#endif

#define U_RREADADDR8(in) (((UINT8*)rdram)[(in) ^ BYTE_ADDR_XOR])
#define U_RREADIDX16(in) (((UINT16*)rdram)[(in) ^ WORD_ADDR_XOR])
#define U_RREADIDX32(in) (rdram[(in)])

#define GETLOWCOL(x)    (((x) & 0x3e) << 2)
#define GETMEDCOL(x)    (((x) & 0x7c0) >> 3)
#define GETHICOL(x)     (((x) & 0xf800) >> 8)

#define HREADADDR8(in)          /*(((in) <= MEM8_LIMIT) ? */(m_hidden_bits[(in) ^ BYTE_ADDR_XOR])/* : 0)*/
#define HWRITEADDR8(in, val)    /*{if ((in) <= MEM8_LIMIT) */m_hidden_bits[(in) ^ BYTE_ADDR_XOR] = val;/*}*/

//sign-extension macros
#define SIGN22(x)   (((x & 0x00200000) * 0x7ff) | (x & 0x1fffff))
#define SIGN17(x)   (((x & 0x00010000) * 0xffff) | (x & 0xffff))
#define SIGN16(x)   (((x & 0x00008000) * 0x1ffff) | (x & 0x7fff))
#define SIGN13(x)   (((x & 0x00001000) * 0xfffff) | (x & 0xfff))
#define SIGN11(x)   (((x & 0x00000400) * 0x3fffff) | (x & 0x3ff))
#define SIGN9(x)    (((x & 0x00000100) * 0xffffff) | (x & 0xff))
#define SIGN8(x)    (((x & 0x00000080) * 0x1ffffff) | (x & 0x7f))

#define KURT_AKELEY_SIGN9(x)    ((((x) & 0x180) == 0x180) ? ((x) | ~0x1ff) : ((x) & 0x1ff))

#define SPAN_R      (0)
#define SPAN_G      (1)
#define SPAN_B      (2)
#define SPAN_A      (3)
#define SPAN_S      (4)
#define SPAN_T      (5)
#define SPAN_W      (6)
#define SPAN_Z      (7)

#define RDP_CVG_SPAN_MAX            (1024)

#define EXTENT_AUX_COUNT            (sizeof(rdp_span_aux)*(480*192)) // Screen coverage *192, more or less

/*****************************************************************************/

class n64_periphs;
class n64_rdp;

struct misc_state_t;
struct other_modes_t;
struct combine_mdoes_t;
struct color_inputs_t;
struct span_base_t;
struct rectangle_t;

class color_t
{
	public:
		color_t()
		{
			c = 0;
		}

		union
		{
			UINT32 c;
#ifdef LSB_FIRST
			struct { UINT8 a, b, g, r; } i;
#else
			struct { UINT8 r, g, b, a; } i;
#endif
		};
};

enum
{
	BIT_DEPTH_32 = 0,
	BIT_DEPTH_16,

	BIT_DEPTH_COUNT
};

class SpanParam
{
	public:
		union
		{
			UINT32 w;
#ifdef LSB_FIRST
			struct { UINT16 l; INT16 h; } h;
#else
			struct { INT16 h; UINT16 l; } h;
#endif
		};
};

struct n64_tile_t
{
	INT32 format; // Image data format: RGBA, YUV, CI, IA, I
	INT32 size; // Size of texel element: 4b, 8b, 16b, 32b
	INT32 line; // Size of tile line in bytes
	INT32 tmem; // Starting tmem address for this tile in bytes
	INT32 palette; // Palette number for 4b CI texels
	INT32 ct, mt, cs, ms; // Clamp / mirror enable bits for S / T direction
	INT32 mask_t, shift_t, mask_s, shift_s; // Mask values / LOD shifts
	INT32 lshift_s, rshift_s, lshift_t, rshift_t;
	INT32 wrapped_mask_s, wrapped_mask_t;
	bool clamp_s, clamp_t;
	UINT16 sl, tl, sh, th;      // 10.2 fixed-point, starting and ending texel row / column
	INT32 num;
};

struct span_base_t
{
	INT32 m_span_dr;
	INT32 m_span_dg;
	INT32 m_span_db;
	INT32 m_span_da;
	INT32 m_span_ds;
	INT32 m_span_dt;
	INT32 m_span_dw;
	INT32 m_span_dz;
	INT32 m_span_dymax;
	INT32 m_span_dzpix;
	INT32 m_span_drdy;
	INT32 m_span_dgdy;
	INT32 m_span_dbdy;
	INT32 m_span_dady;
	INT32 m_span_dzdy;
};

struct misc_state_t
{
	misc_state_t()
	{
		m_max_level = 0;
		m_min_level = 0;
	}

	INT32 m_fb_format;          // Framebuffer pixel format index (0 - I, 1 - IA, 2 - CI, 3 - RGBA)
	INT32 m_fb_size;            // Framebuffer pixel size index (0 - 4bpp, 1 - 8bpp, 2 - 16bpp, 3 - 32bpp)
	INT32 m_fb_width;           // Framebuffer width, in pixels
	INT32 m_fb_height;          // Framebuffer height, in pixels
	UINT32 m_fb_address;        // Framebuffer source address offset (in bytes) from start of RDRAM

	UINT32 m_zb_address;        // Z-buffer source address offset (in bytes) from start of RDRAM

	INT32 m_ti_format;          // Format for Texture Interface (TI) transfers
	INT32 m_ti_size;            // Size (in bytes) of TI transfers
	INT32 m_ti_width;           // Width (in pixels) of TI transfers
	UINT32 m_ti_address;        // Destination address for TI transfers

	UINT8 m_random_seed;        // %HACK%, adds 19 each time it's read and is more or less random

	UINT32 m_max_level;         // Maximum LOD level for texture filtering
	UINT32 m_min_level;         // Minimum LOD level for texture filtering

	UINT16 m_primitive_z;       // Forced Z value for current primitive, if applicable
	UINT16 m_primitive_dz;      // Forced Delta-Z value for current primitive, if applicable
};

struct combine_modes_t
{
	INT32 sub_a_rgb0;
	INT32 sub_b_rgb0;
	INT32 mul_rgb0;
	INT32 add_rgb0;
	INT32 sub_a_a0;
	INT32 sub_b_a0;
	INT32 mul_a0;
	INT32 add_a0;

	INT32 sub_a_rgb1;
	INT32 sub_b_rgb1;
	INT32 mul_rgb1;
	INT32 add_rgb1;
	INT32 sub_a_a1;
	INT32 sub_b_a1;
	INT32 mul_a1;
	INT32 add_a1;
};

struct other_modes_t
{
	INT32 cycle_type;
	bool persp_tex_en;
	bool detail_tex_en;
	bool sharpen_tex_en;
	bool tex_lod_en;
	bool en_tlut;
	bool tlut_type;
	bool sample_type;
	bool mid_texel;
	bool bi_lerp0;
	bool bi_lerp1;
	bool convert_one;
	bool key_en;
	INT32 rgb_dither_sel;
	INT32 alpha_dither_sel;
	INT32 blend_m1a_0;
	INT32 blend_m1a_1;
	INT32 blend_m1b_0;
	INT32 blend_m1b_1;
	INT32 blend_m2a_0;
	INT32 blend_m2a_1;
	INT32 blend_m2b_0;
	INT32 blend_m2b_1;
	INT32 tex_edge;
	INT32 force_blend;
	INT32 blend_shift;
	bool alpha_cvg_select;
	bool cvg_times_alpha;
	INT32 z_mode;
	INT32 cvg_dest;
	bool color_on_cvg;
	UINT8 image_read_en;
	bool z_update_en;
	bool z_compare_en;
	bool antialias_en;
	bool z_source_sel;
	INT32 dither_alpha_en;
	INT32 alpha_compare_en;
	INT32 alpha_dither_mode;
};

struct color_inputs_t
{
	// combiner inputs
	UINT8* combiner_rgbsub_a_r[2];
	UINT8* combiner_rgbsub_a_g[2];
	UINT8* combiner_rgbsub_a_b[2];
	UINT8* combiner_rgbsub_b_r[2];
	UINT8* combiner_rgbsub_b_g[2];
	UINT8* combiner_rgbsub_b_b[2];
	UINT8* combiner_rgbmul_r[2];
	UINT8* combiner_rgbmul_g[2];
	UINT8* combiner_rgbmul_b[2];
	UINT8* combiner_rgbadd_r[2];
	UINT8* combiner_rgbadd_g[2];
	UINT8* combiner_rgbadd_b[2];

	UINT8* combiner_alphasub_a[2];
	UINT8* combiner_alphasub_b[2];
	UINT8* combiner_alphamul[2];
	UINT8* combiner_alphaadd[2];

	// blender input
	UINT8* blender1a_r[2];
	UINT8* blender1a_g[2];
	UINT8* blender1a_b[2];
	UINT8* blender1b_a[2];
	UINT8* blender2a_r[2];
	UINT8* blender2a_g[2];
	UINT8* blender2a_b[2];
	UINT8* blender2b_a[2];
};

// This is enormous and horrible
struct rdp_span_aux
{
	UINT32              m_unscissored_rx;
	UINT16              m_cvg[RDP_CVG_SPAN_MAX];
	color_t             m_memory_color;
	color_t             m_pixel_color;
	color_t             m_inv_pixel_color;
	color_t             m_blended_pixel_color;
	color_t             m_combined_color;
	color_t             m_texel0_color;
	color_t             m_texel1_color;
	color_t             m_next_texel_color;
	color_t             m_blend_color;          /* constant blend color */
	color_t             m_prim_color;           /* flat primitive color */
	color_t             m_env_color;            /* generic color constant ('environment') */
	color_t             m_fog_color;            /* generic color constant ('fog') */
	color_t             m_shade_color;          /* gouraud-shaded color */
	color_t             m_key_scale;            /* color-keying constant */
	color_t             m_noise_color;          /* noise */
	UINT8               m_lod_fraction;         /* Z-based LOD fraction for this poly */
	UINT8               m_prim_lod_fraction;    /* fixed LOD fraction for this poly */
	color_inputs_t      m_color_inputs;
	UINT32              m_current_pix_cvg;
	UINT32              m_current_mem_cvg;
	UINT32              m_current_cvg_bit;
	INT32               m_shift_a;
	INT32               m_shift_b;
	INT32               m_precomp_s;
	INT32               m_precomp_t;
	INT32               m_blend_enable;
	bool                m_pre_wrap;
	INT32               m_dzpix_enc;
	UINT8*              m_tmem;                /* pointer to texture cache for this polygon */
	bool                m_start_span;
};

struct rectangle_t
{
	UINT16 m_xl;    // 10.2 fixed-point
	UINT16 m_yl;    // 10.2 fixed-point
	UINT16 m_xh;    // 10.2 fixed-point
	UINT16 m_yh;    // 10.2 fixed-point
};

struct z_decompress_entry_t
{
	UINT32 shift;
	UINT32 add;
};

struct cv_mask_derivative_t
{
	UINT8 cvg;
	UINT8 cvbit;
	UINT8 xoff;
	UINT8 yoff;
};

struct rdp_poly_state
{
	n64_rdp*            m_rdp;                  /* pointer back to the RDP state */

	misc_state_t        m_misc_state;           /* miscellaneous rasterizer bits */
	other_modes_t       m_other_modes;          /* miscellaneous rasterizer bits (2) */
	span_base_t         m_span_base;            /* span initial values for triangle rasterization */
	rectangle_t         m_scissor;              /* screen-space scissor bounds */
	UINT32              m_fill_color;           /* poly fill color */
	n64_tile_t          m_tiles[8];             /* texture tile state */
	UINT8               m_tmem[0x1000];         /* texture cache */
	INT32               tilenum;                /* texture tile index */
	bool                flip;                   /* left-major / right-major flip */
	bool                rect;                   /* primitive is rectangle (vs. triangle) */
};

typedef void (*rdp_command_t)(UINT32 w1, UINT32 w2);

class n64_rdp : public poly_manager<UINT32, rdp_poly_state, 8, 32000>
{
public:
	n64_rdp(n64_state &state);

	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	void init_internal_state()
	{
		m_tmem = auto_alloc_array(machine(), UINT8, 0x1000);
		memset(m_tmem, 0, 0x1000);

		UINT8* normpoint = machine().root_device().memregion("normpoint")->base();
		UINT8* normslope = machine().root_device().memregion("normslope")->base();

		for(INT32 i = 0; i < 64; i++)
		{
			m_norm_point_rom[i] = (normpoint[(i << 1) + 1] << 8) | normpoint[i << 1];
			m_norm_slope_rom[i] = (normslope[(i << 1) + 1] << 8) | normslope[i << 1];
		}

		memset(m_tiles, 0, 8 * sizeof(n64_tile_t));
		memset(m_cmd_data, 0, sizeof(m_cmd_data));
	}

	void        process_command_list();
	UINT32      read_data(UINT32 address);
	void        disassemble(char* buffer);

	void        set_machine(running_machine& machine) { m_machine = &machine; }

	// CPU-visible registers
	void        set_start(UINT32 val) { m_start = val; }
	UINT32      get_start() const { return m_start; }

	void        set_end(UINT32 val) { m_end = val; }
	UINT32      get_end() const { return m_end; }

	void        set_current(UINT32 val) { m_current = val; }
	UINT32      get_current() const { return m_current; }

	void        set_status(UINT32 val) { m_status = val; }
	UINT32      get_status() const { return m_status; }

	// Color Combiner
	INT32       color_combiner_equation(INT32 a, INT32 b, INT32 c, INT32 d);
	INT32       alpha_combiner_equation(INT32 a, INT32 b, INT32 c, INT32 d);
	void        set_suba_input_rgb(UINT8** input_r, UINT8** input_g, UINT8** input_b, INT32 code, rdp_span_aux* userdata);
	void        set_subb_input_rgb(UINT8** input_r, UINT8** input_g, UINT8** input_b, INT32 code, rdp_span_aux* userdata);
	void        set_mul_input_rgb(UINT8** input_r, UINT8** input_g, UINT8** input_b, INT32 code, rdp_span_aux* userdata);
	void        set_add_input_rgb(UINT8** input_r, UINT8** input_g, UINT8** input_b, INT32 code, rdp_span_aux* userdata);
	void        set_sub_input_alpha(UINT8** input, INT32 code, rdp_span_aux* userdata);
	void        set_mul_input_alpha(UINT8** input, INT32 code, rdp_span_aux* userdata);

	// Texture memory
	UINT8*      get_tmem8() { return m_tmem; }
	UINT16*     get_tmem16() { return (UINT16*)m_tmem; }

	// Emulation Accelerators
	UINT8       get_random() { return m_misc_state.m_random_seed += 0x13; }

	// YUV Factors
	void        set_yuv_factors(INT32 k0, INT32 k1, INT32 k2, INT32 k3, INT32 k4, INT32 k5) { m_k0 = k0; m_k1 = k1; m_k2 = k2; m_k3 = k3; m_k4 = k4; m_k5 = k5; }
	INT32       get_k0() const { return m_k0; }
	INT32       get_k1() const { return m_k1; }
	INT32       get_k2() const { return m_k2; }
	INT32       get_k3() const { return m_k3; }
	INT32*      get_k4() { return &m_k4; }
	INT32*      get_k5() { return &m_k5; }

	// Blender-related (move into RDP::Blender)
	void        set_blender_input(INT32 cycle, INT32 which, UINT8** input_r, UINT8** input_g, UINT8** input_b, UINT8** input_a, INT32 a, INT32 b, rdp_span_aux* userdata);

	// Span rasterization
	void        span_draw_1cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid);
	void        span_draw_2cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid);
	void        span_draw_copy(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid);
	void        span_draw_fill(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid);

	// Render-related (move into eventual drawing-related classes?)
	void            tc_div(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst);
	void            tc_div_no_perspective(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst);
	UINT32          get_log2(UINT32 lod_clamp);
	void            render_spans(INT32 start, INT32 end, INT32 tilenum, bool flip, extent_t* spans, bool rect, rdp_poly_state* object);
	void            get_alpha_cvg(UINT8* comb_alpha, rdp_span_aux* userdata, const rdp_poly_state &object);

	void            z_store(const rdp_poly_state &object, UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 z, UINT32 enc);
	UINT32          z_decompress(UINT32 zcurpixel);
	UINT32          dz_decompress(UINT32 zcurpixel, UINT32 dzcurpixel);
	UINT32          dz_compress(UINT32 value);
	INT32           normalize_dzpix(INT32 sum);
	bool            z_compare(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 sz, UINT16 dzpix, rdp_span_aux* userdata, const rdp_poly_state &object);

	// Fullscreen update-related
	void            video_update(n64_periphs* n64, bitmap_rgb32 &bitmap);

	// Commands
	void        cmd_invalid(UINT32 w1, UINT32 w2);
	void        cmd_noop(UINT32 w1, UINT32 w2);
	void        cmd_triangle(UINT32 w1, UINT32 w2);
	void        cmd_triangle_z(UINT32 w1, UINT32 w2);
	void        cmd_triangle_t(UINT32 w1, UINT32 w2);
	void        cmd_triangle_tz(UINT32 w1, UINT32 w2);
	void        cmd_triangle_s(UINT32 w1, UINT32 w2);
	void        cmd_triangle_sz(UINT32 w1, UINT32 w2);
	void        cmd_triangle_st(UINT32 w1, UINT32 w2);
	void        cmd_triangle_stz(UINT32 w1, UINT32 w2);
	void        cmd_tex_rect(UINT32 w1, UINT32 w2);
	void        cmd_tex_rect_flip(UINT32 w1, UINT32 w2);
	void        cmd_sync_load(UINT32 w1, UINT32 w2);
	void        cmd_sync_pipe(UINT32 w1, UINT32 w2);
	void        cmd_sync_tile(UINT32 w1, UINT32 w2);
	void        cmd_sync_full(UINT32 w1, UINT32 w2);
	void        cmd_set_key_gb(UINT32 w1, UINT32 w2);
	void        cmd_set_key_r(UINT32 w1, UINT32 w2);
	void        cmd_set_fill_color32(UINT32 w1, UINT32 w2);
	void        cmd_set_convert(UINT32 w1, UINT32 w2);
	void        cmd_set_scissor(UINT32 w1, UINT32 w2);
	void        cmd_set_prim_depth(UINT32 w1, UINT32 w2);
	void        cmd_set_other_modes(UINT32 w1, UINT32 w2);
	void        cmd_load_tlut(UINT32 w1, UINT32 w2);
	void        cmd_set_tile_size(UINT32 w1, UINT32 w2);
	void        cmd_load_block(UINT32 w1, UINT32 w2);
	void        cmd_load_tile(UINT32 w1, UINT32 w2);
	void        cmd_fill_rect(UINT32 w1, UINT32 w2);
	void        cmd_set_tile(UINT32 w1, UINT32 w2);
	void        cmd_set_fog_color(UINT32 w1, UINT32 w2);
	void        cmd_set_blend_color(UINT32 w1, UINT32 w2);
	void        cmd_set_prim_color(UINT32 w1, UINT32 w2);
	void        cmd_set_env_color(UINT32 w1, UINT32 w2);
	void        cmd_set_combine(UINT32 w1, UINT32 w2);
	void        cmd_set_texture_image(UINT32 w1, UINT32 w2);
	void        cmd_set_mask_image(UINT32 w1, UINT32 w2);
	void        cmd_set_color_image(UINT32 w1, UINT32 w2);

	void        rgbaz_clip(INT32 sr, INT32 sg, INT32 sb, INT32 sa, INT32* sz, rdp_span_aux* userdata);
	void        rgbaz_correct_triangle(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z, rdp_span_aux* userdata, const rdp_poly_state &object);

	void        triangle(bool shade, bool texture, bool zbuffer);

	void        get_dither_values(INT32 x, INT32 y, INT32* cdith, INT32* adith, const rdp_poly_state &object);

	UINT16 decompress_cvmask_frombyte(UINT8 x);
	void lookup_cvmask_derivatives(UINT32 mask, UINT8* offx, UINT8* offy, rdp_span_aux* userdata);

	misc_state_t m_misc_state;

	// Color constants
	color_t         m_blend_color;          /* constant blend color */
	color_t         m_prim_color;           /* flat primitive color */
	color_t         m_env_color;            /* generic color constant ('environment') */
	color_t         m_fog_color;            /* generic color constant ('fog') */
	color_t         m_key_scale;            /* color-keying constant */
	UINT8           m_lod_fraction;         /* Z-based LOD fraction for this poly */
	UINT8           m_prim_lod_fraction;    /* fixed LOD fraction for this poly */

	color_t         m_one;
	color_t         m_zero;

	UINT32          m_fill_color;

	other_modes_t   m_other_modes;

	n64_blender_t   m_blender;

	n64_texture_pipe_t m_tex_pipe;

	UINT8 m_hidden_bits[0x800000];

	UINT8 m_replicated_rgba[32];

	UINT16 m_dzpix_normalize[0x10000];

	rectangle_t     m_scissor;
	span_base_t     m_span_base;

	rectangle       m_visarea;

	void            draw_triangle(bool shade, bool texture, bool zbuffer, bool rect);
	void            compute_cvg_noflip(extent_t* spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base);
	void            compute_cvg_flip(extent_t* spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base);

	void*           m_aux_buf;
	UINT32          m_aux_buf_ptr;
	UINT32          m_aux_buf_index;

	bool            rdp_range_check(UINT32 addr);

	n64_tile_t      m_tiles[8];

private:
	void    write_pixel(UINT32 curpixel, INT32 r, INT32 g, INT32 b, rdp_span_aux* userdata, const rdp_poly_state &object);
	void    read_pixel(UINT32 curpixel, rdp_span_aux* userdata, const rdp_poly_state &object);
	void    copy_pixel(UINT32 curpixel, INT32 r, INT32 g, INT32 b, INT32 m_current_pix_cvg, const rdp_poly_state &object);
	void    fill_pixel(UINT32 curpixel, const rdp_poly_state &object);

	void    precalc_cvmask_derivatives(void);
	void    z_build_com_table(void);

	void    video_update16(n64_periphs* n64, bitmap_rgb32 &bitmap);
	void    video_update32(n64_periphs* n64, bitmap_rgb32 &bitmap);

	running_machine* m_machine;

	combine_modes_t m_combine;
	bool            m_pending_mode_block;
	bool            m_pipe_clean;

	cv_mask_derivative_t cvarray[(1 << 8)];

	UINT16  m_z_com_table[0x40000]; //precalced table of compressed z values, 18b: 512 KB array!
	UINT32  m_z_complete_dec_table[0x4000]; //the same for decompressed z values, 14b
	UINT8   m_compressed_cvmasks[0x10000]; //16bit cvmask -> to byte

	UINT32  m_cmd_data[0x1000];
	UINT32  m_temp_rect_data[0x1000];

	INT32   m_cmd_ptr;
	INT32   m_cmd_cur;

	UINT32  m_start;
	UINT32  m_end;
	UINT32  m_current;
	UINT32  m_status;

	UINT8*  m_tmem;

	// YUV factors
	INT32 m_k0;
	INT32 m_k1;
	INT32 m_k2;
	INT32 m_k3;
	INT32 m_k4;
	INT32 m_k5;

	// Texture perspective division
	INT32 m_norm_point_rom[64];
	INT32 m_norm_slope_rom[64];

	INT32 m_gamma_table[256];
	INT32 m_gamma_dither_table[0x4000];

	static UINT32 s_special_9bit_clamptable[512];
	static const z_decompress_entry_t m_z_dec_table[8];

	static const UINT8 s_bayer_matrix[16];
	static const UINT8 s_magic_matrix[16];
	static const rdp_command_t m_commands[0x40];
	static const INT32 s_rdp_command_length[];
	static const char* s_image_format[];
	static const char* s_image_size[];
};

#endif // _VIDEO_N64_H_
