#ifndef _VIDEO_N64_H_
#define _VIDEO_N64_H_

#include "emu.h"
#include "video/rdpblend.h"
#include "video/rdptri.h"
#include "video/rdpfb.h"
#include "video/rdpfrect.h"
#include "video/rdptrect.h"
#include "video/rdptpipe.h"
#include "video/rdpspn16.h"
#include "video/rdpfetch.h"

/*****************************************************************************/

#define PIXEL_SIZE_4BIT			0
#define PIXEL_SIZE_8BIT			1
#define PIXEL_SIZE_16BIT		2
#define PIXEL_SIZE_32BIT		3

#define CYCLE_TYPE_1			0
#define CYCLE_TYPE_2			1
#define CYCLE_TYPE_COPY			2
#define CYCLE_TYPE_FILL			3

#define SAMPLE_TYPE_1x1         0
#define SAMPLE_TYPE_2x2         1

#define BYTE_ADDR_XOR		BYTE4_XOR_BE(0)
#define WORD_ADDR_XOR		(WORD_XOR_BE(0) >> 1)

#define XOR_SWAP_BYTE_SHIFT		2
#define XOR_SWAP_WORD_SHIFT		1
#define XOR_SWAP_DWORD_SHIFT	0

#define XOR_SWAP_BYTE	4
#define XOR_SWAP_WORD	2
#define XOR_SWAP_DWORD	1

//sign-extension macros
#define SIGN17(x)	(((x) & 0x10000) ? ((x) | ~0x1ffff) : ((x) & 0x1ffff))
#define SIGN16(x)	(((x) & 0x8000) ? ((x) | ~0xffff) : ((x) & 0xffff))
#define SIGN11(x)	(((x) & 0x400) ? ((x) | ~0x7ff) : ((x) & 0x7ff))

/*****************************************************************************/

namespace N64
{

namespace RDP
{

class Processor;
class MiscState;
class OtherModes;

class Color
{
	public:
		Color()
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

class Tile
{
	public:
		int format;	// Image data format: RGBA, YUV, CI, IA, I
		int size; // Size of texel element: 4b, 8b, 16b, 32b
		UINT32 line; // Size of tile line in bytes
		UINT32 tmem; // Starting tmem address for this tile in bytes
		int palette; // Palette number for 4b CI texels
		int ct, mt, cs, ms; // Clamp / mirror enable bits for S / T direction
		int mask_t, shift_t, mask_s, shift_s; // Mask values / LOD shifts
		UINT16 sl, tl, sh, th;		// 10.2 fixed-point, starting and ending texel row / column
		int num;
};

class MiscState
{
	public:
		MiscState()
		{
			m_curpixel_cvg = 0;
			m_curpixel_overlap = 0;

			m_max_level = 0;
			m_min_level = 0;
		}

		int m_fb_format;
		int m_fb_size;
		int m_fb_width;
		int m_fb_height;
		UINT32 m_fb_address;

		UINT32 m_zb_address;

		int m_ti_format;
		int m_ti_size;
		int m_ti_width;
		UINT32 m_ti_address;

		UINT32 m_curpixel_cvg;
		UINT32 m_curpixel_overlap;

		UINT8 m_random_seed;

		int m_special_bsel0;
		int m_special_bsel1;

		UINT32 m_max_level;
		UINT32 m_min_level;

		UINT16 m_primitive_z;
		UINT16 m_primitive_delta_z;
};

class CombineModes
{
	public:
		int sub_a_rgb0;
		int sub_b_rgb0;
		int mul_rgb0;
		int add_rgb0;
		int sub_a_a0;
		int sub_b_a0;
		int mul_a0;
		int add_a0;

