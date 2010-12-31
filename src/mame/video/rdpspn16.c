#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define LookUpCC(A, B, C, D) m_rdp->GetCCLUT2()[(m_rdp->GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

void Processor::RenderSpans(int start, int end, int tilenum, bool flip)
{
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
		switch(m_other_modes.cycle_type)
		{
			case CYCLE_TYPE_1: m_span[i].Draw1Cycle(i, tilenum, flip); break;
			case CYCLE_TYPE_2: m_span[i].Draw2Cycle(i, tilenum, flip); break;
			case CYCLE_TYPE_COPY: m_span[i].DrawCopy(i, tilenum, flip); break;
			case CYCLE_TYPE_FILL: m_span[i].DrawFill(i, tilenum, flip); break;
		}

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
}

void Span::SetMachine(running_machine *machine)
{
	_n64_state *state = machine->driver_data<_n64_state>();

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();
	m_misc_state = m_rdp->GetMiscState();
}

void Span::RGBAZClip(int sr, int sg, int sb, int sa, int *sz)
{
	m_rdp->GetShadeColor()->i.r = m_rdp->GetSpecial9BitClampTable()[sr & 0x1ff];
	m_rdp->GetShadeColor()->i.g = m_rdp->GetSpecial9BitClampTable()[sg & 0x1ff];
	m_rdp->GetShadeColor()->i.b = m_rdp->GetSpecial9BitClampTable()[sb & 0x1ff];
	m_rdp->GetShadeColor()->i.a = m_rdp->GetSpecial9BitClampTable()[sa & 0x1ff];

	INT32 zanded = (*sz) & 0x60000;

	zanded >>= 17;
	switch(zanded)
	{
		case 0: *sz &= 0x3ffff;											break;
		case 1:	*sz &= 0x3ffff;											break;
		case 2: *sz = 0x3ffff;											break;
		case 3: *sz = 0x3ffff;											break;
	}
}

void Span::RGBAZCorrectTriangle(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z)
{
	if (m_rdp->GetMiscState()->m_curpixel_cvg == 8)
	{
		*r >>= 2;
		*g >>= 2;
		*b >>= 2;
		*a >>= 2;
		*z = (*z >> 3) & 0x7ffff;
	}
	else
	{
		INT32 summand_xr = offx * SIGN13(m_rdp->m_span_dr >> 14);
		INT32 summand_yr = offy * SIGN13(m_rdp->m_span_drdy >> 14);
		INT32 summand_xb = offx * SIGN13(m_rdp->m_span_db >> 14);
		INT32 summand_yb = offy * SIGN13(m_rdp->m_span_dbdy >> 14);
		INT32 summand_xg = offx * SIGN13(m_rdp->m_span_dg >> 14);
		INT32 summand_yg = offy * SIGN13(m_rdp->m_span_dgdy >> 14);
		INT32 summand_xa = offx * SIGN13(m_rdp->m_span_da >> 14);
		INT32 summand_ya = offy * SIGN13(m_rdp->m_span_dady >> 14);

		INT32 summand_xz = offx * SIGN22(m_rdp->m_span_dz >> 10);
		INT32 summand_yz = offy * SIGN22(m_rdp->m_span_dzdy >> 10);

		*r = ((*r << 2) + summand_xr + summand_yr) >> 4;
		*g = ((*g << 2) + summand_xg + summand_yg) >> 4;
		*b = ((*b << 2) + summand_xb + summand_yb) >> 4;
		*a = ((*a << 2) + summand_xa + summand_ya) >> 4;
		*z = (((*z << 2) + summand_xz + summand_yz) >> 5) & 0x7ffff;
	}
}

