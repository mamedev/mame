// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Texture Fetch Unit (TF)
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/rdptpipe.h"
#include "video/n64.h"
#include "video/rgbutil.h"

#define RELATIVE(x, y)  ((((x) >> 3) - (y)) << 3) | (x & 7);

void n64_texture_pipe_t::set_machine(running_machine &machine)
{
	n64_state* state = machine.driver_data<n64_state>();

	m_rdp = state->m_rdp;

	for(INT32 i = 0; i < 0x10000; i++)
	{
		m_expand_16to32_table[i] = color_t((i & 1) ? 0xff : 0x00, m_rdp->m_replicated_rgba[(i >> 11) & 0x1f], m_rdp->m_replicated_rgba[(i >>  6) & 0x1f], m_rdp->m_replicated_rgba[(i >>  1) & 0x1f]);
	}

	for(UINT32 i = 0; i < 0x80000; i++)
	{
		if (i & 0x40000)
		{
			m_lod_lookup[i] = 0x7fff;
		}
		else if (i & 0x20000)
		{
			m_lod_lookup[i] = 0x8000;
		}
		else
		{
			if ((i & 0x18000) == 0x8000)
			{
				m_lod_lookup[i] = 0x7fff;
			}
			else if ((i & 0x18000) == 0x10000)
			{
				m_lod_lookup[i] = 0x8000;
			}
			else
			{
				m_lod_lookup[i] = i & 0xffff;
			}
		}
	}

	m_log2_table[0] = m_log2_table[1] = 0;
	for (INT32 i = 2; i < 256; i++)
	{
		for (INT32 k = 7; k > 0; k--)
		{
			if ((i >> k) & 1)
			{
				m_log2_table[i] = k;
				break;
			}
		}
	}

	m_st2_add.set(1, 0, 1, 0);
	m_v1.set(1, 1, 1, 1);
	m_yuv1.set(1, 0, 0, 0);
}

void n64_texture_pipe_t::tclod_tcclamp(INT32* sss, INT32* sst)
{
	INT32 temps = *sss;
	INT32 tempt = *sst;

	if (!(temps & 0x40000))
	{
		if (!(temps & 0x20000))
		{
			const INT32 tempanded = temps & 0x18000;
			if (tempanded != 0x8000)
			{
				if (tempanded != 0x10000)
				{
					*sss &= 0xffff;
				}
				else
				{
					*sss = 0x8000;
				}
			}
			else
			{
				*sss = 0x7fff;
			}
		}
		else
		{
			*sss = 0x8000;
		}
	}
	else
	{
		*sss = 0x7fff;
	}

	if (!(tempt & 0x40000))
	{
		if (!(tempt & 0x20000))
		{
			const INT32 tempanded = tempt & 0x18000;
			if (tempanded != 0x8000)
			{
				if (tempanded != 0x10000)
				{
					*sst &= 0xffff;
				}
				else
				{
					*sst = 0x8000;
				}
			}
			else
			{
				*sst = 0x7fff;
			}
		}
		else
		{
			*sst = 0x8000;
		}
	}
	else
	{
		*sst = 0x7fff;
	}
}

