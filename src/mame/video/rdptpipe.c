#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define RELATIVE(x, y) 	((((x) >> 3) - (y)) << 3) | (x & 7);

void TexturePipe::SetMachine(running_machine *machine)
{
	_n64_state *state = (_n64_state *)machine->driver_data;

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();

	m_tex_fetch.SetMachine(m_machine);
}

void TexturePipe::CalculateClampDiffs(UINT32 prim_tile)
{
	int start;
	int end;
	if (m_other_modes->tex_lod_en || prim_tile == 7)
	{
		start = 0;
		end = 7;
	}
	else
	{
		start = prim_tile;
		end = prim_tile + 1;
	}
	for (; start <= end; start++)
	{
		m_clamp_s_diff[start] = (m_rdp->GetTiles()[start].sh >> 2) - (m_rdp->GetTiles()[start].sl >> 2);
		m_clamp_t_diff[start] = (m_rdp->GetTiles()[start].th >> 2) - (m_rdp->GetTiles()[start].tl >> 2);
	}
}

void TexturePipe::Mask(INT32* S, INT32* T, Tile* tile)
{
	INT32 swrap, twrap;

	if (tile->mask_s) // Select clamp if mask == 0
	{
		swrap = *S >> (tile->mask_s > 10 ? 10 : tile->mask_s);
		swrap &= 1;
		if (tile->ms && swrap)
		{
			*S = (~(*S)) & m_maskbits_table[tile->mask_s]; // Mirroring and masking
		}
		else if (tile->mask_s)
		{
			*S &= m_maskbits_table[tile->mask_s]; // Masking
		}
	}

	if (tile->mask_t)
	{
		twrap = *T >> (tile->mask_t > 10 ? 10 : tile->mask_t);
		twrap &= 1;
		if (tile->mt && twrap)
		{
			*T = (~(*T)) & m_maskbits_table[tile->mask_t]; // Mirroring and masking
		}
		else if (tile->mask_t)
		{
			*T &= m_maskbits_table[tile->mask_t];
		}
	}
}

void TexturePipe::TexShift(INT32* S, INT32* T, bool* maxs, bool* maxt, Tile *tile)
{
	*S = SIGN16(*S);
	*T = SIGN16(*T);

	if (tile->shift_s)
	{
		if (tile->shift_s < 11)
		{
			*S >>= tile->shift_s;
		}
		else
		{
			*S <<= (16 - tile->shift_s);
		}
		*S = SIGN16(*S);
	}

	if (tile->shift_s)
	{
		if (tile->shift_t < 11)
		{
			*T >>= tile->shift_t;
		}
    	else
    	{
			*T <<= (16 - tile->shift_t);
		}
		*T = SIGN16(*T);
	}

	*maxs = ((*S >> 3) >= tile->sh);
	*maxt = ((*T >> 3) >= tile->th);
}