void Span::Draw1Cycle(int index, int tilenum, bool flip)
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

	UINT32 zb = m_misc_state->m_zb_address >> 1;
	UINT32 zhb = zb;
	UINT8 offx = 0, offy = 0;

	INT32 tile1 = tilenum;

	m_rdp->GetTexPipe()->CalculateClampDiffs(tile1);

	bool noisecompute = m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1] == &m_rdp->GetNoiseColor()->i.r;
	bool partialreject = (m_rdp->GetColorInputs()->blender2b_a[0] == &m_rdp->GetInvPixelColor()->i.a && m_rdp->GetColorInputs()->blender1b_a[0] == &m_rdp->GetPixelColor()->i.a);
	bool bsel0 = (m_rdp->GetColorInputs()->blender2b_a[0] == &m_rdp->GetMemoryColor()->i.a);

	int drinc = flip ? (m_rdp->m_span_dr) : -m_rdp->m_span_dr;
	int dginc = flip ? (m_rdp->m_span_dg) : -m_rdp->m_span_dg;
	int dbinc = flip ? (m_rdp->m_span_db) : -m_rdp->m_span_db;
	int dainc = flip ? (m_rdp->m_span_da) : -m_rdp->m_span_da;
	int dzinc = flip ? (m_rdp->m_span_dz) : -m_rdp->m_span_dz;
	int dsinc = flip ? (m_rdp->m_span_ds) : -m_rdp->m_span_ds;
	int dtinc = flip ? (m_rdp->m_span_dt) : -m_rdp->m_span_dt;
	int dwinc = flip ? (m_rdp->m_span_dw) : -m_rdp->m_span_dw;
	int dzpix = m_rdp->m_span_dzpix;
	int xinc = flip ? 1 : -1;

	int fb_index = m_misc_state->m_fb_width * index;

	int cdith = 0;
	int adith = 0;

	int xstart = m_lx;
	int xend = m_unscissored_rx;
	int xend_scissored = m_rx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);
	m_rdp->GetTexPipe()->m_start_span = true;
	UINT32 fir, fig, fib;

	for (int j = 0; j <= length; j++)
	{
		int sr = r.w >> 14;
		int sg = g.w >> 14;
		int sb = b.w >> 14;
		int sa = a.w >> 14;
		int ss = s.w >> 16;
		int st = t.w >> 16;
		int sw = w.w >> 16;
		int sz = (z.w >> 10) & 0x3fffff;
		INT32 sss = 0;
		INT32 sst = 0;

		if (m_other_modes->z_source_sel)
		{
			sz = (((UINT32)m_misc_state->m_primitive_z) << 6) & 0x3fffff;
			dzpix = m_misc_state->m_primitive_delta_z;
			dzinc = m_rdp->m_span_dz = m_rdp->m_span_dzdy = 0;
		}

		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			m_rdp->lookup_cvmask_derivatives(m_cvg[x], &offx, &offy);

			if (m_rdp->GetTexPipe()->m_start_span)
			{
				if (m_other_modes->persp_tex_en)
				{
					m_rdp->TCDiv(ss, st, sw, &sss, &sst);
				}
				else
				{
					m_rdp->TCDivNoPersp(ss, st, sw, &sss, &sst);
				}
			}
			else
			{
				sss = m_rdp->GetTexPipe()->m_precomp_s;
				sst = m_rdp->GetTexPipe()->m_precomp_t;
			}

			m_rdp->GetTexPipe()->LOD1Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz);
			RGBAZClip(sr, sg, sb, sa, &sz);

			m_rdp->GetTexPipe()->Cycle(m_rdp->GetTexel0Color(), m_rdp->GetTexel0Color(), sss, sst, tilenum, 0);

			m_rdp->ColorCombiner1Cycle(noisecompute);

			UINT32 curpixel = fb_index + x;
			UINT32 zbcur = zb + curpixel;
			UINT32 zhbcur = zhb + curpixel;

			m_rdp->GetFramebuffer()->Read(curpixel);

			if(m_rdp->ZCompare(zbcur, zhbcur, sz, dzpix))
			{
				m_rdp->GetDitherValues(index, j, &cdith, &adith);

				bool rendered = m_rdp->GetBlender()->Blend1Cycle(&fir, &fig, &fib, cdith, adith, partialreject, bsel0);

                if (rendered)
				{
					m_rdp->GetFramebuffer()->Write(curpixel, fir, fig, fib);
					if (m_other_modes->z_update_en)
					{
						m_rdp->ZStore(zbcur, zhbcur, sz);
					}
				}
			}
		}

		r.w += drinc;
		g.w += dginc;
		b.w += dbinc;
		a.w += dainc;
		s.w += dsinc;
		t.w += dtinc;
		w.w += dwinc;
		z.w += dzinc;

		x += xinc;
	}
}

