// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Texture Fetch Unit (TF)
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "emu.h"

struct other_modes_t;
struct misc_state_t;
class color_t;
struct rdp_span_aux;
struct rdp_poly_state;
struct n64_tile_t;

class n64_texture_pipe_t
{
	public:
		typedef UINT32 (n64_texture_pipe_t::*texel_fetcher_t) (INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		typedef void (n64_texture_pipe_t::*texel_cycler_t) (color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object);

		n64_texture_pipe_t()
		{
			m_maskbits_table[0] = 0x3ff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((UINT16)(0xffff) >> (16 - i)) & 0x3ff;
			}
			m_start_span = false;

			for (int idx = 0; idx < 80; idx++)
			{
				m_texel_fetch[idx] = &n64_texture_pipe_t::fetch_nop;
			}

			m_texel_fetch[ 8] = &n64_texture_pipe_t::fetch_rgba16_raw;
			m_texel_fetch[ 9] = &n64_texture_pipe_t::fetch_rgba16_raw;
			m_texel_fetch[10] = &n64_texture_pipe_t::fetch_rgba16_tlut0;
			m_texel_fetch[11] = &n64_texture_pipe_t::fetch_rgba16_tlut1;
			m_texel_fetch[12] = &n64_texture_pipe_t::fetch_rgba32_raw;
			m_texel_fetch[13] = &n64_texture_pipe_t::fetch_rgba32_raw;
			m_texel_fetch[14] = &n64_texture_pipe_t::fetch_rgba32_tlut0;
			m_texel_fetch[15] = &n64_texture_pipe_t::fetch_rgba32_tlut1;

			m_texel_fetch[24] = &n64_texture_pipe_t::fetch_yuv;
			m_texel_fetch[25] = &n64_texture_pipe_t::fetch_yuv;
			m_texel_fetch[26] = &n64_texture_pipe_t::fetch_yuv;
			m_texel_fetch[27] = &n64_texture_pipe_t::fetch_yuv;

			m_texel_fetch[32] = &n64_texture_pipe_t::fetch_ci4_raw;
			m_texel_fetch[33] = &n64_texture_pipe_t::fetch_ci4_raw;
			m_texel_fetch[34] = &n64_texture_pipe_t::fetch_ci4_tlut0;
			m_texel_fetch[35] = &n64_texture_pipe_t::fetch_ci4_tlut1;
			m_texel_fetch[36] = &n64_texture_pipe_t::fetch_ci8_raw;
			m_texel_fetch[37] = &n64_texture_pipe_t::fetch_ci8_raw;
			m_texel_fetch[38] = &n64_texture_pipe_t::fetch_ci8_tlut0;
			m_texel_fetch[39] = &n64_texture_pipe_t::fetch_ci8_tlut1;

			m_texel_fetch[48] = &n64_texture_pipe_t::fetch_ia4_raw;
			m_texel_fetch[49] = &n64_texture_pipe_t::fetch_ia4_raw;
			m_texel_fetch[50] = &n64_texture_pipe_t::fetch_ia4_tlut0;
			m_texel_fetch[51] = &n64_texture_pipe_t::fetch_ia4_tlut1;
			m_texel_fetch[52] = &n64_texture_pipe_t::fetch_ia8_raw;
			m_texel_fetch[53] = &n64_texture_pipe_t::fetch_ia8_raw;
			m_texel_fetch[54] = &n64_texture_pipe_t::fetch_ia8_tlut0;
			m_texel_fetch[55] = &n64_texture_pipe_t::fetch_ia8_tlut1;
			m_texel_fetch[56] = &n64_texture_pipe_t::fetch_ia16_raw;
			m_texel_fetch[57] = &n64_texture_pipe_t::fetch_ia16_raw;
			m_texel_fetch[58] = &n64_texture_pipe_t::fetch_ia16_tlut0;
			m_texel_fetch[59] = &n64_texture_pipe_t::fetch_ia16_tlut1;

			m_texel_fetch[64] = &n64_texture_pipe_t::fetch_i4_raw;
			m_texel_fetch[65] = &n64_texture_pipe_t::fetch_i4_raw;
			m_texel_fetch[66] = &n64_texture_pipe_t::fetch_i4_tlut0;
			m_texel_fetch[67] = &n64_texture_pipe_t::fetch_i4_tlut1;
			m_texel_fetch[68] = &n64_texture_pipe_t::fetch_i8_raw;
			m_texel_fetch[69] = &n64_texture_pipe_t::fetch_i8_raw;
			m_texel_fetch[70] = &n64_texture_pipe_t::fetch_i8_tlut0;
			m_texel_fetch[71] = &n64_texture_pipe_t::fetch_i8_tlut1;

			m_cycle[0] = &n64_texture_pipe_t::cycle_nearest;
			m_cycle[1] = &n64_texture_pipe_t::cycle_nearest_lerp;
			m_cycle[2] = &n64_texture_pipe_t::cycle_linear;
			m_cycle[3] = &n64_texture_pipe_t::cycle_linear_lerp;
		}

		void                cycle_nearest(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_nearest_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_linear(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_linear_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object);

		texel_cycler_t      m_cycle[4];

		void                copy(color_t* TEX, INT32 SSS, INT32 SST, UINT32 tilenum, const rdp_poly_state& object, rdp_span_aux* userdata);
		void                calculate_clamp_diffs(UINT32 prim_tile, const rdp_poly_state& object);
		void                lod_1cycle(INT32* sss, INT32* sst, const INT32 s, const INT32 t, const INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                lod_2cycle(INT32* sss, INT32* sst, const INT32 s, const INT32 t, const INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, const INT32 prim_tile, INT32* t1, INT32* t2, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                lod_2cycle_limited(INT32* sss, INT32* sst, const INT32 s, const INT32 t, INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, const INT32 prim_tile, INT32* t1, const rdp_poly_state& object);

		void                set_machine(running_machine& machine);

		bool                m_start_span;

	private:
		void                mask(INT32* S, INT32* T, const n64_tile_t& tile);
		void                mask_coupled(INT32* S, INT32* S1, INT32* T, INT32* T1, const n64_tile_t& tile);

		void                shift_cycle(INT32* S, INT32* T, bool* maxs, bool* maxt, const n64_tile_t& tile);
		void                shift_copy(INT32* S, INT32* T, const n64_tile_t& tile);

		void                clamp_cycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, const bool maxs, const bool maxt, const INT32 tilenum, const n64_tile_t& tile);
		void                clamp_cycle_light(INT32* S, INT32* T, const bool maxs, const bool maxt, const INT32 tilenum, const n64_tile_t& tile);

		UINT32              fetch_nop(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		UINT32              fetch_rgba16_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_rgba16_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_rgba16_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_rgba32_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_rgba32_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_rgba32_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		UINT32              fetch_yuv(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		UINT32              fetch_ci4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ci4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ci4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ci8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ci8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ci8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		UINT32              fetch_ia4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia16_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia16_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_ia16_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		UINT32              fetch_i4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_i4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_i4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_i8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_i8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);
		UINT32              fetch_i8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata);

		texel_fetcher_t     m_texel_fetch[16*5];

		n64_rdp*            m_rdp;

		INT32               m_maskbits_table[16];
		UINT32              m_expand_16to32_table[0x10000];
		UINT16              m_lod_lookup[0x80000];
		INT32               m_clamp_s_diff[8];
		INT32               m_clamp_t_diff[8];
};

#endif // _VIDEO_RDPTEXPIPE_H_
