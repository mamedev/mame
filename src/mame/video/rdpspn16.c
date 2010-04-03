#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define LookUpCC(A, B, C, D) m_rdp->GetCCLUT2()[(m_rdp->GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

void Processor::RenderSpans(int start, int end, int tilenum, bool shade, bool texture, bool zbuffer, bool flip)
{
	m_tex_pipe.CalculateClampDiffs(tilenum);

	int clipy1 = GetScissor()->m_yh;
	int clipy2 = GetScissor()->m_yl;

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

	for(int i = start; i <= end; i++)
	{
		m_span[i].SetMachine(m_machine);
		m_span[i].Draw(i, tilenum, shade, texture, zbuffer, flip);
	}
}

void Span::Dump()
{
	printf("    m_lx = %d\n", m_lx);
	printf("    m_rx = %d\n",  m_rx);
	printf("    m_s.w = %08x\n", m_s.w);
	printf("    m_t.w = %08x\n", m_t.w);
	printf("    m_w.w = %08x\n", m_w.w);
	printf("    m_r.w = %08x\n", m_r.w);
	printf("    m_g.w = %08x\n", m_g.w);
	printf("    m_b.w = %08x\n", m_b.w);
	printf("    m_a.w = %08x\n", m_a.w);
	printf("    m_z.w = %08x\n", m_z.w);

	printf("    CVG: ");
	for(int index = 0; index < RDP_CVG_SPAN_MAX; index++)
	{
		printf("%d", m_cvg[index]);
	}
	printf("\n");

	printf("    m_dymax = %08x\n", m_dymax);
	printf("    m_ds.w = %08x\n", m_ds.w);
	printf("    m_dt.w = %08x\n", m_dt.w);
	printf("    m_dw.w = %08x\n", m_dw.w);
	printf("    m_dr.w = %08x\n", m_dr.w);
	printf("    m_dg.w = %08x\n", m_dg.w);
	printf("    m_db.w = %08x\n", m_db.w);
	printf("    m_da.w = %08x\n", m_da.w);
	printf("    m_dz.w = %08x\n", m_dz.w);
	printf("    m_dzpix = %08x\n", m_dzpix);
}

void Span::SetMachine(running_machine *machine)
{
	_n64_state *state = (_n64_state *)machine->driver_data;

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();
	m_misc_state = m_rdp->GetMiscState();
}

