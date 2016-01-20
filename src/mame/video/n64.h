// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef _VIDEO_N64_H_
#define _VIDEO_N64_H_

#include "emu.h"
#include "includes/n64.h"
#include "video/poly.h"

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

#define EXTENT_AUX_COUNT            (sizeof(rdp_span_aux)*(480*192)) // Screen coverage *192, more or less

/*****************************************************************************/

class n64_periphs;
class n64_rdp;

#include "video/n64types.h"
#include "video/rdpblend.h"
#include "video/rdptpipe.h"

typedef void (*rdp_command_t)(UINT32 w1, UINT32 w2);

class n64_rdp : public poly_manager<UINT32, rdp_poly_state, 8, 32000>
{
public:
	n64_rdp(n64_state &state);

	running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }

	void init_internal_state()
	{
		m_tmem = std::make_unique<UINT8[]>(0x1000);
		memset(m_tmem.get(), 0, 0x1000);

		UINT8* normpoint = machine().root_device().memregion("normpoint")->base();
		UINT8* normslope = machine().root_device().memregion("normslope")->base();

		for(INT32 i = 0; i < 64; i++)
		{
			m_norm_point_rom[i] = (normpoint[(i << 1) + 1] << 8) | normpoint[i << 1];
			m_norm_slope_rom[i] = (normslope[(i << 1) + 1] << 8) | normslope[i << 1];
		}

		memset(m_tiles, 0, 8 * sizeof(n64_tile_t));
		memset(m_cmd_data, 0, sizeof(m_cmd_data));

		for (INT32 i = 0; i < 8; i++)
		{
			m_tiles[i].num = i;
			m_tiles[i].invmm = rgbaint_t(~0, ~0, ~0, ~0);
			m_tiles[i].invmask = rgbaint_t(~0, ~0, ~0, ~0);
		}
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
	void        set_suba_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata);
	void        set_subb_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata);
	void        set_mul_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata);
	void        set_add_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata);
	void        set_sub_input_alpha(color_t** input, INT32 code, rdp_span_aux* userdata);
	void        set_mul_input_alpha(color_t** input, INT32 code, rdp_span_aux* userdata);

	// Texture memory
	UINT8*      get_tmem8() { return m_tmem.get(); }
	UINT16*     get_tmem16() { return (UINT16*)m_tmem.get(); }

	// YUV Factors
	void        set_yuv_factors(color_t k023, color_t k1, color_t k4, color_t k5) { m_k023 = k023; m_k1 = k1; m_k4 = k4; m_k5 = k5; }
	color_t&    get_k023() { return m_k023; }
	color_t&    get_k1() { return m_k1; }

	// Blender-related (move into RDP::Blender)
	void        set_blender_input(INT32 cycle, INT32 which, color_t** input_rgb, color_t** input_a, INT32 a, INT32 b, rdp_span_aux* userdata);

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
	INT32           get_alpha_cvg(INT32 comb_alpha, rdp_span_aux* userdata, const rdp_poly_state &object);

	void            z_store(const rdp_poly_state &object, UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 z, UINT32 enc);
	UINT32          z_decompress(UINT32 zcurpixel);
	UINT32          dz_decompress(UINT32 zcurpixel, UINT32 dzcurpixel);
	UINT32          dz_compress(UINT32 value);
	INT32           normalize_dzpix(INT32 sum);
	bool            z_compare(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 sz, UINT16 dzpix, rdp_span_aux* userdata, const rdp_poly_state &object);

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
	color_t         m_prim_alpha;           /* flat primitive alpha */
	color_t         m_env_color;            /* generic color constant ('environment') */
	color_t         m_env_alpha;            /* generic alpha constant ('environment') */
	color_t         m_fog_color;            /* generic color constant ('fog') */
	color_t         m_key_scale;            /* color-keying constant */
	color_t         m_lod_fraction;         /* Z-based LOD fraction for this poly */
	color_t         m_prim_lod_fraction;    /* fixed LOD fraction for this poly */

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

	void            draw_triangle(bool shade, bool texture, bool zbuffer, bool rect);

	std::unique_ptr<UINT8[]>  m_aux_buf;
	UINT32          m_aux_buf_ptr;
	UINT32          m_aux_buf_index;

	bool            rdp_range_check(UINT32 addr);

	n64_tile_t      m_tiles[8];

private:
	void    compute_cvg_noflip(extent_t* spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base);
	void    compute_cvg_flip(extent_t* spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base);

	void    write_pixel(UINT32 curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object);
	void    read_pixel(UINT32 curpixel, rdp_span_aux* userdata, const rdp_poly_state &object);
	void    copy_pixel(UINT32 curpixel, color_t& color, const rdp_poly_state &object);
	void    fill_pixel(UINT32 curpixel, const rdp_poly_state &object);

	void    precalc_cvmask_derivatives(void);
	void    z_build_com_table(void);

	typedef void (n64_rdp::*compute_cvg_t) (extent_t* spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base);
	compute_cvg_t   m_compute_cvg[2];

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

	std::unique_ptr<UINT8[]>  m_tmem;

	// YUV factors
	color_t m_k023;
	color_t m_k1;
	color_t m_k4;
	color_t m_k5;

	// Texture perspective division
	INT32 m_norm_point_rom[64];
	INT32 m_norm_slope_rom[64];

	static UINT32 s_special_9bit_clamptable[512];
	static const z_decompress_entry_t m_z_dec_table[8];

	static const UINT8 s_bayer_matrix[16];
	static const UINT8 s_magic_matrix[16];
	static const rdp_command_t m_commands[0x40];
	static const INT32 s_rdp_command_length[];
	static const char* s_image_format[];
	static const char* s_image_size[];

public:
	bool ignore;
	bool dolog;
};

#endif // _VIDEO_N64_H_
