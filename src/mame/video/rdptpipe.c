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
#include "video/n64.h"

#define RELATIVE(x, y)  ((((x) >> 3) - (y)) << 3) | (x & 7);

void n64_texture_pipe_t::set_machine(running_machine &machine)
{
	n64_state* state = machine.driver_data<n64_state>();

	m_rdp = state->m_rdp;

	for(INT32 i = 0; i < 0x10000; i++)
	{
		color_t c;
		c.i.r = m_rdp->m_replicated_rgba[(i >> 11) & 0x1f];
		c.i.g = m_rdp->m_replicated_rgba[(i >>  6) & 0x1f];
		c.i.b = m_rdp->m_replicated_rgba[(i >>  1) & 0x1f];
		c.i.a = (i & 1) ? 0xff : 0x00;
		m_expand_16to32_table[i] = c.c;
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
}

void n64_texture_pipe_t::mask(INT32* S, INT32* T, const n64_tile_t& tile)
{
	if (tile.mask_s)
	{
		INT32 wrap = *S >> tile.wrapped_mask_s;
		wrap &= 1;
		if (tile.ms && wrap)
		{
			*S = (~(*S));
		}
		*S &= m_maskbits_table[tile.mask_s];
	}

	if (tile.mask_t)
	{
		INT32 wrap = *T >> tile.wrapped_mask_t;
		wrap &= 1;
		if (tile.mt && wrap)
		{
			*T = (~(*T));
		}
		*T &= m_maskbits_table[tile.mask_t];
	}
}

void n64_texture_pipe_t::mask_coupled(INT32* S, INT32* S1, INT32* T, INT32* T1, const n64_tile_t& tile)
{
	if (tile.mask_s)
	{
		const INT32 maskbits_s = m_maskbits_table[tile.mask_s];
		if (tile.ms)
		{
			const INT32 swrapthreshold = tile.mask_s > 10 ? 10 : tile.mask_s;
			const INT32 wrap = (*S >> swrapthreshold) & 1;
			const INT32 wrap1 = (*S1 >> swrapthreshold) & 1;
			if (wrap)
			{
				*S = (~(*S));
			}
			if (wrap1)
			{
				*S1 = (~(*S1));
			}
		}
		*S &= maskbits_s;
		*S1 &= maskbits_s;
	}

	if (tile.mask_t)
	{
		const INT32 maskbits_t = m_maskbits_table[tile.mask_t];
		if (tile.mt)
		{
			const INT32 twrapthreshold = tile.mask_t > 10 ? 10 : tile.mask_t;
			const INT32 wrap = (*T >> twrapthreshold) & 1;
			const INT32 wrap1 = (*T1 >> twrapthreshold) & 1;
			if (wrap)
			{
				*T = (~(*T));
			}
			if (wrap1)
			{
				*T1 = (~(*T1));
			}
		}
		*T &= maskbits_t;
		*T1 &= maskbits_t;
	}
}

void n64_texture_pipe_t::shift_cycle(INT32* S, INT32* T, bool* maxs, bool* maxt, const n64_tile_t& tile)
{
	INT32 sss = (INT32)(INT16)*S;
	sss >>= tile.rshift_s;
	sss <<= tile.lshift_s;
	sss = (INT32)(INT16)sss;
	*maxs = ((sss >> 3) >= tile.sh);
	*S = (((sss >> 3) - tile.sl) << 3) | (sss & 7);

	INT32 sst = (INT32)(INT16)*T;
	sst >>= tile.rshift_t;
	sst <<= tile.lshift_t;
	*T = (INT32)(INT16)sst;
	*maxt = ((*T >> 3) >= tile.th);
	*T = (((sst >> 3) - tile.tl) << 3) | (sst & 7);
}

void n64_texture_pipe_t::shift_copy(INT32* S, INT32* T, const n64_tile_t& tile)
{
	INT32 sss = (INT32)(INT16)*S;
	sss >>= tile.rshift_s;
	sss <<= tile.lshift_s;
	*S = (INT32)(INT16)sss;

	INT32 sst = (INT32)(INT16)*T;
	sst >>= tile.rshift_t;
	sst <<= tile.lshift_t;
	*T = (INT32)(INT16)sst;
}

void n64_texture_pipe_t::clamp_cycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, const bool maxs, const bool maxt, const INT32 tilenum, const n64_tile_t& tile)
{
	if (tile.clamp_s)
	{
		if (*S & 0x10000)
		{
			*S = 0;
			*SFRAC = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[tilenum];
			*SFRAC = 0;
		}
		else
		{
			*S = (SIGN17(*S) >> 5) & 0x1fff;
		}
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (tile.clamp_t)
	{
		if (*T & 0x10000)
		{
			*T = 0;
			*TFRAC = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[tilenum];
			*TFRAC = 0;
		}
		else
		{
			*T = (SIGN17(*T) >> 5) & 0x1fff;
		}
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

void n64_texture_pipe_t::clamp_cycle_light(INT32* S, INT32* T, const bool maxs, const bool maxt, const INT32 tilenum, const n64_tile_t& tile)
{
	if (tile.clamp_s)
	{
		if (*S & 0x10000)
		{
			*S = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[tilenum];
		}
		else
		{
			*S = (SIGN17(*S) >> 5) & 0x1fff;
		}
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (tile.clamp_t)
	{
		if (*T & 0x10000)
		{
			*T = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[tilenum];
		}
		else
		{
			*T = (SIGN17(*T) >> 5) & 0x1fff;
		}
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

void n64_texture_pipe_t::cycle_nearest(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	//  const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = object.m_tiles[tilenum];
	const UINT32 tformat = tile.format;
	const UINT32 tsize =  tile.size;
	const UINT32 tpal = tile.palette;
	const UINT32 index = (tformat << 4) | (tsize << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	color_t t0;

	INT32 sss1 = SSS, sst1 = SST;
	bool maxs, maxt;
	shift_cycle(&sss1, &sst1, &maxs, &maxt, tile);
	clamp_cycle_light(&sss1, &sst1, maxs, maxt, tilenum, tile);
	mask(&sss1, &sst1, tile);

	UINT32 tbase = tile.tmem + ((tile.line * sst1) & 0x1ff);

	t0.c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, tbase, tpal, userdata);

	const INT32 newk0 = SIGN9(m_rdp->get_k0());
	const INT32 newk1 = SIGN9(m_rdp->get_k1());
	const INT32 newk2 = SIGN9(m_rdp->get_k2());
	const INT32 newk3 = SIGN9(m_rdp->get_k3());
	const INT32 invk0 = ~newk0;
	const INT32 invk1 = ~newk1;
	const INT32 invk2 = ~newk2;
	const INT32 invk3 = ~newk3;
	if (object.m_other_modes.convert_one && cycle)
	{
		t0 = *prev;
	}
	t0.i.r = SIGN9(t0.i.r);
	t0.i.g = SIGN9(t0.i.g);
	t0.i.b = SIGN9(t0.i.b);
	TEX->i.r = t0.i.b + (((newk0 - invk0) * t0.i.g + 0x80) >> 8);
	TEX->i.g = t0.i.b + (((newk1 - invk1) * t0.i.r + (newk2 - invk2) * t0.i.g + 0x80) >> 8);
	TEX->i.b = t0.i.b + (((newk3 - invk3) * t0.i.r + 0x80) >> 8);
	TEX->i.a = t0.i.b;
	TEX->i.r &= 0x1ff;
	TEX->i.g &= 0x1ff;
	TEX->i.b &= 0x1ff;
	TEX->i.a &= 0x1ff;
}

void n64_texture_pipe_t::cycle_nearest_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = tiles[tilenum];
	const UINT32 tformat = tile.format;
	const UINT32 tsize =  tile.size;
	const UINT32 tpal = tile.palette;
	const UINT32 index = (tformat << 4) | (tsize << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	color_t t0;

	INT32 sss1 = SSS, sst1 = SST;
	bool maxs, maxt;
	shift_cycle(&sss1, &sst1, &maxs, &maxt, tile);
	clamp_cycle_light(&sss1, &sst1, maxs, maxt, tilenum, tile);
	mask(&sss1, &sst1, tile);

	UINT32 tbase = tile.tmem + ((tile.line * sst1) & 0x1ff);

	(*TEX).c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, tbase, tpal, userdata);
}

void n64_texture_pipe_t::cycle_linear(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = tiles[tilenum];

	const UINT32 tpal = tile.palette;
	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	INT32 sss1 = SSS, sst1 = SST;
	bool maxs, maxt;
	shift_cycle(&sss1, &sst1, &maxs, &maxt, tile);

	INT32 sfrac = sss1 & 0x1f;
	INT32 tfrac = sst1 & 0x1f;

	clamp_cycle(&sss1, &sst1, &sfrac, &tfrac, maxs, maxt, tilenum, tile);

	INT32 sss2 = sss1 + 1;
	INT32 sst2 = sst1 + 1;

	mask_coupled(&sss1, &sss2, &sst1, &sst2, tile);

	const UINT32 tbase = tile.tmem + ((tile.line * sst1) & 0x1ff);

	bool upper = ((sfrac + tfrac) >= 0x20);

	INT32 invsf = 0;
	INT32 invtf = 0;
	if (upper)
	{
		invsf = 0x20 - sfrac;
		invsf <<= 3;

		invtf = 0x20 - tfrac;
		invtf <<= 3;
	}

	sfrac <<= 3;
	tfrac <<= 3;

	color_t t0;
	t0.c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, tbase, tpal, userdata);
	const INT32 newk0 = SIGN9(m_rdp->get_k0());
	const INT32 newk1 = SIGN9(m_rdp->get_k1());
	const INT32 newk2 = SIGN9(m_rdp->get_k2());
	const INT32 newk3 = SIGN9(m_rdp->get_k3());
	const INT32 invk0 = ~newk0;
	const INT32 invk1 = ~newk1;
	const INT32 invk2 = ~newk2;
	const INT32 invk3 = ~newk3;
	if (object.m_other_modes.convert_one && cycle)
	{
		t0 = *prev;
	}
	t0.i.r = SIGN9(t0.i.r); t0.i.g = SIGN9(t0.i.g); t0.i.b = SIGN9(t0.i.b);
	TEX->i.r = t0.i.b + (((newk0 - invk0) * t0.i.g + 0x80) >> 8);
	TEX->i.g = t0.i.b + (((newk1 - invk1) * t0.i.r + (newk2 - invk2) * t0.i.g + 0x80) >> 8);
	TEX->i.b = t0.i.b + (((newk3 - invk3) * t0.i.r + 0x80) >> 8);
	TEX->i.a = t0.i.b;
	TEX->i.r &= 0x1ff;
	TEX->i.g &= 0x1ff;
	TEX->i.b &= 0x1ff;
	TEX->i.a &= 0x1ff;
}

void n64_texture_pipe_t::cycle_linear_lerp(color_t* TEX, color_t* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux* userdata, const rdp_poly_state& object)
{
	const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = tiles[tilenum];

	UINT32 tpal = tile.palette;
	UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;

	INT32 sss1 = SSS, sst1 = SST;
	bool maxs, maxt;
	shift_cycle(&sss1, &sst1, &maxs, &maxt, tile);

	INT32 sfrac = sss1 & 0x1f;
	INT32 tfrac = sst1 & 0x1f;

	clamp_cycle(&sss1, &sst1, &sfrac, &tfrac, maxs, maxt, tilenum, tile);

	INT32 sss2 = sss1 + 1;
	INT32 sst2 = sst1 + 1;

	mask_coupled(&sss1, &sss2, &sst1, &sst2, tile);

	const UINT32 tbase1 = tile.tmem + ((tile.line * sst1) & 0x1ff);
	const UINT32 tbase2 = tile.tmem + ((tile.line * sst2) & 0x1ff);

	bool upper = ((sfrac + tfrac) >= 0x20);

	INT32 invsf = 0;
	INT32 invtf = 0;
	if (upper)
	{
		invsf = 0x20 - sfrac;
		invsf <<= 3;

		invtf = 0x20 - tfrac;
		invtf <<= 3;
	}

	INT32 center = (sfrac == 0x10) && (tfrac == 0x10) && object.m_other_modes.mid_texel;

	sfrac <<= 3;
	tfrac <<= 3;

	color_t t1;
	color_t t2;
	t1.c = ((this)->*(m_texel_fetch[index]))(sss2, sst1, tbase1, tpal, userdata);
	t2.c = ((this)->*(m_texel_fetch[index]))(sss1, sst2, tbase2, tpal, userdata);
	if (!center)
	{
		if (upper)
		{
			color_t t3;
			t3.c = ((this)->*(m_texel_fetch[index]))(sss2, sst2, tbase2, tpal, userdata);
			TEX->i.r = t3.i.r + (((invsf * (t2.i.r - t3.i.r)) + (invtf * (t1.i.r - t3.i.r)) + 0x80) >> 8);
			TEX->i.g = t3.i.g + (((invsf * (t2.i.g - t3.i.g)) + (invtf * (t1.i.g - t3.i.g)) + 0x80) >> 8);
			TEX->i.b = t3.i.b + (((invsf * (t2.i.b - t3.i.b)) + (invtf * (t1.i.b - t3.i.b)) + 0x80) >> 8);
			TEX->i.a = t3.i.a + (((invsf * (t2.i.a - t3.i.a)) + (invtf * (t1.i.a - t3.i.a)) + 0x80) >> 8);
		}
		else
		{
			color_t t0;
			t0.c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, tbase1, tpal, userdata);
			TEX->i.r = t0.i.r + (((sfrac * (t1.i.r - t0.i.r)) + (tfrac * (t2.i.r - t0.i.r)) + 0x80) >> 8);
			TEX->i.g = t0.i.g + (((sfrac * (t1.i.g - t0.i.g)) + (tfrac * (t2.i.g - t0.i.g)) + 0x80) >> 8);
			TEX->i.b = t0.i.b + (((sfrac * (t1.i.b - t0.i.b)) + (tfrac * (t2.i.b - t0.i.b)) + 0x80) >> 8);
			TEX->i.a = t0.i.a + (((sfrac * (t1.i.a - t0.i.a)) + (tfrac * (t2.i.a - t0.i.a)) + 0x80) >> 8);
		}
		TEX->i.r &= 0x1ff;
		TEX->i.g &= 0x1ff;
		TEX->i.b &= 0x1ff;
		TEX->i.a &= 0x1ff;
	}
	else
	{
		color_t t0;
		color_t t3;
		t0.c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, 1, tpal, userdata);
		t3.c = ((this)->*(m_texel_fetch[index]))(sss2, sst2, tbase2, tpal, userdata);
		TEX->i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
		TEX->i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
		TEX->i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
		TEX->i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
	}
}

void n64_texture_pipe_t::copy(color_t* TEX, INT32 SSS, INT32 SST, UINT32 tilenum, const rdp_poly_state& object, rdp_span_aux* userdata)
{
	const n64_tile_t* tiles = object.m_tiles;
	const n64_tile_t& tile = tiles[tilenum];

	INT32 sss1 = SSS;
	INT32 sst1 = SST;
	shift_copy(&sss1, &sst1, tile);
	sss1 = (((sss1 >> 3) - tile.sl) << 3) | (sss1 & 7);
	sss1 = (SIGN17(sss1) >> 5) & 0x1fff;

	sst1 = (((sst1 >> 3) - tile.tl) << 3) | (sst1 & 7);
	sst1 = (SIGN17(sst1) >> 5) & 0x1fff;
	mask(&sss1, &sst1, tile);

	const UINT32 index = (tile.format << 4) | (tile.size << 2) | ((UINT32) object.m_other_modes.en_tlut << 1) | (UINT32) object.m_other_modes.tlut_type;
	const UINT32 tbase = tile.tmem + ((tile.line * sst1) & 0x1ff);
	TEX->c = ((this)->*(m_texel_fetch[index]))(sss1, sst1, tbase, tile.palette, userdata);
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

	userdata->m_lod_fraction = ((lod << 3) >> l_tile) & 0xff;

	if(!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
	{
		if (distant)
		{
			userdata->m_lod_fraction = 0xff;
		}
		else if (magnify)
		{
			userdata->m_lod_fraction = 0;
		}
	}

	if(object.m_other_modes.sharpen_tex_en && magnify)
	{
		userdata->m_lod_fraction |= 0x100;
	}
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

	userdata->m_lod_fraction = ((lod << 3) >> l_tile) & 0xff;

	if(!object.m_other_modes.sharpen_tex_en && !object.m_other_modes.detail_tex_en)
	{
		if (distant)
		{
			userdata->m_lod_fraction = 0xff;
		}
		else if (magnify)
		{
			userdata->m_lod_fraction = 0;
		}
	}

	if(object.m_other_modes.sharpen_tex_en && magnify)
	{
		userdata->m_lod_fraction |= 0x100;
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

void n64_texture_pipe_t::calculate_clamp_diffs(UINT32 prim_tile, const rdp_poly_state& object)
{
	const n64_tile_t* tiles = object.m_tiles;
	if (object.m_other_modes.cycle_type == CYCLE_TYPE_2)
	{
		if (object.m_other_modes.tex_lod_en)
		{
			for (INT32 start = 0; start <= 7; start++)
			{
				m_clamp_s_diff[start] = (tiles[start].sh >> 2) - (tiles[start].sl >> 2);
				m_clamp_t_diff[start] = (tiles[start].th >> 2) - (tiles[start].tl >> 2);
			}
		}
		else
		{
			const INT32 start = prim_tile;
			const INT32 end = (prim_tile + 1) & 7;
			m_clamp_s_diff[start] = (tiles[start].sh >> 2) - (tiles[start].sl >> 2);
			m_clamp_t_diff[start] = (tiles[start].th >> 2) - (tiles[start].tl >> 2);
			m_clamp_s_diff[end] = (tiles[end].sh >> 2) - (tiles[end].sl >> 2);
			m_clamp_t_diff[end] = (tiles[end].th >> 2) - (tiles[end].tl >> 2);
		}
	}
	else//1-cycle or copy
	{
		m_clamp_s_diff[prim_tile] = (tiles[prim_tile].sh >> 2) - (tiles[prim_tile].sl >> 2);
		m_clamp_t_diff[prim_tile] = (tiles[prim_tile].th >> 2) - (tiles[prim_tile].tl >> 2);
	}
}

#define USE_64K_LUT (1)

static INT32 sTexAddrSwap16[2] = { WORD_ADDR_XOR, WORD_XOR_DWORD_SWAP };
static INT32 sTexAddrSwap8[2] = { BYTE_ADDR_XOR, BYTE_XOR_DWORD_SWAP };

UINT32 n64_texture_pipe_t::fetch_rgba16_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_rgba16_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;
	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_rgba16_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

#if USE_64K_LUT
	return m_expand_16to32_table[((UINT16*)userdata->m_tmem)[taddr]];
#else
	const UINT16 c = ((UINT16*)userdata->m_tmem)[taddr];
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_rgba32_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT32 *tc = ((UINT32*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_rgba32_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT32 *tc = ((UINT32*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT32 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 24) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_rgba32_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT32 c = ((UINT16*)userdata->m_tmem)[taddr];
	color_t color;
	color.i.r = (c >> 8) & 0xff;
	color.i.g = c & 0xff;
	c = ((UINT16*)userdata->m_tmem)[taddr | 0x400];
	color.i.b = (c >>  8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_nop(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata) { return 0; }

UINT32 n64_texture_pipe_t::fetch_yuv(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
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

	color_t color;
	color.i.r = u;
	color.i.g = v;
	color.i.b = y;
	color.i.a = y;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ci4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_ci4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ci4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	p = (tpal << 4) | p;

	color_t color;
	color.i.r = color.i.g = color.i.b = color.i.a = p;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ci8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_ci8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ci8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	color_t color;
	color.i.r = color.i.g = color.i.b = color.i.a = tc[taddr];

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_ia4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | p) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
	UINT8 i = p & 0xe;
	i = (i << 4) | (i << 1) | (i >> 2);

	color_t color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = (p & 1) * 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_ia8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 p = tc[taddr];
	const UINT16 c = ((UINT16*)(userdata->m_tmem + 0x800))[p << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 p = tc[taddr];
	UINT8 i = p & 0xf0;
	i |= (i >> 4);

	color_t color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = ((p & 0xf) << 4) | (p & 0xf);

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia16_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[c];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(c);
	color.i.g = GET_MED_RGBA16_TMEM(c);
	color.i.b = GET_LOW_RGBA16_TMEM(c);
	color.i.a = (c & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_ia16_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x3ff;

	UINT16 c = tc[taddr];
	c = ((UINT16*)(userdata->m_tmem + 0x800))[(c >> 8) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_ia16_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT16 *tc = ((UINT16*)userdata->m_tmem);
	const INT32 taddr = (((tbase << 2) + s) ^ sTexAddrSwap16[t & 1]) & 0x7ff;

	const UINT16 c = tc[taddr];
	const UINT8 i = (c >> 8);

	color_t color;
	color.i.r = i;
	color.i.g = i;
	color.i.b = i;
	color.i.a = c & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_i4_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 byteval = tc[taddr];
	const UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[k];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(k);
	color.i.g = GET_MED_RGBA16_TMEM(k);
	color.i.b = GET_LOW_RGBA16_TMEM(k);
	color.i.a = (k & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_i4_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 byteval = tc[taddr];
	const UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[((tpal << 4) | c) << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
	color.i.a = k & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_i4_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = ((((tbase << 4) + s) >> 1) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 byteval = tc[taddr];
	UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
	c |= (c << 4);

	color_t color;
	color.i.r = c;
	color.i.g = c;
	color.i.b = c;
	color.i.a = c;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_i8_tlut0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 c = tc[taddr];
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

#if USE_64K_LUT
	return m_expand_16to32_table[k];
#else
	color_t color;
	color.i.r = GET_HI_RGBA16_TMEM(k);
	color.i.g = GET_MED_RGBA16_TMEM(k);
	color.i.b = GET_LOW_RGBA16_TMEM(k);
	color.i.a = (k & 1) * 0xff;
	return color.c;
#endif
}

UINT32 n64_texture_pipe_t::fetch_i8_tlut1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0x7ff;

	const UINT8 c = tc[taddr];
	const UINT16 k = ((UINT16*)(userdata->m_tmem + 0x800))[c << 2];

	color_t color;
	color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
	color.i.a = k & 0xff;

	return color.c;
}

UINT32 n64_texture_pipe_t::fetch_i8_raw(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux* userdata)
{
	const UINT8 *tc = userdata->m_tmem;
	const INT32 taddr = (((tbase << 3) + s) ^ sTexAddrSwap8[t & 1]) & 0xfff;

	const UINT8 c = tc[taddr];

	color_t color;
	color.i.r = c;
	color.i.g = c;
	color.i.b = c;
	color.i.a = c;

	return color.c;
}