		int sub_a_rgb1;
		int sub_b_rgb1;
		int mul_rgb1;
		int add_rgb1;
		int sub_a_a1;
		int sub_b_a1;
		int mul_a1;
		int add_a1;
};

class OtherModes
{
	public:
		int cycle_type;
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
		int rgb_dither_sel;
		int alpha_dither_sel;
		int blend_m1a_0;
		int blend_m1a_1;
		int blend_m1b_0;
		int blend_m1b_1;
		int blend_m2a_0;
		int blend_m2a_1;
		int blend_m2b_0;
		int blend_m2b_1;
		int tex_edge;
		bool force_blend;
		bool alpha_cvg_select;
		bool cvg_times_alpha;
		int z_mode;
		int cvg_dest;
		bool color_on_cvg;
		bool image_read_en;
		bool z_update_en;
		bool z_compare_en;
		bool antialias_en;
		bool z_source_sel;
		bool dither_alpha_en;
		bool alpha_compare_en;
};

class ColorInputs
{
	public:
		// combiner inputs
		UINT8 *combiner_rgbsub_a_r[2];
		UINT8 *combiner_rgbsub_a_g[2];
		UINT8 *combiner_rgbsub_a_b[2];
		UINT8 *combiner_rgbsub_b_r[2];
		UINT8 *combiner_rgbsub_b_g[2];
		UINT8 *combiner_rgbsub_b_b[2];
		UINT8 *combiner_rgbmul_r[2];
		UINT8 *combiner_rgbmul_g[2];
		UINT8 *combiner_rgbmul_b[2];
		UINT8 *combiner_rgbadd_r[2];
		UINT8 *combiner_rgbadd_g[2];
		UINT8 *combiner_rgbadd_b[2];

		UINT8 *combiner_alphasub_a[2];
		UINT8 *combiner_alphasub_b[2];
		UINT8 *combiner_alphamul[2];
		UINT8 *combiner_alphaadd[2];