void Span::Draw2Cycle(int index, int tilenum, bool flip)
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

	UINT32 zb = m_misc_state->m_zb_address >> 1;
	UINT32 zhb = zb;
	UINT8 offx = 0, offy = 0;

	INT32 tile2 = (tilenum + 1) & 7;
	INT32 tile1 = tilenum;
	UINT32 prim_tile = tilenum;

	int newtile1 = tile1;
	INT32 news = 0;
	INT32 newt = 0;

	m_rdp->GetTexPipe()->CalculateClampDiffs(tile1);

	bool noisecompute = (m_rdp->GetColorInputs()->combiner_rgbsub_a_r[0] == &m_rdp->GetNoiseColor()->i.r || m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1] == &m_rdp->GetPixelColor()->i.r);
	bool partialreject = (m_rdp->GetColorInputs()->blender2b_a[1] == &m_rdp->GetInvPixelColor()->i.a && m_rdp->GetColorInputs()->blender1b_a[1] == &m_rdp->GetPixelColor()->i.a);
	bool bsel0 = (m_rdp->GetColorInputs()->blender2b_a[0] == &m_rdp->GetMemoryColor()->i.a);
	bool bsel1 = (m_rdp->GetColorInputs()->blender2b_a[1] == &m_rdp->GetMemoryColor()->i.a);

	int dzpix = m_rdp->m_span_dzpix;
	int drinc = flip ? (m_rdp->m_span_dr) : -m_rdp->m_span_dr;
	int dginc = flip ? (m_rdp->m_span_dg) : -m_rdp->m_span_dg;
	int dbinc = flip ? (m_rdp->m_span_db) : -m_rdp->m_span_db;
	int dainc = flip ? (m_rdp->m_span_da) : -m_rdp->m_span_da;
	int dzinc = flip ? (m_rdp->m_span_dz) : -m_rdp->m_span_dz;
	int dsinc = flip ? (m_rdp->m_span_ds) : -m_rdp->m_span_ds;
	int dtinc = flip ? (m_rdp->m_span_dt) : -m_rdp->m_span_dt;
	int dwinc = flip ? (m_rdp->m_span_dw) : -m_rdp->m_span_dw;
	int xinc = flip ? 1 : -1;

	int fb_index = m_misc_state->m_fb_width * index;

	int cdith = 0;
	int adith = 0;

	int xstart = m_lx;
	int xend = m_unscissored_rx;
	int xend_scissored = m_rx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);
	m_rdp->GetTexPipe()->m_start_span = true;
	UINT32 fir, fig, fib;

	//printf( "Span length: %d\n", length);

	for (int j = 0; j <= length; j++)
	{
		int sr = r.w >> 14;
		int sg = g.w >> 14;
		int sb = b.w >> 14;
		int sa = a.w >> 14;
		int ss = s.h.h;
		int st = t.h.h;
		int sw = w.h.h;
		int sz = (z.w >> 10) & 0x3fffff;
		INT32 sss = 0;
		INT32 sst = 0;
		Color c1;
		Color c2;

		if (m_other_modes->z_source_sel)
		{
			sz = (((UINT32)m_misc_state->m_primitive_z) << 6) & 0x3fffff;
			dzpix = m_misc_state->m_primitive_delta_z;
			dzinc = m_rdp->m_span_dz = m_rdp->m_span_dzdy = 0;
		}

		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			m_rdp->lookup_cvmask_derivatives(m_cvg[x], &offx, &offy);

			if (m_rdp->GetTexPipe()->m_start_span)
			{
				if (m_other_modes->persp_tex_en)
				{
					m_rdp->TCDiv(ss, st, sw, &sss, &sst);
				}
				else
				{
					m_rdp->TCDivNoPersp(ss, st, sw, &sss, &sst);
				}
			}
			else
			{
				sss = m_rdp->GetTexPipe()->m_precomp_s;
				sst = m_rdp->GetTexPipe()->m_precomp_t;
			}

			m_rdp->GetTexPipe()->LOD2Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2);

			news = m_rdp->GetTexPipe()->m_precomp_s;
			newt = m_rdp->GetTexPipe()->m_precomp_t;
			m_rdp->GetTexPipe()->LOD2CycleLimited(&news, &newt, s.w + dsinc, t.w + dtinc, w.w + dwinc, dsinc, dtinc, dwinc, prim_tile, &newtile1);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz);
			RGBAZClip(sr, sg, sb, sa, &sz);

			m_rdp->GetTexPipe()->Cycle(m_rdp->GetTexel0Color(), m_rdp->GetTexel0Color(), sss, sst, tile1, 0);
			m_rdp->GetTexPipe()->Cycle(m_rdp->GetTexel1Color(), m_rdp->GetTexel0Color(), sss, sst, tile2, 1);

			m_rdp->GetTexPipe()->Cycle(m_rdp->GetNextTexelColor(), m_rdp->GetNextTexelColor(), sss, sst, tile2, 1);

			m_rdp->ColorCombiner2Cycle(noisecompute);

			UINT32 curpixel = fb_index + x;
			UINT32 zbcur = zb + curpixel;
			UINT32 zhbcur = zhb + curpixel;

			m_rdp->GetFramebuffer()->Read(curpixel);

			if(m_rdp->ZCompare(zbcur, zhbcur, sz, dzpix))
			{
				m_rdp->GetDitherValues(index, j, &cdith, &adith);

				bool rendered = m_rdp->GetBlender()->Blend2Cycle(&fir, &fig, &fib, cdith, adith, partialreject, bsel0, bsel1);

                if (rendered)
				{
					m_rdp->GetFramebuffer()->Write(curpixel, fir, fig, fib);
					if (m_other_modes->z_update_en)
					{
						m_rdp->ZStore(zbcur, zhbcur, sz);
					}
				}
			}
		}

		r.w += drinc;
		g.w += dginc;
		b.w += dbinc;
		a.w += dainc;
		s.w += dsinc;
		t.w += dtinc;
		w.w += dwinc;
		z.w += dzinc;

		x += xinc;
	}
}

