#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

#define LookUpCC(A, B, C, D) m_rdp->GetCCLUT2()[(m_rdp->GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

void n64_rdp::RenderSpans(int start, int end, int tilenum, bool flip, extent_t *Spans, bool rect, rdp_poly_state *object)
{
	int clipy1 = Scissor.m_yh;
	int clipy2 = Scissor.m_yl;
	int offset = 0;

	if (clipy2 <= 0)
	{
		return;
	}

	if (start < clipy1)
	{
		offset = clipy1 - start;
		start = clipy1;
	}
	if (start >= clipy2)
	{
		offset = start - (clipy2 - 1);
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2)
	{
		end = clipy2 - 1;
	}

	object->m_rdp = this;
	memcpy(&object->MiscState, &MiscState, sizeof(MiscStateT));
	memcpy(&object->OtherModes, &OtherModes, sizeof(OtherModesT));
	memcpy(&object->SpanBase, &SpanBase, sizeof(SpanBaseT));
	memcpy(&object->Scissor, &Scissor, sizeof(Rectangle));
	memcpy(&object->m_tiles, &m_tiles, 8 * sizeof(N64Tile));
	object->tilenum = tilenum;
	object->flip = flip;
	object->FillColor = FillColor;
	object->rect = rect;

	switch(OtherModes.cycle_type)
	{
		case CYCLE_TYPE_1:
			render_triangle_custom(visarea, render_delegate(FUNC(n64_rdp::SpanDraw1Cycle), this), start, (end - start) + 1, Spans + offset);
			break;
		case CYCLE_TYPE_2:
			render_triangle_custom(visarea, render_delegate(FUNC(n64_rdp::SpanDraw2Cycle), this), start, (end - start) + 1, Spans + offset);
			break;

		case CYCLE_TYPE_COPY:
			render_triangle_custom(visarea, render_delegate(FUNC(n64_rdp::SpanDrawCopy), this), start, (end - start) + 1, Spans + offset);
			break;

		case CYCLE_TYPE_FILL:
			render_triangle_custom(visarea, render_delegate(FUNC(n64_rdp::SpanDrawFill), this), start, (end - start) + 1, Spans + offset);
			break;
	}
	wait();
}