		// blender input
		UINT8 *blender1a_r[2];
		UINT8 *blender1a_g[2];
		UINT8 *blender1a_b[2];
		UINT8 *blender1b_a[2];
		UINT8 *blender2a_r[2];
		UINT8 *blender2a_g[2];
		UINT8 *blender2a_b[2];
		UINT8 *blender2b_a[2];
};

class Processor
{
	public:
		Processor()
		{
			m_cmd_ptr = 0;
			m_cmd_cur = 0;

			m_start = 0;
			m_end = 0;
			m_current = 0;
			m_status = 0x88;

			for(int i = 0; i < (1 << 16); i++)
			{
				UINT8 r = ((i >> 8) & 0xf8) | (i >> 13);
				UINT8 g = ((i >> 3) & 0xf8) | ((i >>  8) & 0x07);
				UINT8 b = ((i << 2) & 0xf8) | ((i >>  3) & 0x07);
				UINT8 a = (i & 1) ? 0xff : 0;
				m_rgb16_to_rgb32_lut[i] = (r << 24) | (g << 16) | (b << 8) | a;

				r = g = b = (i >> 8) & 0xff;
				a = i & 0xff;
				m_ia8_to_rgb32_lut[i] = (r << 24) | (g << 16) | (b << 8) | a;
			}

			for(int i = 0; i < (1 << 24); i++)
			{
				UINT8 A = (i >> 16) & 0x000000ff;
				UINT8 B = (i >> 8) & 0x000000ff;
				UINT8 C = i & 0x000000ff;
				m_cc_lut1[i] = (INT16)((((((INT32)A - (INT32)B) * (INT32)C) + 0x80) >> 8) & 0x0000ffff);
			}

			for(int i = 0; i < (1 << 16); i++)
			{
				for(int j = 0; j < (1 << 8); j++)
				{
					INT32 temp = (INT32)((INT16)i) + j;
					if(temp > 255)
					{
						m_cc_lut2[(i << 8) | j] = 255;
					}
					else if(temp < 0)
					{
						m_cc_lut2[(i << 8) | j] = 0;
					}
					else
					{
						m_cc_lut2[(i << 8) | j] = (UINT8)temp;
					}
				}
			}

			for (int i = 0; i < 8; i++)
			{
				m_tiles[i].num = i;
			}

			m_one_color.c = 0xffffffff;
			m_zero_color.c = 0x00000000;

			m_color_inputs.combiner_rgbsub_a_r[0] = m_color_inputs.combiner_rgbsub_a_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbsub_a_g[0] = m_color_inputs.combiner_rgbsub_a_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbsub_a_b[0] = m_color_inputs.combiner_rgbsub_a_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbsub_b_r[0] = m_color_inputs.combiner_rgbsub_b_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbsub_b_g[0] = m_color_inputs.combiner_rgbsub_b_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbsub_b_b[0] = m_color_inputs.combiner_rgbsub_b_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbmul_r[0] = m_color_inputs.combiner_rgbmul_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbmul_g[0] = m_color_inputs.combiner_rgbmul_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbmul_b[0] = m_color_inputs.combiner_rgbmul_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbadd_r[0] = m_color_inputs.combiner_rgbadd_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbadd_g[0] = m_color_inputs.combiner_rgbadd_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbadd_b[0] = m_color_inputs.combiner_rgbadd_b[1] = &m_one_color.i.b;

			m_color_inputs.combiner_alphasub_a[0] = m_color_inputs.combiner_alphasub_a[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphasub_b[0] = m_color_inputs.combiner_alphasub_b[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphamul[0] = m_color_inputs.combiner_alphamul[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphaadd[0] = m_color_inputs.combiner_alphaadd[1] = &m_one_color.i.a;

			m_color_inputs.blender1a_r[0] = m_color_inputs.blender1a_r[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1a_g[0] = m_color_inputs.blender1a_g[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1a_b[0] = m_color_inputs.blender1a_b[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1b_a[0] = m_color_inputs.blender1b_a[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_r[0] = m_color_inputs.blender2a_r[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_g[0] = m_color_inputs.blender2a_g[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_b[0] = m_color_inputs.blender2a_b[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2b_a[0] = m_color_inputs.blender2b_a[1] = &m_pixel_color.i.r;

			m_tmem = NULL;

			m_machine = NULL;

			memset(m_hidden_bits, 3, 4194304); // Hack / fix for letters in Rayman 2

			m_prim_lod_frac = 0;
			m_lod_frac = 0;

			for (int i = 0; i < 256; i++)
			{
				m_gamma_table[i] = sqrt((float)(i << 6));
				m_gamma_table[i] <<= 1;
			}

			for (int i = 0; i < 0x4000; i++)
			{
				m_gamma_dither_table[i] = sqrt((float)i);
				m_gamma_dither_table[i] <<= 1;
			}

			BuildCompressedZTable();
		}

		~Processor() { }

		void	Dasm(char *buffer);

		void	ProcessList();
		UINT32	ReadData(UINT32 address);

		void	InitInternalState()
		{
			if(m_machine)
			{
				m_tmem = auto_alloc_array(m_machine, UINT8, 0x1000);
				memset(m_tmem, 0, 0x1000);

				UINT8 *normpoint = memory_region(m_machine, "normpoint");
				UINT8 *normslope = memory_region(m_machine, "normslope");

				for(int i = 0; i < 64; i++)
				{
					m_norm_point_rom[i] = (normpoint[(i << 1) + 1] << 8) | normpoint[i << 1];
					m_norm_slope_rom[i] = (normslope[(i << 1) + 1] << 8) | normslope[i << 1];
				}
			}
		}

		void		SetMachine(running_machine* machine) { m_machine = machine; }

		// CPU-visible registers
		void		SetStartReg(UINT32 val) { m_start = val; }
		UINT32		GetStartReg() const { return m_start; }

		void		SetEndReg(UINT32 val) { m_end = val; }
		UINT32		GetEndReg() const { return m_end; }

		void		SetCurrentReg(UINT32 val) { m_current = val; }
		UINT32		GetCurrentReg() const { return m_current; }

		void		SetStatusReg(UINT32 val) { m_status = val; }
		UINT32		GetStatusReg() const { return m_status; }

		// Functional blocks
		Blender*		GetBlender() { return &m_blender; }
		Framebuffer*	GetFramebuffer() { return &m_framebuffer; }
		TexturePipe*	GetTexPipe() { return &m_tex_pipe; }

		// Internal state
		OtherModes*		GetOtherModes() { return &m_other_modes; }
		ColorInputs*	GetColorInputs() { return &m_color_inputs; }
		CombineModes*	GetCombine() { return &m_combine; }
		MiscState*		GetMiscState() { return &m_misc_state; }

		// Color constants
		Color*		GetBlendColor() { return &m_blend_color; }
		void		SetBlendColor(UINT32 color) { m_blend_color.c = color; }

		Color*		GetPixelColor() { return &m_pixel_color; }
		void		SetPixelColor(UINT32 color) { m_pixel_color.c = color; }

		Color*		GetInvPixelColor() { return &m_inv_pixel_color; }
		void		SetInvPixelColor(UINT32 color) { m_inv_pixel_color.c = color; }

		Color*		GetBlendedColor() { return &m_blended_pixel_color; }
		void		SetBlendedColor(UINT32 color) { m_blended_pixel_color.c = color; }

		Color*		GetMemoryColor() { return &m_memory_color; }
		void		SetMemoryColor(UINT32 color) { m_memory_color.c = color; }

		Color*		GetPrimColor() { return &m_prim_color; }
		void		SetPrimColor(UINT32 color) { m_prim_color.c = color; }

		Color*		GetEnvColor() { return &m_env_color; }
		void		SetEnvColor(UINT32 color) { m_env_color.c = color; }

		Color*		GetFogColor() { return &m_fog_color; }
		void		SetFogColor(UINT32 color) { m_fog_color.c = color; }

		Color*		GetCombinedColor() { return &m_combined_color; }
		void		SetCombinedColor(UINT32 color) { m_combined_color.c = color; }

		Color*		GetTexel0Color() { return &m_texel0_color; }
		void		SetTexel0Color(UINT32 color) { m_texel0_color.c = color; }

		Color*		GetTexel1Color() { return &m_texel1_color; }
		void		SetTexel1Color(UINT32 color) { m_texel1_color.c = color; }

		Color*		GetShadeColor() { return &m_shade_color; }
		void		SetShadeColor(UINT32 color) { m_shade_color.c = color; }

		Color*		GetKeyScale() { return &m_key_scale; }
		void		SetKeyScale(UINT32 scale) { m_key_scale.c = scale; }

		Color*		GetNoiseColor() { return &m_noise_color; }
		void		SetNoiseColor(UINT32 color) { m_noise_color.c = color; }

		Color*		GetOne() { return &m_one_color; }
		Color*		GetZero() { return &m_zero_color; }

		UINT8*		GetLODFrac() { return &m_lod_frac; }
		void		SetLODFrac(UINT8 lod_frac) { m_lod_frac = lod_frac; }

		UINT8*		GetPrimLODFrac() { return &m_prim_lod_frac; }
		void		SetPrimLODFrac(UINT8 prim_lod_frac) { m_prim_lod_frac = prim_lod_frac; }

		// Color Combiner
		void		SetSubAInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code);
		void		SetSubBInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code);
		void		SetMulInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code);
		void		SetAddInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code);
		void		SetSubInputAlpha(UINT8 **input, int code);
		void		SetMulInputAlpha(UINT8 **input, int code);

		// Texture memory
		UINT8*		GetTMEM() { return m_tmem; }
		UINT16*		GetTMEM16() { return (UINT16*)m_tmem; }
		UINT32*		GetTMEM32() { return (UINT32*)m_tmem; }
		UINT16*		GetTLUT() { return (UINT16*)(m_tmem + 0x800); }
		Tile*		GetTiles(){ return m_tiles; }

		// Emulation Accelerators
		UINT32		LookUp16To32(UINT16 in) const { return m_rgb16_to_rgb32_lut[in]; }
		UINT32		LookUpIA8To32(UINT16 in) const { return m_ia8_to_rgb32_lut[in]; }
		UINT16*		GetCCLUT1() { return m_cc_lut1; }
		UINT8*		GetCCLUT2() { return m_cc_lut2; }
		UINT8		GetRandom() { return m_misc_state.m_random_seed += 0x13; }

		// YUV Factors
		void		SetYUVFactors(INT32 k0, INT32 k1, INT32 k2, INT32 k3, INT32 k4, INT32 k5) { m_k0 = k0; m_k1 = k1; m_k2 = k2; m_k3 = k3; m_k4 = k4; m_k5 = k5; }
		INT32		GetK0() const { return m_k0; }
		INT32		GetK1() const { return m_k1; }
		INT32		GetK2() const { return m_k2; }
		INT32		GetK3() const { return m_k3; }
		INT32*		GetK4() { return &m_k4; }
		INT32*		GetK5() { return &m_k5; }

		// Blender-related (move into RDP::Blender)
		void		SetBlenderInput(int cycle, int which, UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, UINT8 **input_a, int a, int b);

		// Render-related (move into eventual drawing-related classes?)
		Rectangle*		GetScissor() { return &m_scissor; }
		void			TCDiv(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst);
		UINT32			GetLog2(UINT32 lod_clamp);
		void			RenderSpans(int start, int end, int tilenum, bool shade, bool texture, bool zbuffer, bool flip);
		UINT8*			GetHiddenBits() { return m_hidden_bits; }
		void			GetAlphaCvg(UINT8 *comb_alpha);
		const UINT8*	GetBayerMatrix() const { return s_bayer_matrix; }
		const UINT8*	GetMagicMatrix() const { return s_magic_matrix; }
		Span*			GetSpans() { return m_span; }
		int				GetCurrFIFOIndex() const { return m_cmd_cur; }
		UINT32			GetFillColor32() const { return m_fill_color; }

		void			ZStore(UINT16* zb, UINT8* zhb, UINT32 z, UINT32 deltaz);
		UINT32			DecompressZ(UINT16 *zb);
		UINT16			DecompressDZ(UINT16* zb, UINT8* zhb);
		INT32			NormalizeDZPix(INT32 sum);
		bool			ZCompare(void* fb, UINT8* hb, UINT16* zb, UINT8* zhb, UINT32 sz, UINT16 dzpix);

		// Fullscreen update-related
		void			VideoUpdate(bitmap_t *bitmap);

		// Commands
		void		CmdInvalid(UINT32 w1, UINT32 w2);
		void		CmdNoOp(UINT32 w1, UINT32 w2);
		void		CmdTriangle(UINT32 w1, UINT32 w2);
		void		CmdTriangleZ(UINT32 w1, UINT32 w2);
		void		CmdTriangleT(UINT32 w1, UINT32 w2);
		void		CmdTriangleTZ(UINT32 w1, UINT32 w2);
		void		CmdTriangleS(UINT32 w1, UINT32 w2);
		void		CmdTriangleSZ(UINT32 w1, UINT32 w2);
		void		CmdTriangleST(UINT32 w1, UINT32 w2);
		void		CmdTriangleSTZ(UINT32 w1, UINT32 w2);
		void		CmdTexRect(UINT32 w1, UINT32 w2);
		void		CmdTexRectFlip(UINT32 w1, UINT32 w2);
		void		CmdSyncLoad(UINT32 w1, UINT32 w2);
		void		CmdSyncPipe(UINT32 w1, UINT32 w2);
		void		CmdSyncTile(UINT32 w1, UINT32 w2);
		void		CmdSyncFull(UINT32 w1, UINT32 w2);
		void		CmdSetKeyGB(UINT32 w1, UINT32 w2);
		void		CmdSetKeyR(UINT32 w1, UINT32 w2);
		void		CmdSetFillColor32(UINT32 w1, UINT32 w2);
		void		CmdSetConvert(UINT32 w1, UINT32 w2);
		void		CmdSetScissor(UINT32 w1, UINT32 w2);
		void		CmdSetPrimDepth(UINT32 w1, UINT32 w2);
		void		CmdSetOtherModes(UINT32 w1, UINT32 w2);
		void		CmdLoadTLUT(UINT32 w1, UINT32 w2);
		void		CmdSetTileSize(UINT32 w1, UINT32 w2);
		void		CmdLoadBlock(UINT32 w1, UINT32 w2);
		void		CmdLoadTile(UINT32 w1, UINT32 w2);
		void		CmdFillRect(UINT32 w1, UINT32 w2);
		void		CmdSetTile(UINT32 w1, UINT32 w2);
		void		CmdSetFogColor(UINT32 w1, UINT32 w2);
		void		CmdSetBlendColor(UINT32 w1, UINT32 w2);
		void		CmdSetPrimColor(UINT32 w1, UINT32 w2);
		void		CmdSetEnvColor(UINT32 w1, UINT32 w2);
		void		CmdSetCombine(UINT32 w1, UINT32 w2);
		void		CmdSetTextureImage(UINT32 w1, UINT32 w2);
		void		CmdSetMaskImage(UINT32 w1, UINT32 w2);
		void		CmdSetColorImage(UINT32 w1, UINT32 w2);

		void		Triangle(bool shade, bool texture, bool zbuffer);
		UINT32		AddRightCvg(UINT32 x, UINT32 k);
		UINT32		AddLeftCvg(UINT32 x, UINT32 k);

		UINT32*		GetCommandData() { return m_cmd_data; }

	protected:
		Blender			m_blender;
		Framebuffer		m_framebuffer;
		TexturePipe		m_tex_pipe;

		OtherModes		m_other_modes;
		MiscState		m_misc_state;
		ColorInputs		m_color_inputs;
		CombineModes	m_combine;

		Color		m_pixel_color;
		Color		m_inv_pixel_color;
		Color		m_blended_pixel_color;
		Color		m_memory_color;
		Color		m_blend_color;

		Color		m_prim_color;
		Color		m_env_color;
		Color		m_fog_color;
		Color		m_combined_color;
		Color		m_texel0_color;
		Color		m_texel1_color;
		Color		m_shade_color;
		Color		m_key_scale;
		Color		m_noise_color;
		UINT8		m_lod_frac;
		UINT8		m_prim_lod_frac;

		Color		m_one_color;
		Color		m_zero_color;

		UINT32		m_fill_color;

		UINT16		m_cc_lut1[(1<<24)];
		UINT8		m_cc_lut2[(1<<24)];
		UINT32		m_rgb16_to_rgb32_lut[(1 << 16)];
		UINT32		m_ia8_to_rgb32_lut[(1 << 16)];

		UINT32		m_cmd_data[0x1000];
		int 		m_cmd_ptr;
		int 		m_cmd_cur;

		UINT32		m_start;
		UINT32		m_end;
		UINT32		m_current;
		UINT32		m_status;

		UINT8*		m_tmem;

		Tile		m_tiles[8];

		running_machine* m_machine;

		// YUV factors
		INT32 m_k0;
		INT32 m_k1;
		INT32 m_k2;
		INT32 m_k3;
		INT32 m_k4;
		INT32 m_k5;

		// Render-related (move into eventual drawing-related classes?)
		Rectangle m_scissor;

		// Texture perspective division
		INT32 m_norm_point_rom[64];
		INT32 m_norm_slope_rom[64];

		UINT8 m_hidden_bits[0x800000];

		Span m_span[4096];

		static const UINT8 s_bayer_matrix[16];
		static const UINT8 s_magic_matrix[16];

		INT32 m_gamma_table[256];
		INT32 m_gamma_dither_table[0x4000];

		class ZDecompressEntry
		{
			public:
				UINT32 shift;
				UINT32 add;
		};

		void BuildCompressedZTable();
		UINT16 m_z_compress_table[0x40000];
		static const ZDecompressEntry m_z_decompress_table[8];

		// Internal screen-update functions
		void			VideoUpdate16(bitmap_t *bitmap);
		void			VideoUpdate32(bitmap_t *bitmap);

		typedef void (*Command)(UINT32 w1, UINT32 w2);

		static const Command m_commands[0x40];
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_N64_H_
