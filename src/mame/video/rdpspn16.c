// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor span-drawing functions
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

#define LookUpCC(A, B, C, D) GetCCLUT2()[(GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

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
	userdata->ShadeColor.i.r = s_special_9bit_clamptable[sr & 0x1ff];
	userdata->ShadeColor.i.g = s_special_9bit_clamptable[sg & 0x1ff];
	userdata->ShadeColor.i.b = s_special_9bit_clamptable[sb & 0x1ff];
	userdata->ShadeColor.i.a = s_special_9bit_clamptable[sa & 0x1ff];

	INT32 zanded = (*sz) & 0x60000;

	zanded >>= 17;
	switch(zanded)
	{
		case 0: *sz &= 0x3ffff;                                         break;
		case 1: *sz &= 0x3ffff;                                         break;
		case 2: *sz = 0x3ffff;                                          break;
		case 3: *sz = 0x3ffff;                                          break;
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

inline void n64_rdp::write_pixel(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	if (object.MiscState.FBSize == 2) // 16-bit framebuffer
	{
		const UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;

		UINT16 finalcolor;
		if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
		{
			finalcolor = RREADIDX16(fb) & 0xfffe;
		}
		else
		{
			finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
		}

		switch (object.OtherModes.cvg_dest)
		{
			case 0:
				if (userdata->BlendEnable)
				{
					UINT32 finalcvg = userdata->CurrentPixCvg + userdata->CurrentMemCvg;
					if (finalcvg & 8)
					{
						finalcvg = 7;
					}
					RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
					HWRITEADDR8(fb, finalcvg & 3);
				}
				else
				{
					const UINT32 finalcvg = (userdata->CurrentPixCvg - 1) & 7;
					RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
					HWRITEADDR8(fb, finalcvg & 3);
				}
				break;
			case 1:
			{
				const UINT32 finalcvg = (userdata->CurrentPixCvg + userdata->CurrentMemCvg) & 7;
				RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
				HWRITEADDR8(fb, finalcvg & 3);
				break;
			}
			case 2:
				RWRITEIDX16(fb, finalcolor | 1);
				HWRITEADDR8(fb, 3);
				break;
			case 3:
				RWRITEIDX16(fb, finalcolor | (userdata->CurrentMemCvg >> 2));
				HWRITEADDR8(fb, userdata->CurrentMemCvg & 3);
				break;
		}
	}
	else // 32-bit framebuffer
	{
		const UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;

		UINT32 finalcolor;
		if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
		{
			finalcolor = RREADIDX32(fb) & 0xffffff00;
		}
		else
		{
			finalcolor = (r << 24) | (g << 16) | (b << 8);
		}

		switch (object.OtherModes.cvg_dest)
		{
			case 0:
				if (userdata->BlendEnable)
				{
					UINT32 finalcvg = userdata->CurrentPixCvg + userdata->CurrentMemCvg;
					if (finalcvg & 8)
					{
						finalcvg = 7;
					}

					RWRITEIDX32(fb, finalcolor | (finalcvg << 5));
				}
				else
				{
					RWRITEIDX32(fb, finalcolor | (((userdata->CurrentPixCvg - 1) & 7) << 5));
				}
				break;
			case 1:
				RWRITEIDX32(fb, finalcolor | (((userdata->CurrentPixCvg + userdata->CurrentMemCvg) & 7) << 5));
				break;
			case 2:
				RWRITEIDX32(fb, finalcolor | 0xE0);
				break;
			case 3:
				RWRITEIDX32(fb, finalcolor | (userdata->CurrentMemCvg << 5));
				break;
		}
	}
}

inline void n64_rdp::read_pixel(UINT32 curpixel, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	if (object.MiscState.FBSize == 2) // 16-bit framebuffer
	{
		UINT16 fword;

		if (object.OtherModes.image_read_en)
		{
			fword = RREADIDX16((object.MiscState.FBAddress >> 1) + curpixel);
			UINT8 hbyte = HREADADDR8((object.MiscState.FBAddress >> 1) + curpixel);
			userdata->MemoryColor.i.a = userdata->CurrentMemCvg << 5;
			userdata->CurrentMemCvg = ((fword & 1) << 2) | (hbyte & 3);
		}
		else
		{
			fword = RREADIDX16((object.MiscState.FBAddress >> 1) + curpixel);
			userdata->MemoryColor.i.a = 0xff;
			userdata->CurrentMemCvg = 7;
		}

		userdata->MemoryColor.i.r = GETHICOL(fword);
		userdata->MemoryColor.i.g = GETMEDCOL(fword);
		userdata->MemoryColor.i.b = GETLOWCOL(fword);
	}
	else // 32-bit framebuffer
	{
		UINT32 mem;

		if (object.OtherModes.image_read_en)
		{
			mem = RREADIDX32((object.MiscState.FBAddress >> 2) + curpixel);
			userdata->MemoryColor.i.a = (mem) & 0xff;
			userdata->CurrentMemCvg = (mem >> 5) & 7;
		}
		else
		{
			mem = RREADIDX32((object.MiscState.FBAddress >> 2) + curpixel);
			userdata->MemoryColor.i.a = 0xff;
			userdata->CurrentMemCvg = 7;
		}

		userdata->MemoryColor.i.r = (mem >> 24) & 0xff;
		userdata->MemoryColor.i.g = (mem >> 16) & 0xff;
		userdata->MemoryColor.i.b = (mem >> 8) & 0xff;
	}
}

inline void n64_rdp::copy_pixel(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, int CurrentPixCvg, const rdp_poly_state &object)
{
	if (object.MiscState.FBSize == 2) // 16-bit framebuffer
	{
		RWRITEIDX16((object.MiscState.FBAddress >> 1) + curpixel, ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((CurrentPixCvg >> 2) & 1));
		HWRITEADDR8((object.MiscState.FBAddress >> 1) + curpixel, CurrentPixCvg & 3);
	}
	else // 32-bit framebuffer
	{
		RWRITEIDX32((object.MiscState.FBAddress >> 2) + curpixel, (r << 24) | (g << 16) | (b << 8) | (CurrentPixCvg << 5));
	}
}

inline void n64_rdp::fill_pixel(UINT32 curpixel, const rdp_poly_state &object)
{
	if (object.MiscState.FBSize == 2) // 16-bit framebuffer
	{
		UINT16 val;
		if (curpixel & 1)
		{
			val = object.FillColor & 0xffff;
		}
		else
		{
			val = (object.FillColor >> 16) & 0xffff;
		}
		RWRITEIDX16((object.MiscState.FBAddress >> 1) + curpixel, val);
		HWRITEADDR8((object.MiscState.FBAddress >> 1) + curpixel, ((val & 1) << 1) | (val & 1));
	}
	else // 32-bit framebuffer
	{
		RWRITEIDX32((object.MiscState.FBAddress >> 2) + curpixel, object.FillColor);
		HWRITEADDR8((object.MiscState.FBAddress >> 1) + (curpixel << 1), (object.FillColor & 0x10000) ? 3 : 0);
		HWRITEADDR8((object.MiscState.FBAddress >> 1) + (curpixel << 1) + 1, (object.FillColor & 0x1) ? 3 : 0);
	}
}

void n64_rdp::SpanDraw1Cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	assert(object.MiscState.FBSize >= 2 && object.MiscState.FBSize < 4);

	const int clipx1 = object.Scissor.m_xh;
	const int clipx2 = object.Scissor.m_xl;
	const int tilenum = object.tilenum;
	const bool flip = object.flip;

	SpanParam r; r.w = extent.param[SPAN_R].start;
	SpanParam g; g.w = extent.param[SPAN_G].start;
	SpanParam b; b.w = extent.param[SPAN_B].start;
	SpanParam a; a.w = extent.param[SPAN_A].start;
	SpanParam z; z.w = extent.param[SPAN_Z].start;
	SpanParam s; s.w = extent.param[SPAN_S].start;
	SpanParam t; t.w = extent.param[SPAN_T].start;
	SpanParam w; w.w = extent.param[SPAN_W].start;

	const UINT32 zb = object.MiscState.ZBAddress >> 1;
	const UINT32 zhb = object.MiscState.ZBAddress;
	UINT8 offx = 0, offy = 0;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux *userdata = (rdp_span_aux*)extent.userdata;

	INT32 m_clamp_s_diff[8];
	INT32 m_clamp_t_diff[8];
	TexPipe.CalculateClampDiffs(tilenum, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

	const bool partialreject = (userdata->ColorInputs.blender2b_a[0] == &userdata->InvPixelColor.i.a && userdata->ColorInputs.blender1b_a[0] == &userdata->PixelColor.i.a);
	const int sel0 = (OtherModes.force_blend ? 2 : 0) | ((userdata->ColorInputs.blender2b_a[0] == &userdata->MemoryColor.i.a) ? 1 : 0);

	int drinc, dginc, dbinc, dainc;
	int dzinc, dzpix;
	int dsinc, dtinc, dwinc;
	int xinc;

	if (!flip)
	{
		drinc = -object.SpanBase.m_span_dr;
		dginc = -object.SpanBase.m_span_dg;
		dbinc = -object.SpanBase.m_span_db;
		dainc = -object.SpanBase.m_span_da;
		dzinc = -object.SpanBase.m_span_dz;
		dsinc = -object.SpanBase.m_span_ds;
		dtinc = -object.SpanBase.m_span_dt;
		dwinc = -object.SpanBase.m_span_dw;
		xinc = -1;
	}
	else
	{
		drinc = object.SpanBase.m_span_dr;
		dginc = object.SpanBase.m_span_dg;
		dbinc = object.SpanBase.m_span_db;
		dainc = object.SpanBase.m_span_da;
		dzinc = object.SpanBase.m_span_dz;
		dsinc = object.SpanBase.m_span_ds;
		dtinc = object.SpanBase.m_span_dt;
		dwinc = object.SpanBase.m_span_dw;
		xinc = 1;
	}

	const int fb_index = object.MiscState.FBWidth * scanline;

	const int xstart = extent.startx;
	const int xend = userdata->m_unscissored_rx;
	const int xend_scissored = extent.stopx;

	int x = xend;

	const int length = flip ? (xstart - xend) : (xend - xstart);
	UINT32 fir, fig, fib;

	if(object.OtherModes.z_source_sel)
	{
		z.w = ((UINT32)object.MiscState.PrimitiveZ) << 16;
		dzpix = object.MiscState.PrimitiveDZ;
		dzinc = 0;
	}
	else
	{
		dzpix = object.SpanBase.m_span_dzpix;
	}

	if (object.MiscState.FBSize < 2 || object.MiscState.FBSize > 4)
		fatalerror("unsupported FBSize %d\n", object.MiscState.FBSize);

	const int blend_index = (object.OtherModes.alpha_cvg_select ? 2 : 0) | ((object.OtherModes.rgb_dither_sel < 3) ? 1 : 0);
	const int cycle0 = ((object.OtherModes.sample_type & 1) << 1) | (object.OtherModes.bi_lerp0 & 1);
	const int acmode = (object.OtherModes.alpha_compare_en ? 2 : 0) | (object.OtherModes.dither_alpha_en ? 1 : 0);

	INT32 sss = 0;
	INT32 sst = 0;

	if (object.OtherModes.persp_tex_en)
	{
		TCDiv(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		TCDivNoPersp(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (int j = 0; j <= length; j++)
	{
		int sr = r.w >> 14;
		int sg = g.w >> 14;
		int sb = b.w >> 14;
		int sa = a.w >> 14;
		int sz = (z.w >> 10) & 0x3fffff;
		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			TexPipe.LOD1Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, userdata, object);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			RGBAZClip(sr, sg, sb, sa, &sz, userdata);

			((TexPipe).*(TexPipe.cycle[cycle0]))(&userdata->Texel0Color, &userdata->Texel0Color, sss, sst, tilenum, 0, userdata, object, m_clamp_s_diff, m_clamp_t_diff);
			//TexPipe.Cycle(&userdata->Texel0Color, &userdata->Texel0Color, sss, sst, tilenum, 0, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

			userdata->NoiseColor.i.r = userdata->NoiseColor.i.g = userdata->NoiseColor.i.b = rand() << 3; // Not accurate

			userdata->PixelColor.i.r = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_r[1],*userdata->ColorInputs.combiner_rgbsub_b_r[1],*userdata->ColorInputs.combiner_rgbmul_r[1],*userdata->ColorInputs.combiner_rgbadd_r[1]);
			userdata->PixelColor.i.g = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_g[1],*userdata->ColorInputs.combiner_rgbsub_b_g[1],*userdata->ColorInputs.combiner_rgbmul_g[1],*userdata->ColorInputs.combiner_rgbadd_g[1]);
			userdata->PixelColor.i.b = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_b[1],*userdata->ColorInputs.combiner_rgbsub_b_b[1],*userdata->ColorInputs.combiner_rgbmul_b[1],*userdata->ColorInputs.combiner_rgbadd_b[1]);
			userdata->PixelColor.i.a = AlphaCombinerEquation(*userdata->ColorInputs.combiner_alphasub_a[1],*userdata->ColorInputs.combiner_alphasub_b[1],*userdata->ColorInputs.combiner_alphamul[1],*userdata->ColorInputs.combiner_alphaadd[1]);

			//Alpha coverage combiner
			GetAlphaCvg(&userdata->PixelColor.i.a, userdata, object);

			const UINT32 curpixel = fb_index + x;
			const UINT32 zbcur = zb + curpixel;
			const UINT32 zhbcur = zhb + curpixel;

			read_pixel(curpixel, userdata, object);

			if(ZCompare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				int cdith = 0;
				int adith = 0;
				GetDitherValues(scanline, j, &cdith, &adith, object);

				bool rendered = ((&Blender)->*(Blender.blend1[(userdata->BlendEnable << 2) | blend_index]))(&fir, &fig, &fib, cdith, adith, partialreject, sel0, acmode, userdata, object);

				if (rendered)
				{
					write_pixel(curpixel, fir, fig, fib, userdata, object);
					if (object.OtherModes.z_update_en)
					{
						ZStore(object, zbcur, zhbcur, sz, userdata->m_dzpix_enc);
					}
				}
			}

			sss = userdata->m_precomp_s;
			sst = userdata->m_precomp_t;
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
	assert(object.MiscState.FBSize >= 2 && object.MiscState.FBSize < 4);

	const int clipx1 = object.Scissor.m_xh;
	const int clipx2 = object.Scissor.m_xl;
	const int tilenum = object.tilenum;
	const bool flip = object.flip;

	SpanParam r; r.w = extent.param[SPAN_R].start;
	SpanParam g; g.w = extent.param[SPAN_G].start;
	SpanParam b; b.w = extent.param[SPAN_B].start;
	SpanParam a; a.w = extent.param[SPAN_A].start;
	SpanParam z; z.w = extent.param[SPAN_Z].start;
	SpanParam s; s.w = extent.param[SPAN_S].start;
	SpanParam t; t.w = extent.param[SPAN_T].start;
	SpanParam w; w.w = extent.param[SPAN_W].start;

	const UINT32 zb = object.MiscState.ZBAddress >> 1;
	const UINT32 zhb = object.MiscState.ZBAddress;

	INT32 tile2 = (tilenum + 1) & 7;
	INT32 tile1 = tilenum;
	const UINT32 prim_tile = tilenum;

	int newtile1 = tile1;
	INT32 news = 0;
	INT32 newt = 0;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux *userdata = (rdp_span_aux*)extent.userdata;

	INT32 m_clamp_s_diff[8];
	INT32 m_clamp_t_diff[8];
	TexPipe.CalculateClampDiffs(tile1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

	bool partialreject = (userdata->ColorInputs.blender2b_a[1] == &userdata->InvPixelColor.i.a && userdata->ColorInputs.blender1b_a[1] == &userdata->PixelColor.i.a);
	int sel0 = (OtherModes.force_blend ? 2 : 0) | ((userdata->ColorInputs.blender2b_a[0] == &userdata->MemoryColor.i.a) ? 1 : 0);
	int sel1 = (OtherModes.force_blend ? 2 : 0) | ((userdata->ColorInputs.blender2b_a[1] == &userdata->MemoryColor.i.a) ? 1 : 0);

	int drinc, dginc, dbinc, dainc;
	int dzinc, dzpix;
	int dsinc, dtinc, dwinc;
	int xinc;

	if (!flip)
	{
		drinc = -object.SpanBase.m_span_dr;
		dginc = -object.SpanBase.m_span_dg;
		dbinc = -object.SpanBase.m_span_db;
		dainc = -object.SpanBase.m_span_da;
		dzinc = -object.SpanBase.m_span_dz;
		dsinc = -object.SpanBase.m_span_ds;
		dtinc = -object.SpanBase.m_span_dt;
		dwinc = -object.SpanBase.m_span_dw;
		xinc = -1;
	}
	else
	{
		drinc = object.SpanBase.m_span_dr;
		dginc = object.SpanBase.m_span_dg;
		dbinc = object.SpanBase.m_span_db;
		dainc = object.SpanBase.m_span_da;
		dzinc = object.SpanBase.m_span_dz;
		dsinc = object.SpanBase.m_span_ds;
		dtinc = object.SpanBase.m_span_dt;
		dwinc = object.SpanBase.m_span_dw;
		xinc = 1;
	}

	const int fb_index = object.MiscState.FBWidth * scanline;

	int cdith = 0;
	int adith = 0;

	const int xstart = extent.startx;
	const int xend = userdata->m_unscissored_rx;
	const int xend_scissored = extent.stopx;

	int x = xend;

	const int length = flip ? (xstart - xend) : (xend - xstart);
	UINT32 fir, fig, fib;

	if(object.OtherModes.z_source_sel)
	{
		z.w = ((UINT32)object.MiscState.PrimitiveZ) << 16;
		dzpix = object.MiscState.PrimitiveDZ;
		dzinc = 0;
	}
	else
	{
		dzpix = object.SpanBase.m_span_dzpix;
	}

	if (object.MiscState.FBSize < 2 || object.MiscState.FBSize > 4)
		fatalerror("unsupported FBSize %d\n", object.MiscState.FBSize);

	const int blend_index = (object.OtherModes.alpha_cvg_select ? 2 : 0) | ((object.OtherModes.rgb_dither_sel < 3) ? 1 : 0);
	const int cycle0 = ((object.OtherModes.sample_type & 1) << 1) | (object.OtherModes.bi_lerp0 & 1);
	const int cycle1 = ((object.OtherModes.sample_type & 1) << 1) | (object.OtherModes.bi_lerp1 & 1);
	const int acmode = (object.OtherModes.alpha_compare_en ? 2 : 0) | (object.OtherModes.dither_alpha_en ? 1 : 0);

	INT32 sss = 0;
	INT32 sst = 0;

	if (object.OtherModes.persp_tex_en)
	{
		TCDiv(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		TCDivNoPersp(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (int j = 0; j <= length; j++)
	{
		int sr = r.w >> 14;
		int sg = g.w >> 14;
		int sb = b.w >> 14;
		int sa = a.w >> 14;
		int sz = (z.w >> 10) & 0x3fffff;
		Color c1;
		Color c2;

		bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			UINT32 compidx = compressed_cvmasks[userdata->m_cvg[x]];
			userdata->CurrentPixCvg = cvarray[compidx].cvg;
			userdata->CurrentCvgBit = cvarray[compidx].cvbit;
			UINT8 offx = cvarray[compidx].xoff;
			UINT8 offy = cvarray[compidx].yoff;
			//lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			TexPipe.LOD2Cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2, userdata, object);

			news = userdata->m_precomp_s;
			newt = userdata->m_precomp_t;
			TexPipe.LOD2CycleLimited(&news, &newt, s.w + dsinc, t.w + dtinc, w.w + dwinc, dsinc, dtinc, dwinc, prim_tile, &newtile1, object);

			RGBAZCorrectTriangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			RGBAZClip(sr, sg, sb, sa, &sz, userdata);

			((TexPipe).*(TexPipe.cycle[cycle0]))(&userdata->Texel0Color, &userdata->Texel0Color, sss, sst, tile1, 0, userdata, object, m_clamp_s_diff, m_clamp_t_diff);
			((TexPipe).*(TexPipe.cycle[cycle1]))(&userdata->Texel1Color, &userdata->Texel0Color, sss, sst, tile2, 1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);
			((TexPipe).*(TexPipe.cycle[cycle1]))(&userdata->NextTexelColor, &userdata->NextTexelColor, sss, sst, tile2, 1, userdata, object, m_clamp_s_diff, m_clamp_t_diff);

			userdata->NoiseColor.i.r = userdata->NoiseColor.i.g = userdata->NoiseColor.i.b = rand() << 3; // Not accurate
			userdata->CombinedColor.i.r = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_r[0],
																*userdata->ColorInputs.combiner_rgbsub_b_r[0],
																*userdata->ColorInputs.combiner_rgbmul_r[0],
																*userdata->ColorInputs.combiner_rgbadd_r[0]);
			userdata->CombinedColor.i.g = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_g[0],
																*userdata->ColorInputs.combiner_rgbsub_b_g[0],
																*userdata->ColorInputs.combiner_rgbmul_g[0],
																*userdata->ColorInputs.combiner_rgbadd_g[0]);
			userdata->CombinedColor.i.b = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_b[0],
																*userdata->ColorInputs.combiner_rgbsub_b_b[0],
																*userdata->ColorInputs.combiner_rgbmul_b[0],
																*userdata->ColorInputs.combiner_rgbadd_b[0]);
			userdata->CombinedColor.i.a = AlphaCombinerEquation(*userdata->ColorInputs.combiner_alphasub_a[0],
																*userdata->ColorInputs.combiner_alphasub_b[0],
																*userdata->ColorInputs.combiner_alphamul[0],
																*userdata->ColorInputs.combiner_alphaadd[0]);

			userdata->Texel0Color = userdata->Texel1Color;
			userdata->Texel1Color = userdata->NextTexelColor;

			userdata->PixelColor.i.r = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_r[1],
																*userdata->ColorInputs.combiner_rgbsub_b_r[1],
																*userdata->ColorInputs.combiner_rgbmul_r[1],
																*userdata->ColorInputs.combiner_rgbadd_r[1]);
			userdata->PixelColor.i.g = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_g[1],
																*userdata->ColorInputs.combiner_rgbsub_b_g[1],
																*userdata->ColorInputs.combiner_rgbmul_g[1],
																*userdata->ColorInputs.combiner_rgbadd_g[1]);
			userdata->PixelColor.i.b = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_b[1],
																*userdata->ColorInputs.combiner_rgbsub_b_b[1],
																*userdata->ColorInputs.combiner_rgbmul_b[1],
																*userdata->ColorInputs.combiner_rgbadd_b[1]);
			userdata->PixelColor.i.a = AlphaCombinerEquation(*userdata->ColorInputs.combiner_alphasub_a[1],
																*userdata->ColorInputs.combiner_alphasub_b[1],
																*userdata->ColorInputs.combiner_alphamul[1],
																*userdata->ColorInputs.combiner_alphaadd[1]);

			//Alpha coverage combiner
			GetAlphaCvg(&userdata->PixelColor.i.a, userdata, object);

			UINT32 curpixel = fb_index + x;
			UINT32 zbcur = zb + curpixel;
			UINT32 zhbcur = zhb + curpixel;

			read_pixel(curpixel, userdata, object);

			if(ZCompare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				GetDitherValues(scanline, j, &cdith, &adith, object);

				bool rendered = ((&Blender)->*(Blender.blend2[(userdata->BlendEnable << 2) | blend_index]))(&fir, &fig, &fib, cdith, adith, partialreject, sel0, sel1, acmode, userdata, object);

				if (rendered)
				{
					write_pixel(curpixel, fir, fig, fib, userdata, object);
					if (object.OtherModes.z_update_en)
					{
						ZStore(object, zbcur, zhbcur, sz, userdata->m_dzpix_enc);
					}
				}
			}
			sss = userdata->m_precomp_s;
			sst = userdata->m_precomp_t;
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
	assert(object.MiscState.FBSize >= 2 && object.MiscState.FBSize < 4);

	int clipx1 = object.Scissor.m_xh;
	int clipx2 = object.Scissor.m_xl;
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
			TexPipe.Copy(&userdata->Texel0Color, sss, sst, tilenum, object, userdata);

			UINT32 curpixel = fb_index + x;
			if ((userdata->Texel0Color.i.a != 0) || (!object.OtherModes.alpha_compare_en))
			{
				copy_pixel(curpixel, userdata->Texel0Color.i.r, userdata->Texel0Color.i.g, userdata->Texel0Color.i.b, userdata->Texel0Color.i.a ? 7 : 0, object);
			}
		}

		s.w += dsinc;
		t.w += dtinc;
		x += xinc;
	}
}

void n64_rdp::SpanDrawFill(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, int threadid)
{
	assert(object.MiscState.FBSize >= 2 && object.MiscState.FBSize < 4);

	const bool flip = object.flip;

	const int clipx1 = object.Scissor.m_xh;
	const int clipx2 = object.Scissor.m_xl;

	const int xinc = flip ? 1 : -1;

	const int fb_index = object.MiscState.FBWidth * scanline;

	const int xstart = extent.startx;
	const int xend_scissored = extent.stopx;

	int x = xend_scissored;

	const int length = flip ? (xstart - xend_scissored) : (xend_scissored - xstart);

	for (int j = 0; j <= length; j++)
	{
		if (x >= clipx1 && x < clipx2)
		{
			fill_pixel(fb_index + x, object);
		}

		x += xinc;
	}
}