void n64_rdp::RGBAZClip(int sr, int sg, int sb, int sa, int *sz, rdp_span_aux *userdata)
{
	userdata->ShadeColor.i.r = m_special_9bit_clamptable[sr & 0x1ff];
	userdata->ShadeColor.i.g = m_special_9bit_clamptable[sg & 0x1ff];
	userdata->ShadeColor.i.b = m_special_9bit_clamptable[sb & 0x1ff];
	userdata->ShadeColor.i.a = m_special_9bit_clamptable[sa & 0x1ff];

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

void n64_rdp::RGBAZCorrectTriangle(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	if (userdata->CurrentPixCvg == 8)
	{
		*r >>= 2;
		*g >>= 2;
		*b >>= 2;
		*a >>= 2;
		*z = (*z >> 3) & 0x7ffff;
	}
	else
	{
		INT32 summand_xr = offx * SIGN13(object.SpanBase.m_span_dr >> 14);
		INT32 summand_yr = offy * SIGN13(object.SpanBase.m_span_drdy >> 14);
		INT32 summand_xb = offx * SIGN13(object.SpanBase.m_span_db >> 14);
		INT32 summand_yb = offy * SIGN13(object.SpanBase.m_span_dbdy >> 14);
		INT32 summand_xg = offx * SIGN13(object.SpanBase.m_span_dg >> 14);
		INT32 summand_yg = offy * SIGN13(object.SpanBase.m_span_dgdy >> 14);
		INT32 summand_xa = offx * SIGN13(object.SpanBase.m_span_da >> 14);
		INT32 summand_ya = offy * SIGN13(object.SpanBase.m_span_dady >> 14);

		INT32 summand_xz = offx * SIGN22(object.SpanBase.m_span_dz >> 10);
		INT32 summand_yz = offy * SIGN22(object.SpanBase.m_span_dzdy >> 10);

		*r = ((*r << 2) + summand_xr + summand_yr) >> 4;
		*g = ((*g << 2) + summand_xg + summand_yg) >> 4;
		*b = ((*b << 2) + summand_xb + summand_yb) >> 4;
		*a = ((*a << 2) + summand_xa + summand_ya) >> 4;
		*z = (((*z << 2) + summand_xz + summand_yz) >> 5) & 0x7ffff;
	}
}

void n64_rdp::SpanDraw1Cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	int clipx1 = object.Scissor.m_xh;
	int clipx2 = object.Scissor.m_xl;
	n64_rdp *m_rdp = object.m_rdp;
	int tilenum = object.tilenum;
	bool flip = object.flip;

	SpanParam r; r.w = extent.param[SPAN_R].start;
	SpanParam g; g.w = extent.param[SPAN_G].start;
	SpanParam b; b.w = extent.param[SPAN_B].start;
	SpanParam a; a.w = extent.param[SPAN_A].start;
	SpanParam z; z.w = extent.param[SPAN_Z].start;
	SpanParam s; s.w = extent.param[SPAN_S].start;
	SpanParam t; t.w = extent.param[SPAN_T].start;
	SpanParam w; w.w = extent.param[SPAN_W].start;

	UINT32 zb = object.MiscState.ZBAddress >> 1;
	UINT32 zhb = zb;
	UINT8 offx = 0, offy = 0;

	INT32 tile1 = tilenum;

	rdp_span_aux *userdata = (rdp_span_aux*)extent.userdata;

	INT32 m_clamp_s_diff[8];
	INT32 m_clamp_t_diff[8];
	m_rdp->TexPipe.CalculateClampDiffs(tile1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

	bool partialreject = (userdata->ColorInputs.blender2b_a[0] == &userdata->InvPixelColor.i.a && userdata->ColorInputs.blender1b_a[0] == &userdata->PixelColor.i.a);
	bool bsel0 = (userdata->ColorInputs.blender2b_a[0] == &userdata->MemoryColor.i.a);

	int drinc = flip ? (object.SpanBase.m_span_dr) : -object.SpanBase.m_span_dr;
	int dginc = flip ? (object.SpanBase.m_span_dg) : -object.SpanBase.m_span_dg;
	int dbinc = flip ? (object.SpanBase.m_span_db) : -object.SpanBase.m_span_db;
	int dainc = flip ? (object.SpanBase.m_span_da) : -object.SpanBase.m_span_da;
	int dzinc = flip ? (object.SpanBase.m_span_dz) : -object.SpanBase.m_span_dz;
	int dsinc = flip ? (object.SpanBase.m_span_ds) : -object.SpanBase.m_span_ds;
	int dtinc = flip ? (object.SpanBase.m_span_dt) : -object.SpanBase.m_span_dt;
	int dwinc = flip ? (object.SpanBase.m_span_dw) : -object.SpanBase.m_span_dw;
	int dzpix = object.SpanBase.m_span_dzpix;
	int xinc = flip ? 1 : -1;

	int fb_index = object.MiscState.FBWidth * scanline;

	int cdith = 0;
	int adith = 0;

	int xstart = extent.startx;
	int xend = userdata->m_unscissored_rx;
	int xend_scissored = extent.stopx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);
	UINT32 fir, fig, fib;

	if(object.OtherModes.z_source_sel)
	{
		z.w = ((UINT32)object.MiscState.PrimitiveZ) << 16;
		dzpix = object.MiscState.PrimitiveDZ;
		dzinc = 0;
	}

	userdata->m_start_span = true;
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

		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			m_rdp->lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			if (userdata->m_start_span)
			{
				if (object.OtherModes.persp_tex_en)
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
				sss = userdata->m_precomp_s;
				sst = userdata->m_precomp_t;
			}

			m_rdp->TexPipe.LOD1Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, userdata, object);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			RGBAZClip(sr, sg, sb, sa, &sz, userdata);

			m_rdp->TexPipe.Cycle(&userdata->Texel0Color, &userdata->Texel0Color, sss, sst, tilenum, 0, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

			m_rdp->ColorCombiner1Cycle(userdata);

			//Alpha coverage combiner
			GetAlphaCvg(&userdata->PixelColor.i.a, userdata, object);

			UINT32 curpixel = fb_index + x;
			UINT32 zbcur = zb + curpixel;
			UINT32 zhbcur = zhb + curpixel;

			((this)->*(_Read[((object.MiscState.FBSize - 2) << 1) | object.OtherModes.image_read_en]))(curpixel, userdata, object);

			if(m_rdp->ZCompare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				m_rdp->GetDitherValues(scanline, j, &cdith, &adith, object);

				bool rendered = m_rdp->Blender.Blend1Cycle(&fir, &fig, &fib, cdith, adith, partialreject, bsel0, userdata, object);

                if (rendered)
				{
					((this)->*(_Write[((object.MiscState.FBSize - 2) << 3) | (object.OtherModes.cvg_dest << 1) | userdata->BlendEnable]))(curpixel, fir, fig, fib, userdata, object);

					if (object.OtherModes.z_update_en)
					{
						m_rdp->ZStore(zbcur, zhbcur, sz, userdata->m_dzpix_enc);
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

void n64_rdp::SpanDraw2Cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	int clipx1 = object.Scissor.m_xh;
	int clipx2 = object.Scissor.m_xl;
	n64_rdp *m_rdp = object.m_rdp;
	int tilenum = object.tilenum;
	bool flip = object.flip;

	SpanParam r; r.w = extent.param[SPAN_R].start;
	SpanParam g; g.w = extent.param[SPAN_G].start;
	SpanParam b; b.w = extent.param[SPAN_B].start;
	SpanParam a; a.w = extent.param[SPAN_A].start;
	SpanParam z; z.w = extent.param[SPAN_Z].start;
	SpanParam s; s.w = extent.param[SPAN_S].start;
	SpanParam t; t.w = extent.param[SPAN_T].start;
	SpanParam w; w.w = extent.param[SPAN_W].start;

	UINT32 zb = object.MiscState.ZBAddress >> 1;
	UINT32 zhb = zb;
	UINT8 offx = 0, offy = 0;

	INT32 tile2 = (tilenum + 1) & 7;
	INT32 tile1 = tilenum;
	UINT32 prim_tile = tilenum;

	int newtile1 = tile1;
	INT32 news = 0;
	INT32 newt = 0;

	rdp_span_aux *userdata = (rdp_span_aux*)extent.userdata;

	INT32 m_clamp_s_diff[8];
	INT32 m_clamp_t_diff[8];
	m_rdp->TexPipe.CalculateClampDiffs(tile1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

	bool partialreject = (userdata->ColorInputs.blender2b_a[1] == &userdata->InvPixelColor.i.a && userdata->ColorInputs.blender1b_a[1] == &userdata->PixelColor.i.a);
	bool bsel0 = (userdata->ColorInputs.blender2b_a[0] == &userdata->MemoryColor.i.a);
	bool bsel1 = (userdata->ColorInputs.blender2b_a[1] == &userdata->MemoryColor.i.a);

	int dzpix = object.SpanBase.m_span_dzpix;
	int drinc = flip ? (object.SpanBase.m_span_dr) : -object.SpanBase.m_span_dr;
	int dginc = flip ? (object.SpanBase.m_span_dg) : -object.SpanBase.m_span_dg;
	int dbinc = flip ? (object.SpanBase.m_span_db) : -object.SpanBase.m_span_db;
	int dainc = flip ? (object.SpanBase.m_span_da) : -object.SpanBase.m_span_da;
	int dzinc = flip ? (object.SpanBase.m_span_dz) : -object.SpanBase.m_span_dz;
	int dsinc = flip ? (object.SpanBase.m_span_ds) : -object.SpanBase.m_span_ds;
	int dtinc = flip ? (object.SpanBase.m_span_dt) : -object.SpanBase.m_span_dt;
	int dwinc = flip ? (object.SpanBase.m_span_dw) : -object.SpanBase.m_span_dw;
	int xinc = flip ? 1 : -1;

	int fb_index = object.MiscState.FBWidth * scanline;

	int cdith = 0;
	int adith = 0;

	int xstart = extent.startx;
	int xend = userdata->m_unscissored_rx;
	int xend_scissored = extent.stopx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);
	UINT32 fir, fig, fib;

	if(object.OtherModes.z_source_sel)
	{
		z.w = ((UINT32)object.MiscState.PrimitiveZ) << 16;
		dzpix = object.MiscState.PrimitiveDZ;
		dzinc = 0;
	}

	userdata->m_start_span = true;
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

		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			m_rdp->lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			if (userdata->m_start_span)
			{
				if (object.OtherModes.persp_tex_en)
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
				sss = userdata->m_precomp_s;
				sst = userdata->m_precomp_t;
			}

			m_rdp->TexPipe.LOD2Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2, userdata, object);

			news = userdata->m_precomp_s;
			newt = userdata->m_precomp_t;
			m_rdp->TexPipe.LOD2CycleLimited(&news, &newt, s.w + dsinc, t.w + dtinc, w.w + dwinc, dsinc, dtinc, dwinc, prim_tile, &newtile1, object);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			RGBAZClip(sr, sg, sb, sa, &sz, userdata);

			m_rdp->TexPipe.Cycle(&userdata->Texel0Color, &userdata->Texel0Color, sss, sst, tile1, 0, userdata, object, m_clamp_s_diff, m_clamp_t_diff);
			m_rdp->TexPipe.Cycle(&userdata->Texel1Color, &userdata->Texel0Color, sss, sst, tile2, 1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

			m_rdp->TexPipe.Cycle(&userdata->NextTexelColor, &userdata->NextTexelColor, sss, sst, tile2, 1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

			m_rdp->ColorCombiner2Cycle(userdata);

			//Alpha coverage combiner
			GetAlphaCvg(&userdata->PixelColor.i.a, userdata, object);

			UINT32 curpixel = fb_index + x;
			UINT32 zbcur = zb + curpixel;
			UINT32 zhbcur = zhb + curpixel;

			((this)->*(_Read[((object.MiscState.FBSize - 2) << 1) | object.OtherModes.image_read_en]))(curpixel, userdata, object);

			if(m_rdp->ZCompare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				m_rdp->GetDitherValues(scanline, j, &cdith, &adith, object);

				bool rendered = m_rdp->Blender.Blend2Cycle(&fir, &fig, &fib, cdith, adith, partialreject, bsel0, bsel1, userdata, object);

                if (rendered)
				{
					((this)->*(_Write[((object.MiscState.FBSize - 2) << 3) | (object.OtherModes.cvg_dest << 1) | userdata->BlendEnable]))(curpixel, fir, fig, fib, userdata, object);
					if (object.OtherModes.z_update_en)
					{
						m_rdp->ZStore(zbcur, zhbcur, sz, userdata->m_dzpix_enc);
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

void n64_rdp::SpanDrawCopy(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	int clipx1 = object.Scissor.m_xh;
	int clipx2 = object.Scissor.m_xl;
	n64_rdp *m_rdp = object.m_rdp;
	int tilenum = object.tilenum;
	bool flip = object.flip;

	SpanParam s; s.w = extent.param[SPAN_S].start;
	SpanParam t; t.w = extent.param[SPAN_T].start;

	int ds = object.SpanBase.m_span_ds / 4;
	int dt = object.SpanBase.m_span_dt / 4;
	int dsinc = flip ? (ds) : -ds;
	int dtinc = flip ? (dt) : -dt;
	int xinc = flip ? 1 : -1;

	int fb_index = object.MiscState.FBWidth * scanline;

	rdp_span_aux *userdata = (rdp_span_aux*)extent.userdata;
	int xstart = extent.startx;
	int xend = userdata->m_unscissored_rx;
	int xend_scissored = extent.stopx;

	int x = xend;

	int length = flip ? (xstart - xend) : (xend - xstart);

	for (int j = 0; j <= length; j++)
	{
		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			INT32 sss = s.h.h;
			INT32 sst = t.h.h;
			m_rdp->TexPipe.Copy(&userdata->Texel0Color, sss, sst, tilenum, object, userdata);

			UINT32 curpixel = fb_index + x;
			if ((userdata->Texel0Color.i.a != 0) || (!object.OtherModes.alpha_compare_en))
			{
				((this)->*(_Copy[object.MiscState.FBSize - 2]))(curpixel, userdata->Texel0Color.i.r, userdata->Texel0Color.i.g, userdata->Texel0Color.i.b, userdata->Texel0Color.i.a ? 7 : 0, object);
			}
		}

		s.w += dsinc;
		t.w += dtinc;
		x += xinc;
	}
}

void n64_rdp::SpanDrawFill(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	bool flip = object.flip;

	int clipx1 = object.Scissor.m_xh;
	int clipx2 = object.Scissor.m_xl;

	int xinc = flip ? 1 : -1;

	int fb_index = object.MiscState.FBWidth * scanline;

	int xstart = extent.startx;
	int xend_scissored = extent.stopx;

	int x = xend_scissored;

	int length = flip ? (xstart - xend_scissored) : (xend_scissored - xstart);

	for (int j = 0; j <= length; j++)
	{
		if (x >= clipx1 && x < clipx2)
		{
			((this)->*(_Fill[object.MiscState.FBSize - 2]))(fb_index + x, object);
		}

		x += xinc;
	}
}