void Span::Draw(int index, int tilenum, bool shade, bool texture, bool zbuffer, bool flip)
{
	int clipx1 = m_rdp->GetScissor()->m_xh;
	int clipx2 = m_rdp->GetScissor()->m_xl;

	SpanParam r = m_r;
	SpanParam g = m_g;
	SpanParam b = m_b;
	SpanParam a = m_a;
	SpanParam z = m_z;
	SpanParam s = m_s;
	SpanParam t = m_t;
	SpanParam w = m_w;

	UINT16 *fb = (UINT16*)&rdram[m_misc_state->m_fb_address / 4];
	UINT16 *zb = (UINT16*)&rdram[m_misc_state->m_zb_address / 4];
	UINT8 *hb = &m_rdp->GetHiddenBits()[m_misc_state->m_fb_address >> 1];
	UINT8 *zhb = &m_rdp->GetHiddenBits()[m_misc_state->m_zb_address >> 1];

	UINT32 prim_tile = tilenum;
	UINT32 tilenum2 = 0;

	int dzpix = m_dzpix;
	int drinc = flip ? (m_dr.w) : -m_dr.w;
	int dginc = flip ? (m_dg.w) : -m_dg.w;
	int dbinc = flip ? (m_db.w) : -m_db.w;
	int dainc = flip ? (m_da.w) : -m_da.w;
	int dzinc = flip ? (m_dz.w) : -m_dz.w;
	int dsinc = flip ? (m_ds.w) : -m_ds.w;
	int dtinc = flip ? (m_dt.w) : -m_dt.w;
	int dwinc = flip ? (m_dw.w) : -m_dw.w;
	int xinc = flip ? 1 : -1;

	int nexts;
	int nextt;
	int nextsw;

	int fb_index = m_misc_state->m_fb_width * index;

	int x = m_rx;

	int length = flip ? (m_lx - m_rx) : (m_rx - m_lx);

	bool disable_lod = false;
	if (m_other_modes->cycle_type != CYCLE_TYPE_2) // Used by World Driver Championship
	{
		disable_lod = true;
	}

	if (m_other_modes->cycle_type == CYCLE_TYPE_2 && texture && !m_other_modes->tex_lod_en)
	{
		tilenum2 = (prim_tile + 1) & 7;
	}

	if (texture && !m_other_modes->tex_lod_en)
	{
		tilenum = prim_tile;
	}

	if(!shade)
	{
		m_rdp->GetShadeColor()->c = m_rdp->GetPrimColor()->c;
	}

	for (int j = 0; j <= length; j++)
	{
		int sr = 0;
		int sg = 0;
		int sb = 0;
		int sa = 0;
		int ss = 0;
		int st = 0;
		int sw = 0;
		int sz = 0;
		int sss = 0;
		int sst = 0;
		Color c1;
		Color c2;

		if(shade)
		{
			sr = r.h.h;
			sg = g.h.h;
			sb = b.h.h;
			sa = a.h.h;
		}

		if(texture)
		{
			ss = s.h.h;
			st = t.h.h;
			sw = w.h.h;
		}

		if(zbuffer)
		{
			sz = z.w >> 13;

			if (m_other_modes->z_source_sel)
			{
				sz = (((UINT32)m_misc_state->m_primitive_z) << 3) & 0x3ffff;
				dzpix = m_misc_state->m_primitive_delta_z;
			}
		}

		if (x >= clipx1 && x < clipx2)
		{
			bool z_compare_result = true;

			m_misc_state->m_curpixel_cvg = m_cvg[x];

			if (m_misc_state->m_curpixel_cvg)
			{
				int curpixel = fb_index + x;
				UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
				UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
				UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
				UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

				if(texture)
				{
					if (m_other_modes->persp_tex_en)
					{
						m_rdp->TCDiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}

					if (m_other_modes->tex_lod_en && !disable_lod)
					{
						if (m_other_modes->persp_tex_en)
						{
							nextsw = (w.w + dwinc) >> 16;
							nexts = (s.w + dsinc) >> 16;
							nextt = (t.w + dtinc) >> 16;
							m_rdp->TCDiv(nexts, nextt, nextsw, &nexts, &nextt);
						}
						else
						{
							nexts = (s.w + dsinc)>>16;
							nextt = (t.w + dtinc)>>16;
						}

						INT32 horstep = SIGN17(nexts & 0x1ffff) - SIGN17(sss & 0x1ffff);
						INT32 vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(sst & 0x1ffff);
						if (horstep & 0x20000)
						{
							horstep = ~horstep & 0x1ffff;
						}
						if (vertstep & 0x20000)
						{
							vertstep = ~vertstep & 0x1ffff;
						}

						int LOD = (horstep >= vertstep) ? horstep : vertstep;
						LOD = (LOD >= m_dymax) ? LOD : m_dymax;

						if (LOD & 0x1c000)
						{
							LOD = 0x7fff;
						}
						if (LOD < m_misc_state->m_min_level)
						{
							LOD = m_misc_state->m_min_level;
						}

						bool magnify = (LOD < 32);
						INT32 l_tile = m_rdp->GetLog2((LOD >> 5) & 0xff);
						bool distant = ((LOD & 0x6000) || (l_tile >= m_misc_state->m_max_level));

						m_rdp->SetLODFrac(((LOD << 3) >> l_tile) & 0xff);

						if (distant)
						{
							l_tile = m_misc_state->m_max_level;
						}
						if(!m_other_modes->sharpen_tex_en && !m_other_modes->detail_tex_en && magnify)
						{
							m_rdp->SetLODFrac(0);
						}
						if(!m_other_modes->sharpen_tex_en && !m_other_modes->detail_tex_en && distant)
						{
							m_rdp->SetLODFrac(0xff);
						}
						if(m_other_modes->sharpen_tex_en && magnify)
						{
							m_rdp->SetLODFrac(*(m_rdp->GetLODFrac()) | 0x100);
						}

						if (!m_other_modes->detail_tex_en)
						{
							tilenum = (prim_tile + l_tile);
							tilenum &= 7;
							if (m_other_modes->sharpen_tex_en)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else if (!distant)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else
							{
								tilenum2 = tilenum;
							}
						}
						else
						{
							if (!magnify)
							{
								tilenum = (prim_tile + l_tile + 1);
							}
							else
							{
								tilenum = (prim_tile + l_tile);
							}
							tilenum &= 7;

							if (!distant && !magnify)
							{
								tilenum2 = (prim_tile + l_tile + 2) & 7;
							}
							else
							{
								tilenum2 = (prim_tile + l_tile + 1) & 7;
							}
						}
					}

					if (m_other_modes->cycle_type == CYCLE_TYPE_1)
					{
						m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(sss, sst, &m_rdp->GetTiles()[tilenum]);
					}
					else
					{
						m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(sss, sst, &m_rdp->GetTiles()[tilenum]);
						m_rdp->GetTexel1Color()->c = m_rdp->GetTexPipe()->Fetch(sss, sst, &m_rdp->GetTiles()[tilenum2]);
					}
				}

				if (shade)
				{
					if (sr > 0xff) sr = 0xff;
					if (sg > 0xff) sg = 0xff;
					if (sb > 0xff) sb = 0xff;
					if (sa > 0xff) sa = 0xff;
					if (sr < 0) sr = 0;
					if (sg < 0) sg = 0;
					if (sb < 0) sb = 0;
					if (sa < 0) sa = 0;
					m_rdp->GetShadeColor()->i.r = sr;
					m_rdp->GetShadeColor()->i.g = sg;
					m_rdp->GetShadeColor()->i.b = sb;
					m_rdp->GetShadeColor()->i.a = sa;
				}

				if (m_other_modes->cycle_type == CYCLE_TYPE_1)
				{
					c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
					c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
					c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
					c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
									  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
									  *m_rdp->GetColorInputs()->combiner_alphamul[1],
									  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
					m_rdp->GetAlphaCvg(&c1.i.a);
				}
				else if (m_other_modes->cycle_type == CYCLE_TYPE_2)
				{
					c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[0],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[0],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_r[0],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_r[0]);
					c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[0],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[0],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_g[0],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_g[0]);
					c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[0],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[0],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_b[0],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_b[0]);
					c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[0],
									  *m_rdp->GetColorInputs()->combiner_alphasub_b[0],
									  *m_rdp->GetColorInputs()->combiner_alphamul[0],
									  *m_rdp->GetColorInputs()->combiner_alphaadd[0]);
					m_rdp->GetCombinedColor()->c = c1.c;
					c2.c = m_rdp->GetTexel0Color()->c;
					m_rdp->GetTexel0Color()->c = m_rdp->GetTexel1Color()->c;
					m_rdp->GetTexel1Color()->c = c2.c;
					c2.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
					c2.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
					c2.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
									  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
					c2.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
									  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
									  *m_rdp->GetColorInputs()->combiner_alphamul[1],
									  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
					m_rdp->GetAlphaCvg(&c2.i.a);
				}

				if ((zbuffer || m_other_modes->z_source_sel) && m_other_modes->z_compare_en)
				{
					z_compare_result = m_rdp->ZCompare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
				}

				if(z_compare_result)
				{
					bool rendered = false;
					int dith = 0;
					if (!m_other_modes->rgb_dither_sel)
					{
						dith = m_rdp->GetMagicMatrix()[(((index) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
					}
					else if (m_other_modes->rgb_dither_sel == 1)
					{
						dith = m_rdp->GetBayerMatrix()[(((index) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
					}

					rendered = m_rdp->GetBlender()->Blend(fbcur, hbcur, c1, c2, dith);

					if (m_other_modes->z_update_en && rendered)
					{
						m_rdp->ZStore(zbcur, zhbcur, sz, dzpix);
					}
				}
			}
		}

		if (shade)
		{
			r.w += drinc;
			g.w += dginc;
			b.w += dbinc;
			a.w += dainc;
		}
		if (texture)
		{
			s.w += dsinc;
			t.w += dtinc;
			w.w += dwinc;
		}
		if (zbuffer)
		{
			z.w += dzinc;
		}

		x += xinc;
	}
}

}

}