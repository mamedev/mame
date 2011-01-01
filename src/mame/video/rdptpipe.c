#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define RELATIVE(x, y)	((((x) >> 3) - (y)) << 3) | (x & 7);

void TexturePipe::SetMachine(running_machine *machine)
{
	_n64_state *state = machine->driver_data<_n64_state>();

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();
	m_misc_state = m_rdp->GetMiscState();
}

void TexturePipe::Mask(INT32* S, INT32* T, INT32 num)
{
	Tile* tile = m_rdp->GetTiles();

	if (tile[num].mask_s)
	{
		INT32 wrap = *S >> (tile[num].mask_s > 10 ? 10 : tile[num].mask_s);
		wrap &= 1;
		if (tile[num].ms && wrap)
		{
			*S = (~(*S));
		}
		*S &= m_maskbits_table[tile[num].mask_s];
	}

	if (tile[num].mask_t)
	{
		INT32 wrap = *T >> (tile[num].mask_t > 10 ? 10 : tile[num].mask_t);
		wrap &= 1;
		if (tile[num].mt && wrap)
		{
			*T = (~(*T));
		}
		*T &= m_maskbits_table[tile[num].mask_t];
	}
}

void TexturePipe::MaskCoupled(INT32* S, INT32* S1, INT32* T, INT32* T1, INT32 num)
{
	Tile* tile = m_rdp->GetTiles();

	if (tile[num].mask_s)
	{
		INT32 maskbits_s = m_maskbits_table[tile[num].mask_s];
		if (tile[num].ms)
		{
			INT32 swrapthreshold = tile[num].mask_s > 10 ? 10 : tile[num].mask_s;
			INT32 wrap = (*S >> swrapthreshold) & 1;
			INT32 wrap1 = (*S1 >> swrapthreshold) & 1;
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

	if (tile[num].mask_t)
	{
		INT32 maskbits_t = m_maskbits_table[tile[num].mask_t];
		if (tile[num].mt)
		{
			INT32 twrapthreshold = tile[num].mask_t > 10 ? 10 : tile[num].mask_t;
			INT32 wrap = (*T >> twrapthreshold) & 1;
			INT32 wrap1 = (*T1 >> twrapthreshold) & 1;
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

void TexturePipe::ShiftCycle(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num)
{
	Tile* tile = m_rdp->GetTiles();
	*S = SIGN16(*S);
	*T = SIGN16(*T);
	if (tile[num].shift_s < 11)
	{
		*S >>= tile[num].shift_s;
	}
	else
	{
		*S <<= (16 - tile[num].shift_s);
	}
	*S = SIGN16(*S);
	if (tile[num].shift_t < 11)
	{
		*T >>= tile[num].shift_t;
	}
    else
    {
		*T <<= (16 - tile[num].shift_t);
	}
	*T = SIGN16(*T);

	*maxs = ((*S >> 3) >= tile[num].sh);
	*maxt = ((*T >> 3) >= tile[num].th);
}

void TexturePipe::ShiftCopy(INT32* S, INT32* T, UINT32 num)
{
	Tile* tile = m_rdp->GetTiles();
	*S = SIGN16(*S);
	*T = SIGN16(*T);
	if (tile[num].shift_s < 11)//?-? tcu_tile
	{
		*S >>= tile[num].shift_s;
	}
	else
	{
		*S <<= (16 - tile[num].shift_s);
	}
	*S = SIGN16(*S);
	if (tile[num].shift_t < 11)
	{
		*T >>= tile[num].shift_t;
	}
    else
    {
		*T <<= (16 - tile[num].shift_t);
	}
	*T = SIGN16(*T);
}

void TexturePipe::ClampCycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num)
{
	Tile* tile = m_rdp->GetTiles();
	int dos = tile[num].cs || !tile[num].mask_s;
	int dot = tile[num].ct || !tile[num].mask_t;

	if (dos)
	{
		if (*S & 0x10000)
		{
			*S = 0;
			*SFRAC = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[num];
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

	if (dot)
	{
		if (*T & 0x10000)
		{
			*T = 0;
			*TFRAC = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[num];
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

void TexturePipe::ClampCycleLight(INT32* S, INT32* T, bool maxs, bool maxt, INT32 num)
{
	Tile* tile = m_rdp->GetTiles();
	int dos = tile[num].cs || !tile[num].mask_s;
	int dot = tile[num].ct || !tile[num].mask_t;

	if (dos)
	{
		if (*S & 0x10000)
		{
			*S = 0;
		}
		else if (maxs)
		{
			*S = m_clamp_s_diff[num];
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

	if (dot)
	{
		if (*T & 0x10000)
		{
			*T = 0;
		}
		else if (maxt)
		{
			*T = m_clamp_t_diff[num];
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

void TexturePipe::Cycle(Color* TEX, Color* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle)
{
	Tile* tile = m_rdp->GetTiles();

#define TRELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);
	INT32 bilerp = cycle ? m_other_modes->bi_lerp1 : m_other_modes->bi_lerp0;
	int convert = m_other_modes->convert_one && cycle;
	Color t0;
	Color t1;
	Color t2;
	Color t3;
	if (m_other_modes->sample_type)
	{
		int sss1, sst1, sss2, sst2;

		INT32 invsf = 0;
		INT32 invtf = 0;
		int center = 0;

		sss1 = SSS;
		sst1 = SST;

		INT32 maxs;
		INT32 maxt;

		ShiftCycle(&sss1, &sst1, &maxs, &maxt, tilenum);

		sss1 = TRELATIVE(sss1, tile[tilenum].sl);
		sst1 = TRELATIVE(sst1, tile[tilenum].tl);

		INT32 sfrac = sss1 & 0x1f;
		INT32 tfrac = sst1 & 0x1f;

		ClampCycle(&sss1, &sst1, &sfrac, &tfrac, maxs, maxt, tilenum);

		sss2 = sss1 + 1;
		sst2 = sst1 + 1;

		MaskCoupled(&sss1, &sss2, &sst1, &sst2, tilenum);

		bool upper = ((sfrac + tfrac) >= 0x20);

		if (upper)
		{
			invsf = 0x20 - sfrac;
			invtf = 0x20 - tfrac;
		}

		center = (sfrac == 0x10) && (tfrac == 0x10) && m_other_modes->mid_texel;

		invsf <<= 3;
		invtf <<= 3;
		sfrac <<= 3;
		tfrac <<= 3;

		t0.c = Fetch(sss1, sst1, tilenum);

		if (bilerp)
		{
			t1.c = Fetch(sss2, sst1, tilenum);
			t2.c = Fetch(sss1, sst2, tilenum);
			t3.c = Fetch(sss2, sst2, tilenum);
			if (!center)
			{
				if (upper)
				{
					TEX->i.r = t3.i.r + (((invsf * (t2.i.r - t3.i.r)) + (invtf * (t1.i.r - t3.i.r)) + 0x80) >> 8);
					TEX->i.g = t3.i.g + (((invsf * (t2.i.g - t3.i.g)) + (invtf * (t1.i.g - t3.i.g)) + 0x80) >> 8);
					TEX->i.b = t3.i.b + (((invsf * (t2.i.b - t3.i.b)) + (invtf * (t1.i.b - t3.i.b)) + 0x80) >> 8);
					TEX->i.a = t3.i.a + (((invsf * (t2.i.a - t3.i.a)) + (invtf * (t1.i.a - t3.i.a)) + 0x80) >> 8);
				}
				else
				{
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
			else//tf.c,24
			{
				TEX->i.r = (t0.i.r + t1.i.r + t2.i.r + t3.i.r) >> 2;
				TEX->i.g = (t0.i.g + t1.i.g + t2.i.g + t3.i.g) >> 2;
				TEX->i.b = (t0.i.b + t1.i.b + t2.i.b + t3.i.b) >> 2;
				TEX->i.a = (t0.i.a + t1.i.a + t2.i.a + t3.i.a) >> 2;
			}
		}
		else
		{
			INT32 newk0 = SIGN9(m_rdp->GetK0());
			INT32 newk1 = SIGN9(m_rdp->GetK1());
			INT32 newk2 = SIGN9(m_rdp->GetK2());
			INT32 newk3 = SIGN9(m_rdp->GetK3());
			INT32 invk0 = ~newk0;
			INT32 invk1 = ~newk1;
			INT32 invk2 = ~newk2;
			INT32 invk3 = ~newk3;
			if (convert)
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
	}
	else
	{
		INT32 sss1 = SSS;
		INT32 sst1 = SST;

		INT32 maxs;
		INT32 maxt;

		ShiftCycle(&sss1, &sst1, &maxs, &maxt, tilenum);
		sss1 = TRELATIVE(sss1, tile[tilenum].sl);
		sst1 = TRELATIVE(sst1, tile[tilenum].tl);

		ClampCycleLight(&sss1, &sst1, maxs, maxt, tilenum);

        Mask(&sss1, &sst1, tilenum);

		t0.c = Fetch(sss1, sst1, tilenum);
		if (bilerp)
		{
			*TEX = t0;
		}
		else
		{
			INT32 newk0 = SIGN9(m_rdp->GetK0());
			INT32 newk1 = SIGN9(m_rdp->GetK1());
			INT32 newk2 = SIGN9(m_rdp->GetK2());
			INT32 newk3 = SIGN9(m_rdp->GetK3());
			INT32 invk0 = ~newk0;
			INT32 invk1 = ~newk1;
			INT32 invk2 = ~newk2;
			INT32 invk3 = ~newk3;
			if (convert)
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
	}
}

void TexturePipe::Copy(Color* TEX, INT32 SSS, INT32 SST, UINT32 tilenum)
{
	Tile* tile = m_rdp->GetTiles();
	INT32 sss1 = SSS;
	INT32 sst1 = SST;
	ShiftCopy(&sss1, &sst1, tilenum);
	sss1 = TRELATIVE(sss1, tile[tilenum].sl);
	sst1 = TRELATIVE(sst1, tile[tilenum].tl);
	sss1 = (SIGN17(sss1) >> 5) & 0x1fff;
	sst1 = (SIGN17(sst1) >> 5) & 0x1fff;
	Mask(&sss1, &sst1, tilenum);
	TEX->c = Fetch(sss1, sst1, tilenum);
}

void TexturePipe::LOD1Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (m_other_modes->persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	m_start_span = false;
	m_precomp_s = nexts;
	m_precomp_t = nextt;

	int tempanded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		tempanded = *sss & 0x18000;
		if (tempanded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		tempanded = *sst & 0x18000;
		if (tempanded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}
}

void TexturePipe::LOD2Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (m_other_modes->persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	m_start_span = false;
	m_precomp_s = nexts;
	m_precomp_t = nextt;

	INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

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

	INT32 temp_anded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		temp_anded = *sss & 0x18000;
		if (temp_anded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (temp_anded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		temp_anded = *sst & 0x18000;
		if (temp_anded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (temp_anded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < m_misc_state->m_min_level)
	{
		lod = m_misc_state->m_min_level;
	}

	bool magnify = (lod < 32);
	INT32 l_tile = m_rdp->GetLog2((lod >> 5) & 0xff);
	bool distant = ((lod & 0x6000) || (l_tile >= m_misc_state->m_max_level));

	m_rdp->SetLODFrac(((lod << 3) >> l_tile) & 0xff);

	if(!m_other_modes->sharpen_tex_en && !m_other_modes->detail_tex_en)
	{
		if (distant)
		{
			m_rdp->SetLODFrac(0xff);
		}
		else if (magnify)
		{
			m_rdp->SetLODFrac(0);
		}
	}

	if(m_other_modes->sharpen_tex_en && magnify)
	{
		m_rdp->SetLODFrac(m_rdp->GetLODFrac() | 0x100);
	}

	if (m_other_modes->tex_lod_en)
	{
		if (distant)
		{
			l_tile = m_misc_state->m_max_level;
		}
		if (!m_other_modes->detail_tex_en)
		{
			*t1 = (prim_tile + l_tile) & 7;
			if (!(distant || (!m_other_modes->sharpen_tex_en && magnify)))
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

void TexturePipe::LOD2CycleLimited(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1)
{
	INT32 nextsw = (w + dwinc) >> 16;
	INT32 nexts = (s + dsinc) >> 16;
	INT32 nextt = (t + dtinc) >> 16;

	if (m_other_modes->persp_tex_en)
	{
		m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
	}
	else
	{
		m_rdp->TCDivNoPersp(nexts, nextt, nextsw, &nexts, &nextt);
	}

	INT32 lodclamp = (((*sst & 0x60000) > 0) | ((nextt & 0x60000) > 0)) || (((*sss & 0x60000) > 0) | ((nexts & 0x60000) > 0));

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

	INT32 tempanded;
	if (*sss & 0x40000)
	{
		*sss = 0x7fff;
	}
	else if (*sss & 0x20000)
	{
		*sss = 0x8000;
	}
	else
	{
		tempanded = *sss & 0x18000;
		if (tempanded == 0x8000)
		{
			*sss = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sss = 0x8000;
		}
		else
		{
			*sss &= 0xffff;
		}
	}

	if (*sst & 0x40000)
	{
		*sst = 0x7fff;
	}
	else if (*sst & 0x20000)
	{
		*sst = 0x8000;
	}
	else
	{
		tempanded = *sst & 0x18000;
		if (tempanded == 0x8000)
		{
			*sst = 0x7fff;
		}
		else if (tempanded == 0x10000)
		{
			*sst = 0x8000;
		}
		else
		{
			*sst &= 0xffff;
		}
	}

	if ((lod & 0x4000) || lodclamp)
	{
		lod = 0x7fff;
	}
	else if (lod < m_misc_state->m_min_level)
	{
		lod = m_misc_state->m_min_level;
	}

	bool magnify = (lod < 32);
	INT32 l_tile = m_rdp->GetLog2((lod >> 5) & 0xff);
	bool distant = (lod & 0x6000) || (l_tile >= m_misc_state->m_max_level);

	if (m_other_modes->tex_lod_en)
	{
		if (distant)
		{
			l_tile = m_misc_state->m_max_level;
		}
		if (!m_other_modes->detail_tex_en)
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

void TexturePipe::CalculateClampDiffs(UINT32 prim_tile)
{
	Tile* tile = m_rdp->GetTiles();
	if (m_other_modes->cycle_type == CYCLE_TYPE_2)
	{
		if (m_other_modes->tex_lod_en)
		{
			int start = 0;
			int end = 7;
			for (; start <= end; start++)
			{
				m_clamp_s_diff[start] = (tile[start].sh >> 2) - (tile[start].sl >> 2);
				m_clamp_t_diff[start] = (tile[start].th >> 2) - (tile[start].tl >> 2);
			}
		}
		else
		{
			int start = prim_tile;
			int end = (prim_tile + 1) & 7;
			m_clamp_s_diff[start] = (tile[start].sh >> 2) - (tile[start].sl >> 2);
			m_clamp_t_diff[start] = (tile[start].th >> 2) - (tile[start].tl >> 2);
			m_clamp_s_diff[end] = (tile[end].sh >> 2) - (tile[end].sl >> 2);
			m_clamp_t_diff[end] = (tile[end].th >> 2) - (tile[end].tl >> 2);
		}
	}
	else//1-cycle or copy
	{
		m_clamp_s_diff[prim_tile] = (tile[prim_tile].sh >> 2) - (tile[prim_tile].sl >> 2);
		m_clamp_t_diff[prim_tile] = (tile[prim_tile].th >> 2) - (tile[prim_tile].tl >> 2);
	}
}

UINT32 TexturePipe::Fetch(INT32 s, INT32 t, INT32 tilenum)
{
	Tile* tile = m_rdp->GetTiles();
	Color color;
	UINT32 tbase = (tile[tilenum].line * t) & 0x1ff;
	UINT32 tformat = tile[tilenum].format;
	UINT32 tsize =	tile[tilenum].size;
	tbase += tile[tilenum].tmem;
	UINT32 tpal	= tile[tilenum].palette;

	if (tformat == FORMAT_I && tsize > PIXEL_SIZE_8BIT)
	{
		tformat = FORMAT_RGBA; // Used by Supercross 2000 (in-game)
	}
	if (tformat == FORMAT_CI && tsize > PIXEL_SIZE_8BIT)
	{
		tformat = FORMAT_RGBA; // Used by Clay Fighter - Sculptor's Cut
	}

	if (tformat == FORMAT_RGBA && tsize < PIXEL_SIZE_16BIT)
	{
		tformat = FORMAT_CI; // Used by Exterem-G2, Madden Football 64, and Rat Attack
	}

	UINT16 *tc16 = m_rdp->GetTMEM16();

	switch (tformat)
	{
		case FORMAT_RGBA:
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
					break;
				case PIXEL_SIZE_8BIT:
					break;
				case PIXEL_SIZE_16BIT:
				{
					int taddr = (tbase << 2) + s;
					taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					taddr &= 0x7ff;

					if (!m_other_modes->en_tlut)
					{
						UINT16 c = tc16[taddr];
						color.i.r = GET_HI_RGBA16_TMEM(c);
						color.i.g = GET_MED_RGBA16_TMEM(c);
						color.i.b = GET_LOW_RGBA16_TMEM(c);
						color.i.a = (c & 1) ? 0xff : 0;
					}
					else
					{
						UINT16 c = tc16[taddr];
						c = m_rdp->GetTLUT()[(c >> 8) << 2];
						if (m_other_modes->tlut_type == 0) // Used by Goldeneye 007 (ocean in Frigate)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else // Used by Beetle Adventure Racing (Mount Mayhem level)
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_32BIT:
				{
					UINT32 *tc = m_rdp->GetTMEM32();
					int taddr = (tbase << 2) + s;
					taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);

					if (!m_other_modes->en_tlut)
					{
						taddr &= 0x3ff;
						UINT32 c = tc16[taddr];
						color.i.r = (c >> 8) & 0xff;
						color.i.g = c & 0xff;
						c = tc16[taddr | 0x400];
						color.i.b = (c >>  8) & 0xff;
						color.i.a = c & 0xff;
					}
					else // Used by California Speed, attract mode
					{
						taddr &= 0x3ff;
						UINT32 c = tc[taddr];
						c = m_rdp->GetTLUT()[(c >> 24) << 2];
						if (m_other_modes->tlut_type == 0)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}

					break;
				}
				default:
					fatalerror("FETCH_TEXEL: unknown RGBA texture size %d\n", tsize);
					break;
			}
			break;
		}
		case FORMAT_YUV: // Used by: Bottom of the 9th, Pokemon Stadium, Ogre Battle, Major League Baseball, Ken Griffey Jr.'s Slugfest, Vigilante 8 Second Offense
		{
			switch(tsize)
			{
			case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = m_rdp->GetTMEM16();

					int taddr = (tbase << 3) + s;
					int taddrlow = taddr >> 1;

					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddrlow ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);

					taddr &= 0x7ff;
					taddrlow &= 0x3ff;

					UINT16 c = tc[taddrlow];

					INT32 y = m_rdp->GetTMEM()[taddr | 0x800];
					INT32 u = c >> 8;
					INT32 v = c & 0xff;

					v ^= 0x80; u ^= 0x80;
					if (v & 0x80)
					{
						v |= 0x100;
					}
					if (u & 0x80)
					{
						u |= 0x100;
					}

					color.i.r = u;
					color.i.g = v;
					color.i.b = y;
					color.i.a = y;
					break;
				}
			default:
				fatalerror("FETCH_TEXEL: unknown YUV texture size %d\n",tsize);
				break;
			}
			break;
		}
		case FORMAT_CI:
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT: // Madden Football 64, Bust-A-Move 2
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = ((tbase << 4) + s) >> 1;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;

					if (m_other_modes->en_tlut)
					{
						taddr &= 0x7ff;
						UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
						UINT16 c = m_rdp->GetTLUT()[((tpal << 4) | p) << 2];

						if (m_other_modes->tlut_type == 0)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}
					else
					{
						UINT8 p = (s & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
						p = (tpal << 4) | p;
						color.i.r = color.i.g = color.i.b = color.i.a = p;
					}

					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = (tbase << 3) + s;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;

					if (m_other_modes->en_tlut)
					{
						taddr &= 0x7ff;
						UINT8 p = tc[taddr];
						UINT16 c = m_rdp->GetTLUT()[p << 2];

						if (m_other_modes->tlut_type == 0)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}
					else
					{
						UINT8 p = tc[taddr];
						color.i.r = p;
						color.i.g = p;
						color.i.b = p;
						color.i.a = p;
					}
					break;
				}
				default:
					fatalerror("FETCH_TEXEL: unknown CI texture size %d\n", tsize);
					break;
			}
			break;
		}
		case FORMAT_IA:
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = ((tbase << 4) + s) >> 1;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;

					if (!m_other_modes->en_tlut)
					{
						UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
						UINT8 i = p & 0xe;
						i = (i << 4) | (i << 1) | (i >> 2);
						color.i.r = i;
						color.i.g = i;
						color.i.b = i;
						color.i.a = (p & 0x1) ? 0xff : 0;
					}
					else
					{
						taddr &= 0x7ff;
						UINT8 p = ((s) & 1) ? (tc[taddr] & 0xf) : (tc[taddr] >> 4);
						UINT16 c = m_rdp->GetTLUT()[((tpal << 4) | p) << 2];
						if (!m_other_modes->tlut_type)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = (tbase << 3) + s;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;
					if (!m_other_modes->en_tlut)
					{
						UINT8 p = tc[taddr];
						UINT8 i = p & 0xf0;
						i |= (i >> 4);
						color.i.r = i;
						color.i.g = i;
						color.i.b = i;
						color.i.a = ((p & 0xf) << 4) | (p & 0xf);
					}
					else
					{
						UINT8 p = tc[taddr & 0x7ff];
						UINT16 c = m_rdp->GetTLUT()[p << 2];
						if (!m_other_modes->tlut_type)
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (c >> 8) & 0xff;
							color.i.a = c & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_16BIT:
				{
					UINT16 *tc = m_rdp->GetTMEM16();
					int taddr = (tbase << 2) + s;
					taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					taddr &= 0x7ff;

					if (!m_other_modes->en_tlut)
					{
						UINT16 c = tc[taddr];
						UINT8 i = (c >> 8);
						color.i.r = i;
						color.i.g = i;
						color.i.b = i;
						color.i.a = c & 0xff;
					}
					else
					{
						UINT16 c = tc[taddr & 0x3ff];
						c = m_rdp->GetTLUT()[(c >> 8) << 2];
						if (m_other_modes->tlut_type == 1)
						{
							color.i.r = c >> 8;
							color.i.g = c >> 8;
							color.i.b = c >> 8;
							color.i.a = c & 0xff;
						}
						else
						{
							color.i.r = GET_HI_RGBA16_TMEM(c);
							color.i.g = GET_MED_RGBA16_TMEM(c);
							color.i.b = GET_LOW_RGBA16_TMEM(c);
							color.i.a = (c & 1) ? 0xff : 0;
						}
					}
					break;
				}
				default:
					color.i.r = color.i.g = color.i.b = color.i.a = 0xff;
					fatalerror("FETCH_TEXEL: unknown IA texture size %d\n", tsize);
					break;
			}
			break;
		}
		case FORMAT_I:
		{
			switch (tsize)
			{
				case PIXEL_SIZE_4BIT:
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = ((tbase << 4) + s) >> 1;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;

					if (!m_other_modes->en_tlut)
					{
						UINT8 byteval = tc[taddr];
						UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
						c |= (c << 4);
						color.i.r = c;
						color.i.g = c;
						color.i.b = c;
						color.i.a = c;
					}
					else
					{
						UINT8 byteval = tc[taddr & 0x7ff];
						UINT8 c = ((s & 1)) ? (byteval & 0xf) : ((byteval >> 4) & 0xf);
						UINT16 k = m_rdp->GetTLUT()[((tpal << 4) | c) << 2];
						if (!m_other_modes->tlut_type)
						{
							color.i.r = GET_HI_RGBA16_TMEM(k);
							color.i.g = GET_MED_RGBA16_TMEM(k);
							color.i.b = GET_LOW_RGBA16_TMEM(k);
							color.i.a = (k & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
							color.i.a = k & 0xff;
						}
					}
					break;
				}
				case PIXEL_SIZE_8BIT:
				{
					UINT8 *tc = m_rdp->GetTMEM();
					int taddr = (tbase << 3) + s;
					taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					taddr &= 0xfff;

					if (!m_other_modes->en_tlut)
					{
						UINT8 c = tc[taddr];
						color.i.r = c;
						color.i.g = c;
						color.i.b = c;
						color.i.a = c;
					}
					else
					{
						UINT8 c = tc[taddr & 0x7ff];
						UINT16 k = m_rdp->GetTLUT()[c << 2];
						if (!m_other_modes->tlut_type)
						{
							color.i.r = GET_HI_RGBA16_TMEM(k);
							color.i.g = GET_MED_RGBA16_TMEM(k);
							color.i.b = GET_LOW_RGBA16_TMEM(k);
							color.i.a = (k & 1) ? 0xff : 0;
						}
						else
						{
							color.i.r = color.i.g = color.i.b = (k >> 8) & 0xff;
							color.i.a = k & 0xff;
						}
					}

					break;
				}
				default:
					fatalerror("FETCH_TEXEL: unknown I texture size %d\n", tsize);
					break;
			}
			break;
		}
		default:
		{
			fatalerror("FETCH_TEXEL: unknown texture format %d, tilenum %d\n", tformat, tilenum);
			break;
		}
	}

	return color.c;
}

} // namespace RDP

} // namespace N64