void TexturePipe::Clamp(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, bool maxs, bool maxt, Tile* tile)
{
	bool notcopy = (m_other_modes->cycle_type != CYCLE_TYPE_COPY);
	bool dosfrac = (tile->cs || !tile->mask_s);
	bool dos = dosfrac && notcopy;
	bool dotfrac = (tile->ct || !tile->mask_t);
	bool dot = dotfrac && notcopy;
	bool overunders = false;
	bool overundert = false;
	if (*S & 0x10000 && dos)
	{
		*S = 0;
		overunders = true;
	}
	else if (maxs && dos)
	{
		*S = m_clamp_s_diff[tile->num];
		overunders = true;
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}
	if (overunders && dosfrac)
	{
		*SFRAC = 0;
	}

	if (*T & 0x10000 && dot)
	{
		*T = 0;
		overundert = true;
	}
	else if (maxt && dot)
	{
		*T = m_clamp_t_diff[tile->num];
		overundert = true;
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
	if (overundert && dotfrac)
	{
		*TFRAC = 0;
	}
}

void TexturePipe::ClampLight(INT32* S, INT32* T, bool maxs, bool maxt, Tile* tile)
{
	bool notcopy = (m_other_modes->cycle_type != CYCLE_TYPE_COPY);
	bool dos = (tile->cs || !tile->mask_s) && notcopy;
	bool dot = (tile->ct || !tile->mask_t) && notcopy;

	if (*S & 0x10000 && dos)
	{
		*S = 0;
	}
	else if (maxs && dos)
	{
		*S = m_clamp_s_diff[tile->num];
	}
	else
	{
		*S = (SIGN17(*S) >> 5) & 0x1fff;
	}

	if (*T & 0x10000 && dot)
	{
		*T = 0;
	}
	else if (maxt && dot)
	{
		*T = m_clamp_t_diff[tile->num];
	}
	else
	{
		*T = (SIGN17(*T) >> 5) & 0x1fff;
	}
}

UINT32 TexturePipe::Fetch(INT32 SSS, INT32 SST, Tile* tile)
{
	Color TEX;
	if (m_other_modes->sample_type)
	{
		INT32 rt[4] = { 0 };
		INT32 gt[4] = { 0 };
		INT32 bt[4] = { 0 };
		INT32 at[4] = { 0 };
		Color t0;
		Color t1;
		Color t2;
		Color t3;

		int sss1 = SSS;
		int sst1 = SST;
		bool maxs = false;
		bool maxt = false;
		TexShift(&sss1, &sst1, &maxs, &maxt, tile);

		int sss2 = sss1 + 32;
		int sst2 = sst1 + 32;
		bool maxs2 = ((sss2 >> 3) >= tile->sh);
		bool maxt2 = ((sst2 >> 3) >= tile->th);

		sss1 = RELATIVE(sss1, tile->sl);
		sst1 = RELATIVE(sst1, tile->tl);
		sss2 = RELATIVE(sss2, tile->sl);
		sst2 = RELATIVE(sst2, tile->tl);

		INT32 SFRAC = sss1 & 0x1f;
		INT32 TFRAC = sst1 & 0x1f;

		Clamp(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tile);
		ClampLight(&sss2, &sst2, maxs2, maxt2, tile);

		Mask(&sss1, &sst1, tile);
        Mask(&sss2, &sst2, tile);

		bool upper = ((SFRAC + TFRAC) >= 0x20);
		INT32 INVSF = 0;
		INT32 INVTF = 0;
		if (upper)
		{
			INVSF = 0x20 - SFRAC;
			INVTF = 0x20 - TFRAC;
		}

		t1.c = m_tex_fetch.Fetch(sss2, sst1, tile);
		t2.c = m_tex_fetch.Fetch(sss1, sst2, tile);
		if(m_other_modes->mid_texel || !upper)
		{
			t0.c = m_tex_fetch.Fetch(sss1, sst1, tile);
			rt[0] = t0.i.r; gt[0] = t0.i.g; bt[0] = t0.i.b; at[0] = t0.i.a;
		}
		rt[1] = t1.i.r; gt[1] = t1.i.g; bt[1] = t1.i.b; at[1] = t1.i.a;
		rt[2] = t2.i.r; gt[2] = t2.i.g; bt[2] = t2.i.b; at[2] = t2.i.a;
		if (m_other_modes->mid_texel || upper)
		{
			t3.c = m_tex_fetch.Fetch(sss2, sst2, tile);
			rt[3] = t3.i.r; gt[3] = t3.i.g; bt[3] = t3.i.b; at[3] = t3.i.a;
		}

		if (!m_other_modes->mid_texel || SFRAC!= 0x10 || TFRAC != 0x10)
		{
			if (upper)
			{
				INT32 R32 = rt[3] + ((INVSF*(rt[2] - rt[3]))>>5) + ((INVTF*(rt[1] - rt[3]))>>5);
				INT32 G32 = gt[3] + ((INVSF*(gt[2] - gt[3]))>>5) + ((INVTF*(gt[1] - gt[3]))>>5);
				INT32 B32 = bt[3] + ((INVSF*(bt[2] - bt[3]))>>5) + ((INVTF*(bt[1] - bt[3]))>>5);
				INT32 A32 = at[3] + ((INVSF*(at[2] - at[3]))>>5) + ((INVTF*(at[1] - at[3]))>>5);
				TEX.i.r = (R32 < 0) ? 0 : R32;
				TEX.i.g = (G32 < 0) ? 0 : G32;
				TEX.i.b = (B32 < 0) ? 0 : B32;
				TEX.i.a = (A32 < 0) ? 0 : A32;
			}
			else
			{
				INT32 R32 = rt[0] + ((SFRAC*(rt[1] - rt[0]))>>5) + ((TFRAC*(rt[2] - rt[0]))>>5);
				INT32 G32 = gt[0] + ((SFRAC*(gt[1] - gt[0]))>>5) + ((TFRAC*(gt[2] - gt[0]))>>5);
				INT32 B32 = bt[0] + ((SFRAC*(bt[1] - bt[0]))>>5) + ((TFRAC*(bt[2] - bt[0]))>>5);
				INT32 A32 = at[0] + ((SFRAC*(at[1] - at[0]))>>5) + ((TFRAC*(at[2] - at[0]))>>5);
				TEX.i.r = (R32 < 0) ? 0 : R32;
				TEX.i.g = (G32 < 0) ? 0 : G32;
				TEX.i.b = (B32 < 0) ? 0 : B32;
				TEX.i.a = (A32 < 0) ? 0 : A32;
			}
		}
		else // Is this accurate?
		{
			TEX.i.r = (rt[0] + rt[1] + rt[2] + rt[3]) >> 2;
			TEX.i.g = (gt[0] + gt[1] + gt[2] + gt[3]) >> 2;
			TEX.i.b = (bt[0] + bt[1] + bt[2] + bt[3]) >> 2;
			TEX.i.a = (at[0] + at[1] + at[2] + at[3]) >> 2;
		}
	}
	else
	{
		int sss1 = SSS;
		int sst1 = SST;

		bool maxs = false;
		bool maxt = false;
		TexShift(&sss1, &sst1, &maxs, &maxt, tile);

		sss1 = RELATIVE(sss1, tile->sl);
		sst1 = RELATIVE(sst1, tile->tl);

		sss1 += 0x10;
		sst1 += 0x10;

		INT32 SFRAC = sss1 & 0x1f;
		INT32 TFRAC = sst1 & 0x1f;

		Clamp(&sss1, &sst1, &SFRAC, &TFRAC, maxs, maxt, tile);

        Mask(&sss1, &sst1, tile);

		/* point sample */
		TEX.c = m_tex_fetch.Fetch(sss1, sst1, tile);
	}

	return TEX.c;
}

} // namespace RDP

} // namespace N64
