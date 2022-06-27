// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Texture Fetch Unit (TF)
    -------------------

    by Ryan Holtz
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "n64types.h"

class n64_texture_pipe_t
{
	public:
		typedef void (n64_texture_pipe_t::*texel_fetcher_t) (rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		typedef void (n64_texture_pipe_t::*texel_cycler_t) (color_t* TEX, color_t* prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle, rdp_span_aux* userdata, const rdp_poly_state& object);

		n64_texture_pipe_t()
		{
			m_maskbits_table[0] = 0xffff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((uint16_t)(0xffff) >> (16 - i)) & 0x3ff;
			}
			m_start_span = false;

			for (auto & elem : m_texel_fetch)
			{
				elem = &n64_texture_pipe_t::fetch_nop;
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

		void                cycle_nearest(color_t* TEX, color_t* prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_nearest_lerp(color_t* TEX, color_t* prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_linear(color_t* TEX, color_t* prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                cycle_linear_lerp(color_t* TEX, color_t* prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle, rdp_span_aux* userdata, const rdp_poly_state& object);

		texel_cycler_t      m_cycle[4];

		void                copy(color_t* TEX, int32_t SSS, int32_t SST, uint32_t tilenum, const rdp_poly_state& object, rdp_span_aux* userdata);
		void                calculate_clamp_diffs(uint32_t prim_tile, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                lod_1cycle(int32_t* sss, int32_t* sst, const int32_t s, const int32_t t, const int32_t w, const int32_t dsinc, const int32_t dtinc, const int32_t dwinc, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                lod_2cycle(int32_t* sss, int32_t* sst, const int32_t s, const int32_t t, const int32_t w, const int32_t dsinc, const int32_t dtinc, const int32_t dwinc, const int32_t prim_tile, int32_t* t1, int32_t* t2, rdp_span_aux* userdata, const rdp_poly_state& object);
		void                lod_2cycle_limited(int32_t* sss, int32_t* sst, const int32_t s, const int32_t t, int32_t w, const int32_t dsinc, const int32_t dtinc, const int32_t dwinc, const int32_t prim_tile, int32_t* t1, const rdp_poly_state& object);

		void                set_machine(running_machine& machine);

		bool                m_start_span;

	private:
		void                mask(rgbaint_t& sstt, const n64_tile_t& tile);

		rgbaint_t           shift_cycle(rgbaint_t& st, const n64_tile_t& tile);
		void                shift_copy(rgbaint_t& st, const n64_tile_t& tile);

		void                clamp_cycle(rgbaint_t& st, rgbaint_t& stfrac, rgbaint_t& maxst, const int32_t tilenum, const n64_tile_t& tile, rdp_span_aux* userdata);
		void                clamp_cycle_light(rgbaint_t& st, rgbaint_t& maxst, const int32_t tilenum, const n64_tile_t& tile, rdp_span_aux* userdata);

		void                fetch_nop(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		void                fetch_rgba16_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_rgba16_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_rgba16_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_rgba32_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_rgba32_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_rgba32_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		void                fetch_yuv(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		void                fetch_ci4_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ci4_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ci4_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ci8_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ci8_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ci8_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		void                fetch_ia4_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia4_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia4_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia8_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia8_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia8_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia16_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia16_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_ia16_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		void                fetch_i4_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_i4_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_i4_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_i8_tlut0(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_i8_tlut1(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);
		void                fetch_i8_raw(rgbaint_t& out, int32_t s, int32_t t, int32_t tbase, int32_t tpal, rdp_span_aux* userdata);

		texel_fetcher_t     m_texel_fetch[16*5];

		n64_rdp*            m_rdp;

		int32_t               m_maskbits_table[16];
		color_t             m_expand_16to32_table[0x10000];
		uint16_t              m_lod_lookup[0x80000];

		rgbaint_t           m_st2_add;
		rgbaint_t           m_v1;
};

#endif // _VIDEO_RDPTEXPIPE_H_