void Span::DrawCopy(int index, int tilenum, bool flip)
{
	int clipx1 = m_rdp->GetScissor()->m_xh;
	int clipx2 = m_rdp->GetScissor()->m_xl;

	SpanParam s = m_s;
	SpanParam t = m_t;

	int ds = m_rdp->m_span_ds / 4;
	int dt = m_rdp->m_span_dt / 4;
	int dsinc = flip ? (ds) : -ds;
	int dtinc = flip ? (dt) : -dt;
	int xinc = flip ? 1 : -1;

	int fb_index = m_misc_state->m_fb_width * index;

	int xstart = m_lx;
	int xend = m_unscissored_rx;
	int xend_scissored = m_rx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);

	for (int j = 0; j <= length; j++)
	{
		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			INT32 sss = s.h.h;
			INT32 sst = t.h.h;
			m_rdp->GetTexPipe()->Copy(m_rdp->GetTexel0Color(), sss, sst, tilenum);

			UINT32 curpixel = fb_index + x;
			m_misc_state->m_curpixel_cvg = m_rdp->GetTexel0Color()->i.a ? 7 : 0;
			if ((m_rdp->GetTexel0Color()->i.a != 0) || (!m_other_modes->alpha_compare_en))
			{
				m_rdp->GetFramebuffer()->Copy(curpixel, m_rdp->GetTexel0Color()->i.r, m_rdp->GetTexel0Color()->i.g, m_rdp->GetTexel0Color()->i.b);
			}
		}

		s.w += dsinc;
		t.w += dtinc;
		x += xinc;
	}
}

void Span::DrawFill(int index, int tilenum, bool flip)
{
	int clipx1 = m_rdp->GetScissor()->m_xh;
	int clipx2 = m_rdp->GetScissor()->m_xl;

	int xinc = flip ? 1 : -1;

	int fb_index = m_misc_state->m_fb_width * index;

	int xstart = m_lx;
	int xend_scissored = m_rx;

	int x = xend_scissored;

	int length = flip ? (xstart - xend_scissored) : (xend_scissored - xstart);

	for (int j = 0; j <= length; j++)
	{
		if (x >= clipx1 && x < clipx2)
		{
			UINT32 curpixel = fb_index + x;
			m_rdp->GetFramebuffer()->Fill(curpixel);
		}

		x += xinc;
	}
}

}

}