void n64_texture_pipe_t::tclod_1cycle_current(INT32* sss, INT32* sst, INT32 nexts, INT32 nextt, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	tclod_tcclamp(sss, sst);

	if (object.m_mode_derivs.m_do_lod)
	{
		INT32 fars, fart, farw;
		if (userdata->m_next_valid)
		{
			if (!userdata->m_end_span || !userdata->m_long_span)
			{
				if (!(userdata->m_pre_end_span && userdata->m_long_span) && !(userdata->m_end_span && userdata->m_mid_span))
				{
					fars = (s + (dsinc << 1)) >> 16;
					fart = (t + (dtinc << 1)) >> 16;
					farw = (w + (dwinc << 1)) >> 16;
				}
				else
				{
					fars = (s - dsinc) >> 16;
					fart = (t - dtinc) >> 16;
					farw = (w - dwinc) >> 16;
				}
			}
			else
			{
				fars = (userdata->m_next_s + dsinc) >> 16;
				fart = (userdata->m_next_t + dtinc) >> 16;
				farw = (userdata->m_next_w + dwinc) >> 16;
			}
		}
		else
		{
			fars = (s + (dsinc << 1)) >> 16;
			fart = (t + (dtinc << 1)) >> 16;
			farw = (w + (dwinc << 1)) >> 16;
		}

		if (object.m_other_modes.persp_tex_en)
		{
			m_rdp->tc_div(fars, fart, farw, &fars, &fart);
		}
		else
		{
			m_rdp->tc_div_no_perspective(fars, fart, farw, &fars, &fart);
		}

		const bool lodclamp = ((fars | nexts | fart | nextt) & 0x60000) ? true : false;

		INT32 lod;
		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		bool magnify, distant;
		UINT32 l_tile;
		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant, userdata, object);

		if (object.m_other_modes.tex_lod_en)
		{
			if (distant)
			{
				l_tile = object.m_misc_state.m_max_level;
			}

			if (object.m_other_modes.detail_tex_en || magnify)
			{
				*t1 = (prim_tile + l_tile) & 7;
			}
			else
			{
				*t1 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

void n64_texture_pipe_t::tclod_2cycle_current(INT32* sss, INT32* sst, INT32 nexts, INT32 nextt, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	tclod_tcclamp(sss, sst);

	if (object.m_mode_derivs.m_do_lod)
	{
		INT32 nextys = (s + object.m_span_base.m_span_dsdy) >> 16;
		INT32 nextyt = (t + object.m_span_base.m_span_dtdy) >> 16;
		INT32 nextyw = (w + object.m_span_base.m_span_dwdy) >> 16;

		if (object.m_other_modes.persp_tex_en)
		{
			m_rdp->tc_div(nextys, nextyt, nextyw, &nextys, &nextyt);
		}
		else
		{
			m_rdp->tc_div_no_perspective(nextys, nextyt, nextyw, &nextys, &nextyt);
		}

		INT32 inits = *sss;
		INT32 initt = *sst;
		const bool lodclamp = ((inits | nexts | nextys | initt | nextt | nextyt) & 0x60000) ? true : false;

		INT32 lod;
		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		bool magnify, distant;
		UINT32 l_tile;
		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant, userdata, object);

		if (object.m_other_modes.tex_lod_en)
		{
			if (distant)
			{
				l_tile = object.m_misc_state.m_max_level;
			}

			if (!object.m_other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!object.m_other_modes.sharpen_tex_en && magnify)))
				{
					*t2 = (*t1 + 1) & 7;
				}
				else
				{
					*t2 = *t1;
				}
			}
			else
			{
				if (!magnify)
				{
					*t1 = prim_tile + l_tile + 1;
				}
				else
				{
					*t1 = prim_tile + l_tile;
				}
				*t1 &= 7;

				if (!distant && !magnify)
				{
					*t2 = prim_tile + l_tile + 2;
				}
				else
				{
					*t2 = prim_tile + l_tile + 1;
				}
				*t2 &= 7;
			}
		}
	}
}

void n64_texture_pipe_t::tclod_1cycle_next(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* pre_lod_frac, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	tclod_tcclamp(sss, sst);

	if (object.m_mode_derivs.m_do_lod)
	{
		INT32 nexts, nextt, nextw, fars, fart, farw;
		if (userdata->m_next_valid)
		{
			if (userdata->m_next_span)
			{
				if (!userdata->m_end_span || !userdata->m_long_span)
				{
					nexts = (s + dsinc)>> 16;
					nextt = (t + dtinc)>> 16;
					nextw = (w + dwinc)>> 16;

					if (!(userdata->m_pre_end_span && userdata->m_long_span) && !(userdata->m_end_span && userdata->m_mid_span))
					{
						fars = (s + (dsinc << 1)) >> 16;
						fart = (t + (dtinc << 1)) >> 16;
						farw = (w + (dwinc << 1)) >> 16;
					}
					else
					{
						fars = (s - dsinc) >> 16;
						fart = (t - dtinc) >> 16;
						farw = (w - dwinc) >> 16;
					}
				}
				else
				{
					nexts = userdata->m_next_s;
					nextt = userdata->m_next_t;
					nextw = userdata->m_next_w;

					fars = (nexts + dsinc) >> 16;
					fart = (nextt + dtinc) >> 16;
					farw = (nextw + dwinc) >> 16;

					nexts >>= 16;
					nextt >>= 16;
					nextw >>= 16;
				}
			}
			else
			{
				if (!userdata->m_almost_mid_span)
				{
					nexts = userdata->m_next_s + dsinc;
					nextt = userdata->m_next_t + dtinc;
					nextw = userdata->m_next_w + dwinc;

					fars = (nexts + dsinc) >> 16;
					fart = (nextt + dtinc) >> 16;
					farw = (nextw + dwinc) >> 16;

					nexts >>= 16;
					nextt >>= 16;
					nextw >>= 16;
				}
				else
				{
					nexts = (s + dsinc) >> 16;
					nextt = (t + dtinc) >> 16;
					nextw = (w + dwinc) >> 16;

					fars = (s - dsinc) >> 16;
					fart = (t - dtinc) >> 16;
					farw = (w - dwinc) >> 16;
				}
			}
		}
		else
		{
			nexts = (s + dsinc) >> 16;
			nextt = (t + dtinc) >> 16;
			nextw = (w + dwinc) >> 16;

			fars = (s + (dsinc << 1)) >> 16;
			fart = (t + (dtinc << 1)) >> 16;
			farw = (w + (dwinc << 1)) >> 16;
		}

		if (object.m_other_modes.persp_tex_en)
		{
			m_rdp->tc_div(fars, fart, farw, &fars, &fart);
			m_rdp->tc_div(nexts, nextt, nextw, &nexts, &nextt);
		}
		else
		{
			m_rdp->tc_div_no_perspective(fars, fart, farw, &fars, &fart);
			m_rdp->tc_div_no_perspective(nexts, nextt, nextw, &nexts, &nextt);
		}

		const bool lodclamp = ((fars | nexts | fart | nextt) & 0x60000) ? true : false;

		INT32 lod;
		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		if ((lod & 0x4000) || lodclamp)
		{
			lod = 0x7fff;
		}
		else if (lod < object.m_misc_state.m_min_level)
		{
			lod = object.m_misc_state.m_min_level;
		}

		const bool magnify = (lod < 32);
		UINT32 l_tile = m_log2_table[(lod >> 5) & 0xff];
		const bool distant = (lod & 0x6000) || (l_tile >= object.m_misc_state.m_max_level);

		*pre_lod_frac = ((lod << 3) >> l_tile) & 0xff;

		if (!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
		{
			if (distant)
			{
				*pre_lod_frac = 0xff;
			}
			else
			{
				*pre_lod_frac = 0;
			}
		}

		if (object.m_other_modes.sharpen_tex_en && magnify)
		{
			*pre_lod_frac |= 0x100;
		}

		if (object.m_other_modes.tex_lod_en)
		{
			if (distant)
			{
				l_tile = object.m_misc_state.m_max_level;
			}

			if (!object.m_other_modes.detail_tex_en || magnify)
			{
				*t1 = (prim_tile + l_tile) & 7;
			}
			else
			{
				*t1 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

void n64_texture_pipe_t::tclod_2cycle_next(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2, INT32* pre_lod_frac, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	tclod_tcclamp(sss, sst);

	if (object.m_mode_derivs.m_do_lod)
	{
		INT32 inits = *sss;
		INT32 initt = *sst;

		INT32 nexts = (s + dsinc) >> 16;
		INT32 nextt = (t + dtinc) >> 16;
		INT32 nextw = (w + dwinc) >> 16;

		INT32 nextys = (s + object.m_span_base.m_span_dsdy) >> 16;
		INT32 nextyt = (t + object.m_span_base.m_span_dtdy) >> 16;
		INT32 nextyw = (w + object.m_span_base.m_span_dwdy) >> 16;

		if (object.m_other_modes.persp_tex_en)
		{
			m_rdp->tc_div(nexts, nextt, nextw, &nexts, &nextt);
			m_rdp->tc_div(nextys, nextyt, nextyw, &nextys, &nextyt);
		}
		else
		{
			m_rdp->tc_div_no_perspective(nexts, nextt, nextw, &nexts, &nextt);
			m_rdp->tc_div_no_perspective(nextys, nextyt, nextyw, &nextys, &nextyt);
		}

		const bool lodclamp = ((inits | nexts | nextys | initt | nextt | nextyt) & 0x60000) ? true : false;

		INT32 lod;
		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		if ((lod & 0x4000) || lodclamp)
		{
			lod = 0x7fff;
		}
		else if (lod < object.m_misc_state.m_min_level)
		{
			lod = object.m_misc_state.m_min_level;
		}

		const bool magnify = (lod < 32);
		UINT32 l_tile = m_log2_table[(lod >> 5) & 0xff];
		const bool distant = (lod & 0x6000) || (l_tile >= object.m_misc_state.m_max_level);

		*pre_lod_frac = ((lod << 3) >> l_tile) & 0xff;

		if (!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
		{
			if (distant)
			{
				*pre_lod_frac = 0xff;
			}
			else
			{
				*pre_lod_frac = 0;
			}
		}

		if (object.m_other_modes.sharpen_tex_en && magnify)
		{
			*pre_lod_frac |= 0x100;
		}

		if (object.m_other_modes.tex_lod_en)
		{
			if (distant)
			{
				l_tile = object.m_misc_state.m_max_level;
			}

			if (!object.m_other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!object.m_other_modes.sharpen_tex_en && magnify)))
				{
					*t2 = (*t1 + 1) & 7;
				}
				else
				{
					*t2 = *t1;
				}
			}
			else
			{
				if (!magnify)
				{
					*t1 = prim_tile + l_tile + 1;
				}
				else
				{
					*t1 = prim_tile + l_tile;
				}
				*t1 &= 7;

				if (!distant && !magnify)
				{
					*t2 = prim_tile + l_tile + 2;
				}
				else
				{
					*t2 = prim_tile + l_tile + 1;
				}
				*t2 &= 7;
			}
		}
	}
}

void n64_texture_pipe_t::lodfrac_lodtile_signals(bool lodclamp, INT32 lod, UINT32* l_tile, bool* magnify, bool* distant, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.m_misc_state.m_min_level)
	{
		lod = object.m_misc_state.m_min_level;
	}

	const bool mag = (lod < 32);
	UINT32 ltil = m_log2_table[(lod >> 5) & 0xff];
	const bool dis = (lod & 0x6000) || (ltil >= object.m_misc_state.m_max_level);

	INT32 lf = ((lod << 3) >> ltil) & 0xff;

	if (object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
	{
		if (dis)
		{
			lf = 0xff;
		}
		else
		{
			lf = 0;
		}
	}

	if (object.m_other_modes.sharpen_tex_en && mag)
	{
		lf |= 0x100;
	}

	*distant = dis;
	*l_tile = ltil;
	*magnify = mag;
	userdata->m_lod_fraction.set(lf, lf, lf, lf);
}

void n64_texture_pipe_t::tclod_4x17_to_15(INT32 scurr, INT32 snext, INT32 tcurr, INT32 tnext, INT32 previous, INT32* lod)
{
	INT32 dels = SIGN17(snext) - SIGN17(scurr);
	INT32 delt = SIGN17(tnext) - SIGN17(tcurr);
	if (dels & 0x20000)
	{
		dels = ~dels & 0x1ffff;
	}
	if (delt & 0x20000)
	{
		delt = ~delt & 0x1ffff;
	}

	dels = (dels > delt) ? dels : delt;
	dels = (previous > dels) ? previous : dels;
	*lod = dels & 0x7fff;
	if (dels & 0x1c000)
	{
		*lod |= 0x4000;
	}
}

void n64_texture_pipe_t::mask(rgbaint_t& sstt, const n64_tile_t& tile)
{
	UINT32 s_mask_bits = m_maskbits_table[tile.mask_s];
	UINT32 t_mask_bits = m_maskbits_table[tile.mask_t];
	rgbaint_t maskbits(s_mask_bits, s_mask_bits, t_mask_bits, t_mask_bits);

	rgbaint_t do_wrap(sstt);
	do_wrap.sra(tile.wrapped_mask);
	do_wrap.and_reg(m_v1);
	do_wrap.cmpeq(m_v1);
	do_wrap.and_reg(tile.mm);

	rgbaint_t wrapped(sstt);
	wrapped.xor_reg(do_wrap);
	wrapped.and_reg(maskbits);
	wrapped.and_reg(tile.mask);
	sstt.and_reg(tile.invmask);
	sstt.or_reg(wrapped);
}

rgbaint_t n64_texture_pipe_t::shift_cycle(rgbaint_t& st, const n64_tile_t& tile)
{
	st.sign_extend(0x00008000, 0xffff8000);
	st.sra(tile.rshift);
	st.shl(tile.lshift);

	rgbaint_t maxst(st);
	maxst.sra_imm(3);
	rgbaint_t maxst_eq(maxst);
	maxst.cmpgt(tile.sth);
	maxst_eq.cmpeq(tile.sth);
	maxst.or_reg(maxst_eq);

	rgbaint_t stlsb(st);
	stlsb.and_imm(7);

	st.sra_imm(3);
	st.sub(tile.stl);
	st.shl_imm(3);
	st.or_reg(stlsb);

	return maxst;
}

inline void n64_texture_pipe_t::shift_copy(rgbaint_t& st, const n64_tile_t& tile)
{
	st.shr(tile.rshift);
	st.shl(tile.lshift);
}

void n64_texture_pipe_t::clamp_cycle(rgbaint_t& st, rgbaint_t& stfrac, rgbaint_t& maxst, const INT32 tilenum, const n64_tile_t& tile, rdp_span_aux* userdata)
{
	rgbaint_t not_clamp(tile.clamp_st);
	not_clamp.xor_imm(0xffffffff);

	rgbaint_t highbit_mask(0x10000, 0x10000, 0x10000, 0x10000);
	rgbaint_t highbit(st);
	highbit.and_reg(highbit_mask);
	highbit.cmpeq(highbit_mask);

	rgbaint_t not_highbit(highbit);
	not_highbit.xor_imm(0xffffffff);

	rgbaint_t not_maxst(maxst);
	not_maxst.xor_imm(0xffffffff);
	not_maxst.and_reg(not_highbit);
	not_maxst.or_reg(not_clamp);

	rgbaint_t shifted_st(st);
	shifted_st.sign_extend(0x00010000, 0xffff0000);
	shifted_st.shr_imm(5);
	shifted_st.and_imm(0x1fff);
	shifted_st.and_reg(not_maxst);
	stfrac.and_reg(not_maxst);

	rgbaint_t clamp_diff(userdata->m_clamp_diff[tilenum]);
	clamp_diff.and_reg(tile.clamp_st);
	clamp_diff.and_reg(maxst);

	st.set(shifted_st);
	st.or_reg(clamp_diff);
}

void n64_texture_pipe_t::clamp_cycle_light(rgbaint_t& st, rgbaint_t& maxst, const INT32 tilenum, const n64_tile_t& tile, rdp_span_aux* userdata)
{
	rgbaint_t not_clamp(tile.clamp_st);
	not_clamp.xor_imm(0xffffffff);

	rgbaint_t highbit_mask(0x10000, 0x10000, 0x10000, 0x10000);
	rgbaint_t highbit(st);
	highbit.and_reg(highbit_mask);
	highbit.cmpeq(highbit_mask);

	rgbaint_t not_highbit(highbit);
	not_highbit.xor_imm(0xffffffff);

	rgbaint_t not_maxst(maxst);
	not_maxst.xor_imm(0xffffffff);
	not_maxst.and_reg(not_highbit);
	not_maxst.or_reg(not_clamp);

	rgbaint_t shifted_st(st);
	shifted_st.sign_extend(0x00010000, 0xffff0000);
	shifted_st.shr_imm(5);
	shifted_st.and_imm(0x1fff);
	shifted_st.and_reg(not_maxst);

	rgbaint_t clamp_diff(userdata->m_clamp_diff[tilenum]);
	clamp_diff.and_reg(tile.clamp_st);
	clamp_diff.and_reg(maxst);

	st.set(shifted_st);
	st.or_reg(clamp_diff);
}

void n64_texture_pipe_t::cycle_nearest(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t& tile = object.m_tiles[tilenum];
	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	rgbaint_t st(0, SSS, 0, SST);
	rgbaint_t maxst = shift_cycle(st, tile);
	clamp_cycle_light(st, maxst, tilenum, tile, userdata);
	mask(st, tile);

	UINT32 tbase = tile.tmem + ((tile.line * st.get_b32()) & 0x1ff);

	rgbaint_t t0;
	((this)->*(m_texel_fetch[index]))(t0, st.get_r32(), st.get_b32(), tbase, tile.palette, userdata);
	if (object.m_other_modes.convert_one && cycle)
	{
		t0.set(*prev);
	}

	if (tile.format == FORMAT_YUV)
	{
		t0.sign_extend(0x00000100, 0xffffff00);
	}

	rgbaint_t k13r(m_rdp->get_k13());
	k13r.mul_imm(t0.get_r32());

	TEX->set(m_rdp->get_k02());
	TEX->mul_imm(t0.get_g32());
	TEX->add(k13r);
	TEX->add_imm(0x80);
	TEX->shr_imm(8);
	TEX->add_imm(t0.get_b32());
	TEX->and_imm(0x1ff);
}

void n64_texture_pipe_t::cycle_nearest_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t& tile = object.m_tiles[tilenum];
	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	rgbaint_t st(0, SSS, 0, SST);
	rgbaint_t maxst = shift_cycle(st, tile);
	clamp_cycle_light(st, maxst, tilenum, tile, userdata);
	mask(st, tile);

	UINT32 tbase = tile.tmem + ((tile.line * st.get_b32()) & 0x1ff);

	((this)->*(m_texel_fetch[index]))(*TEX, st.get_r32(), st.get_b32(), tbase, tile.palette, userdata);

	if (object.m_other_modes.convert_one && cycle)
	{
		const UINT32 prev_b = prev->get_b32();
		TEX->set(prev_b, prev_b, prev_b, prev_b);
	}
}

void n64_texture_pipe_t::cycle_linear(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t& tile = object.m_tiles[tilenum];
	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	rgbaint_t st(0, SSS, 0, SST);
	rgbaint_t maxst = shift_cycle(st, tile);
	rgbaint_t stfrac(st);
	stfrac.and_imm(0x1f);

	clamp_cycle(st, stfrac, maxst, tilenum, tile, userdata);

	mask(st, tile);

	const UINT32 tbase = tile.tmem + ((tile.line * st.get_b32()) & 0x1ff);

	rgbaint_t t0;
	((this)->*(m_texel_fetch[index]))(t0, st.get_r32(), st.get_b32(), tbase, tile.palette, userdata);
	if (object.m_other_modes.convert_one && cycle)
	{
		t0.set(*prev);
	}

	t0.sign_extend(0x00000100, 0xffffff00);

	rgbaint_t k13r(m_rdp->get_k13());
	k13r.mul_imm(t0.get_r32());

	TEX->set(m_rdp->get_k02());
	TEX->mul_imm(t0.get_g32());
	TEX->add(k13r);
	TEX->add_imm(0x80);
	TEX->shr_imm(8);
	TEX->add_imm(t0.get_b32());
	TEX->and_imm(0x1ff);
}

void n64_texture_pipe_t::cycle_linear_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t& tile = object.m_tiles[tilenum];

	UINT32 tpal = tile.palette;
	UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	rgbaint_t sstt(SSS, SSS, SST, SST);
	rgbaint_t maxst = shift_cycle(sstt, tile);
	rgbaint_t stfrac = sstt;
	stfrac.and_imm(0x1f);

	clamp_cycle(sstt, stfrac, maxst, tilenum, tile, userdata);

	sstt.add(m_st2_add);

	mask(sstt, tile);

	const UINT32 tbase1 = tile.tmem + ((tile.line * sstt.get_b32()) & 0x1ff);
	const UINT32 tbase2 = tile.tmem + ((tile.line * sstt.get_g32()) & 0x1ff);

	const UINT32 sfrac = stfrac.get_r32();
	const UINT32 tfrac = stfrac.get_b32();

	rgbaint_t t2;
	((this)->*(m_texel_fetch[index]))(*TEX, sstt.get_a32(), sstt.get_b32(), tbase1, tpal, userdata);
	((this)->*(m_texel_fetch[index]))(t2, sstt.get_r32(), sstt.get_g32(), tbase2, tpal, userdata);

	if ((sfrac != 0x10) || (tfrac != 0x10) || !object.m_other_modes.mid_texel)
	{
		if ((sfrac + tfrac) >= 0x20)
		{
			rgbaint_t invstf(stfrac);
			invstf.set(stfrac);
			invstf.subr_imm(0x20);
			invstf.shl_imm(3);

			rgbaint_t t3;
			((this)->*(m_texel_fetch[index]))(t3, sstt.get_a32(), sstt.get_g32(), tbase2, tpal, userdata);

			TEX->sub(t3);
			t2.sub(t3);

			TEX->mul_imm(invstf.get_b32());
			t2.mul_imm(invstf.get_r32());

			TEX->add(t2);
			TEX->add_imm(0x0080);
			TEX->sra_imm(8);
			TEX->add(t3);
		}
		else
		{
			stfrac.shl_imm(3);

			rgbaint_t t0;
			((this)->*(m_texel_fetch[index]))(t0, sstt.get_r32(), sstt.get_b32(), tbase1, tpal, userdata);

			TEX->sub(t0);
			t2.sub(t0);

			TEX->mul_imm(stfrac.get_r32());
			t2.mul_imm(stfrac.get_b32());

			TEX->add(t2);
			TEX->add_imm(0x80);
			TEX->sra_imm(8);
			TEX->add(t0);
		}
	}
	else
	{
		rgbaint_t t0, t3;
		((this)->*(m_texel_fetch[index]))(t0, sstt.get_r32(), sstt.get_b32(), tbase1, tpal, userdata);
		((this)->*(m_texel_fetch[index]))(t3, sstt.get_a32(), sstt.get_g32(), tbase2, tpal, userdata);
		TEX->add(t0);
		TEX->add(t2);
		TEX->add(t3);
		TEX->sra(2);
	}
}

void n64_texture_pipe_t::copy(color_t* TEX, INT32 SSS, INT32 SST, UINT32 tilenum, const rdp_poly_state& object, rdp_span_aux* userdata)
{
	const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = tiles[tilenum];

	rgbaint_t st(0, SSS, 0, SST);
	shift_copy(st, tile);
	rgbaint_t stlsb(st);
	stlsb.and_imm(7);
	st.shr_imm(3);
	st.sub(rgbaint_t(0, tile.sl, 0, tile.tl));
	st.shl_imm(3);
	st.add(stlsb);
	st.sign_extend(0x00010000, 0xffff0000);
	st.shr_imm(5);
	st.and_imm(0x1fff);
	mask(st, tile);

	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;
	const UINT32 tbase = tile.tmem + ((tile.line * st.get_b32()) & 0x1ff);
	((this)->*(m_texel_fetch[index]))(*TEX, st.get_r32(), st.get_b32(), tbase, tile.palette, userdata);
}

void n64_texture_pipe_t::lod_1cycle(INT32* sss, INT32* sst, const INT32 s, const INT32 t, const INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.m_other_modes.persp_tex_en)
	{
		m_rdp->tc_div(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->tc_div_no_perspective(nexts, nextt, nextsw, &nexts, &nextt);
	}

	userdata->m_start_span = false;
	userdata->m_precomp_s = nexts;
	userdata->m_precomp_t = nextt;

	const INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

	INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(*sss & 0x1ffff);
	INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(*sst & 0x1ffff);
	if (horstep & 0x20000)
	{
		horstep = ~horstep & 0x1ffff;
	}
	if (vertstep & 0x20000)
	{
		vertstep = ~vertstep & 0x1ffff;
	}

	INT32 lod = (horstep >= vertstep) ? horstep : vertstep;

	*sss = m_lod_lookup[*sss & 0x7ffff];
	*sst = m_lod_lookup[*sst & 0x7ffff];

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.m_misc_state.m_min_level)
	{
		lod = object.m_misc_state.m_min_level;
	}

	INT32 l_tile = m_rdp->get_log2((lod >> 5) & 0xff);
	const bool magnify = (lod < 32);
	const bool distant = ((lod & 0x6000) || (l_tile >= object.m_misc_state.m_max_level));

	UINT8 lod_fraction = ((lod << 3) >> l_tile) & 0xff;

	if(!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
	{
		if (distant)
		{
			lod_fraction = 0xff;
		}
		else if (magnify)
		{
			lod_fraction = 0;
		}
	}

	userdata->m_lod_fraction.set(lod_fraction, lod_fraction, lod_fraction, lod_fraction);
	/* FIXME: ???
	if(object.m_other_modes.sharpen_tex_en && magnify)
	{
	    userdata->m_lod_fraction |= 0x100;
	}
	*/
}

void n64_texture_pipe_t::lod_2cycle(INT32* sss, INT32* sst, const INT32 s, const INT32 t, const INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, const INT32 prim_tile, INT32* t1, INT32* t2, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.m_other_modes.persp_tex_en)
	{
		m_rdp->tc_div(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->tc_div_no_perspective(nexts, nextt, nextsw, &nexts, &nextt);
	}

	userdata->m_start_span = false;
	userdata->m_precomp_s = nexts;
	userdata->m_precomp_t = nextt;

	const INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

	INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(*sss & 0x1ffff);
	INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(*sst & 0x1ffff);
	if (horstep & 0x20000)
	{
		horstep = ~horstep & 0x1ffff;
	}
	if (vertstep & 0x20000)
	{
		vertstep = ~vertstep & 0x1ffff;
	}

	INT32 lod = (horstep >= vertstep) ? horstep : vertstep;

	*sss = m_lod_lookup[*sss & 0x7ffff];
	*sst = m_lod_lookup[*sst & 0x7ffff];

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.m_misc_state.m_min_level)
	{
		lod = object.m_misc_state.m_min_level;
	}

	INT32 l_tile = m_rdp->get_log2((lod >> 5) & 0xff);
	const bool magnify = (lod < 32);
	const bool distant = ((lod & 0x6000) || (l_tile >= object.m_misc_state.m_max_level));

	UINT8 lod_fraction = ((lod << 3) >> l_tile) & 0xff;

	if(!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
	{
		if (distant)
		{
			lod_fraction = 0xff;
		}
		else if (magnify)
		{
			lod_fraction = 0;
		}
	}

	userdata->m_lod_fraction.set(lod_fraction, lod_fraction, lod_fraction, lod_fraction);

	/* FIXME: ???
	if(object.m_other_modes.sharpen_tex_en && magnify)
	{
	    userdata->m_lod_fraction |= 0x100;
	}*/

	if (object.m_other_modes.tex_lod_en)
	{
		if (distant)
		{
			l_tile = object.m_misc_state.m_max_level;
		}
		if (!object.m_other_modes.detail_tex_en)
		{
			*t1 = (prim_tile + l_tile) & 7;
			if (!(distant || (!object.m_other_modes.sharpen_tex_en && magnify)))
			{
				*t2 = (*t1 + 1) & 7;
			}
			else
			{
				*t2 = *t1; // World Driver Championship, Stunt Race 64, Beetle Adventure Racing
			}
		}
		else // Beetle Adventure Racing, World Driver Championship (ingame_, NFL Blitz 2001, Pilotwings
		{
			if (!magnify)
			{
				*t1 = (prim_tile + l_tile + 1);
			}
			else
			{
				*t1 = (prim_tile + l_tile);
			}
			*t1 &= 7;
			if (!distant && !magnify)
			{
				*t2 = (prim_tile + l_tile + 2) & 7;
			}
			else
			{
				*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

void n64_texture_pipe_t::lod_2cycle_limited(INT32* sss, INT32* sst, const INT32 s, const INT32 t, const INT32 w, const INT32 dsinc, const INT32 dtinc, const INT32 dwinc, const INT32 prim_tile, INT32* t1, const rdp_poly_state& object)
{
	const INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (object.m_other_modes.persp_tex_en)
	{
		m_rdp->tc_div(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->tc_div_no_perspective(nexts, nextt, nextsw, &nexts, &nextt);
	}

	const INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

	INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(*sss & 0x1ffff);
	INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(*sst & 0x1ffff);
	if (horstep & 0x20000)
	{
		horstep = ~horstep & 0x1ffff;
	}
	if (vertstep & 0x20000)
	{
		vertstep = ~vertstep & 0x1ffff;
	}

	INT32 lod = (horstep >= vertstep) ? horstep : vertstep;

	*sss = m_lod_lookup[*sss & 0x7ffff];
	*sst = m_lod_lookup[*sst & 0x7ffff];

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < object.m_misc_state.m_min_level)
	{
		lod = object.m_misc_state.m_min_level;
	}

	INT32 l_tile = m_rdp->get_log2((lod >> 5) & 0xff);
	const bool magnify = (lod < 32);
	const bool distant = (lod & 0x6000) || (l_tile >= object.m_misc_state.m_max_level);

	if (object.m_other_modes.tex_lod_en)
	{
		if (distant)
		{
			l_tile = object.m_misc_state.m_max_level;
		}
		if (!object.m_other_modes.detail_tex_en)
		{
			*t1 = (prim_tile + l_tile) & 7;
		}
		else
		{
			if (!magnify)
			{
				*t1 = (prim_tile + l_tile + 1);
			}
			else
			{
				*t1 = (prim_tile + l_tile);
			}
			*t1 &= 7;
		}
	}
}

void n64_texture_pipe_t::calculate_clamp_diffs(UINT32 prim_tile, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	if (userdata == NULL)
	{
		printf("Whoa, userdata is NULL!\n"); fflush(stdout);
	}
	const n64_tile_t* tiles = object.m_tiles;
	if (object.m_other_modes.cycle_type == CYCLE_TYPE_2)
	{
		if (object.m_other_modes.tex_lod_en)
		{
			for (INT32 start = 0; start <= 7; start++)
			{
				userdata->m_clamp_diff[start].set((tiles[start].sh >> 2) - (tiles[start].sl >> 2), (tiles[start].sh >> 2) - (tiles[start].sl >> 2), (tiles[start].th >> 2) - (tiles[start].tl >> 2), (tiles[start].th >> 2) - (tiles[start].tl >> 2));
			}
		}
		else
		{
			const INT32 start = prim_tile;
			const INT32 end = (prim_tile + 1) & 7;
			userdata->m_clamp_diff[start].set((tiles[start].sh >> 2) - (tiles[start].sl >> 2), (tiles[start].sh >> 2) - (tiles[start].sl >> 2), (tiles[start].th >> 2) - (tiles[start].tl >> 2), (tiles[start].th >> 2) - (tiles[start].tl >> 2));
			userdata->m_clamp_diff[end].set((tiles[end].sh >> 2) - (tiles[end].sl >> 2), (tiles[end].sh >> 2) - (tiles[end].sl >> 2), (tiles[end].th >> 2) - (tiles[end].tl >> 2), (tiles[end].th >> 2) - (tiles[end].tl >> 2));
		}
	}
	else//1-cycle or copy
	{
		userdata->m_clamp_diff[prim_tile].set((tiles[prim_tile].sh >> 2) - (tiles[prim_tile].sl >> 2), (tiles[prim_tile].sh >> 2) - (tiles[prim_tile].sl >> 2), (tiles[prim_tile].th >> 2) - (tiles[prim_tile].tl >> 2), (tiles[prim_tile].th >> 2) - (tiles[prim_tile].tl >> 2));
	}
}

#define USE_64K_LUT (1)

static INT32 sTexAddrSwap16[2] = { WORD_ADDR_XOR, WORD_XOR_DWORD_SWAP };
static INT32 sTexAddrSwap8[2] = { BYTE_ADDR_XOR, BYTE_XOR_DWORD_SWAP };

void n64_texture_pipe_t::fetch_rgba16_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_rgba16_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_rgba16_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	const UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_rgba32_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT32 *tc = ((UINT32*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_rgba32_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT32 *tc = ((UINT32*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_rgba32_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	const UINT16 cl = ((UINT16*)userdata->m_tmem)[taddr];
	const UINT16 ch = ((UINT16*)userdata->m_tmem)[taddr | 0x400];

	out.set(ch & 0xff, cl >> 8, cl & 0xff, ch >> 8);
}

void n64_texture_pipe_t::fetch_nop(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata) { }

void n64_texture_pipe_t::fetch_yuv(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);

	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;
	const INT32 taddrlow = ((taddr >> 1) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	const UINT16 c = tc[taddrlow];

	INT32 y = userdata->m_tmem[taddr | 0x800];
	INT32 u = c >> 8;
	INT32 v = c & 0xff;

	v ^= 0x80; u ^= 0x80;
	u |= ((u & 0x80) << 1);
	v |= ((v & 0x80) << 1);

	out.set(y & 0xff, y & 0xff, u & 0xff, v & 0xff);
}

void n64_texture_pipe_t::fetch_ci4_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_ci4_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_ci4_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	p = (tpal << 4) | p;

	out.set(p, p, p, p);
}

void n64_texture_pipe_t::fetch_ci8_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_ci8_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_ci8_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 p = tc[taddr];
	out.set(p, p, p, p);
}

void n64_texture_pipe_t::fetch_ia4_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_ia4_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_ia4_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT8 i = p & 0xe;
	i = (i << 4) | (i << 1) | (i >> 2);

	out.set((p & 1) * 0xff, i, i, i);
}

void n64_texture_pipe_t::fetch_ia8_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_ia8_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_ia8_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 p = tc[taddr];
	UINT8 i = p & 0xf0;
	i |= (i >> 4);

	out.set(((p << 4) | (p & 0xf)) & 0xff, i, i, i);
}

void n64_texture_pipe_t::fetch_ia16_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[c]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_ia16_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	const UINT8 k = (c >> 8) & 0xff;
	out.set(c & 0xff, k, k, k);
}

void n64_texture_pipe_t::fetch_ia16_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	const UINT16 c = tc[taddr];
	const UINT8 i = (c >> 8);
	out.set(c & 0xff, i, i, i);
}

void n64_texture_pipe_t::fetch_i4_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 byteval = tc[taddr];
	const UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[k]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_i4_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 byteval = tc[taddr];
	const UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

	const UINT8 i = (k >> 8) & 0xff;
	out.set(k & 0xff, i, i, i);
}

void n64_texture_pipe_t::fetch_i4_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 byteval = tc[taddr];
	UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	c |= (c << 4);

	out.set(c, c, c, c);
}

void n64_texture_pipe_t::fetch_i8_tlut0(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 c = tc[taddr];
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

#if USE_64K_LUT
	out.set(m_expand_16to32_table[k]);
#else
	out.set((c & 1) * 0xff, GET_HI_RGBA16_TMEM(c), GET_MED_RGBA16_TMEM(c), GET_LOW_RGBA16_TMEM(c));
#endif
}

void n64_texture_pipe_t::fetch_i8_tlut1(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 c = tc[taddr];
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

	const UINT8 i = (k >> 8) & 0xff;
	out.set(k & 0xff, i, i, i);
}

void n64_texture_pipe_t::fetch_i8_raw(rgbaint_t& out, INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 c = tc[taddr];

	out.set(c, c, c, c);
}
