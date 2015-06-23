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
#include "video/rgbutil.h"

void n64_rdp::render_spans(INT32 start, INT32 end, INT32 tilenum, bool flip, extent_t* spans, bool rect, rdp_poly_state* object)
{
	const INT32 clipy1 = m_scissor.m_yh;
	const INT32 clipy2 = m_scissor.m_yl;
	INT32 offset = 0;

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
	memcpy(&object->m_misc_state, &m_misc_state, sizeof(misc_state_t));
	memcpy(&object->m_other_modes, &m_other_modes, sizeof(other_modes_t));
	memcpy(&object->m_span_base, &m_span_base, sizeof(span_base_t));
	memcpy(&object->m_scissor, &m_scissor, sizeof(rectangle_t));
	memcpy(&object->m_tiles, &m_tiles, 8 * sizeof(n64_tile_t));
	object->tilenum = tilenum;
	object->flip = flip;
	object->m_fill_color = m_fill_color;
	object->rect = rect;

	switch(m_other_modes.cycle_type)
	{
		case CYCLE_TYPE_1:
			render_triangle_custom(m_visarea, render_delegate(FUNC(n64_rdp::span_draw_1cycle), this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_2:
			render_triangle_custom(m_visarea, render_delegate(FUNC(n64_rdp::span_draw_2cycle), this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_COPY:
			render_triangle_custom(m_visarea, render_delegate(FUNC(n64_rdp::span_draw_copy), this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_FILL:
			render_triangle_custom(m_visarea, render_delegate(FUNC(n64_rdp::span_draw_fill), this), start, (end - start) + 1, spans + offset);
			break;
	}
	//wait();
}

void n64_rdp::rgbaz_clip(INT32 sr, INT32 sg, INT32 sb, INT32 sa, INT32* sz, rdp_span_aux* userdata)
{
	userdata->m_shade_color.set(sa, sr, sg, sb);
	userdata->m_shade_color.clamp_and_clear(0xfffffe00);
	UINT32 a = userdata->m_shade_color.get_a();
	userdata->m_shade_alpha.set(a, a, a, a);

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

void n64_rdp::rgbaz_correct_triangle(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	if (userdata->m_current_pix_cvg == 8)
	{
		*r >>= 2;
		*g >>= 2;
		*b >>= 2;
		*a >>= 2;
		*z = (*z >> 3) & 0x7ffff;
	}
	else
	{
		INT32 summand_xr = offx * SIGN13(object.m_span_base.m_span_dr >> 14);
		INT32 summand_yr = offy * SIGN13(object.m_span_base.m_span_drdy >> 14);
		INT32 summand_xb = offx * SIGN13(object.m_span_base.m_span_db >> 14);
		INT32 summand_yb = offy * SIGN13(object.m_span_base.m_span_dbdy >> 14);
		INT32 summand_xg = offx * SIGN13(object.m_span_base.m_span_dg >> 14);
		INT32 summand_yg = offy * SIGN13(object.m_span_base.m_span_dgdy >> 14);
		INT32 summand_xa = offx * SIGN13(object.m_span_base.m_span_da >> 14);
		INT32 summand_ya = offy * SIGN13(object.m_span_base.m_span_dady >> 14);

		INT32 summand_xz = offx * SIGN22(object.m_span_base.m_span_dz >> 10);
		INT32 summand_yz = offy * SIGN22(object.m_span_base.m_span_dzdy >> 10);

		*r = ((*r << 2) + summand_xr + summand_yr) >> 4;
		*g = ((*g << 2) + summand_xg + summand_yg) >> 4;
		*b = ((*b << 2) + summand_xb + summand_yb) >> 4;
		*a = ((*a << 2) + summand_xa + summand_ya) >> 4;
		*z = (((*z << 2) + summand_xz + summand_yz) >> 5) & 0x7ffff;
	}
}

inline void n64_rdp::write_pixel(UINT32 curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	if (object.m_misc_state.m_fb_size == 2) // 16-bit framebuffer
	{
		const UINT32 fb = (object.m_misc_state.m_fb_address >> 1) + curpixel;

		UINT16 finalcolor;
		if (object.m_other_modes.color_on_cvg && !userdata->m_pre_wrap)
		{
			finalcolor = RREADIDX16(fb) & 0xfffe;
		}
		else
		{
			color.shr_imm(3);
			finalcolor = (color.get_r() << 11) | (color.get_g() << 6) | (color.get_b() << 1);
		}

		switch (object.m_other_modes.cvg_dest)
		{
			case 0:
				if (userdata->m_blend_enable)
				{
					UINT32 finalcvg = userdata->m_current_pix_cvg + userdata->m_current_mem_cvg;
					if (finalcvg & 8)
					{
						finalcvg = 7;
					}
					RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
					HWRITEADDR8(fb, finalcvg & 3);
				}
				else
				{
					const UINT32 finalcvg = (userdata->m_current_pix_cvg - 1) & 7;
					RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
					HWRITEADDR8(fb, finalcvg & 3);
				}
				break;
			case 1:
			{
				const UINT32 finalcvg = (userdata->m_current_pix_cvg + userdata->m_current_mem_cvg) & 7;
				RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
				HWRITEADDR8(fb, finalcvg & 3);
				break;
			}
			case 2:
				RWRITEIDX16(fb, finalcolor | 1);
				HWRITEADDR8(fb, 3);
				break;
			case 3:
				RWRITEIDX16(fb, finalcolor | (userdata->m_current_mem_cvg >> 2));
				HWRITEADDR8(fb, userdata->m_current_mem_cvg & 3);
				break;
		}
	}
	else // 32-bit framebuffer
	{
		const UINT32 fb = (object.m_misc_state.m_fb_address >> 2) + curpixel;

		UINT32 finalcolor;
		if (object.m_other_modes.color_on_cvg && !userdata->m_pre_wrap)
		{
			finalcolor = RREADIDX32(fb) & 0xffffff00;
		}
		else
		{
			finalcolor = (color.get_r() << 24) | (color.get_g() << 16) | (color.get_b() << 8);
		}

		switch (object.m_other_modes.cvg_dest)
		{
			case 0:
				if (userdata->m_blend_enable)
				{
					UINT32 finalcvg = userdata->m_current_pix_cvg + userdata->m_current_mem_cvg;
					if (finalcvg & 8)
					{
						finalcvg = 7;
					}

					RWRITEIDX32(fb, finalcolor | (finalcvg << 5));
				}
				else
				{
					RWRITEIDX32(fb, finalcolor | (((userdata->m_current_pix_cvg - 1) & 7) << 5));
				}
				break;
			case 1:
				RWRITEIDX32(fb, finalcolor | (((userdata->m_current_pix_cvg + userdata->m_current_mem_cvg) & 7) << 5));
				break;
			case 2:
				RWRITEIDX32(fb, finalcolor | 0xE0);
				break;
			case 3:
				RWRITEIDX32(fb, finalcolor | (userdata->m_current_mem_cvg << 5));
				break;
		}
	}
}

inline void n64_rdp::read_pixel(UINT32 curpixel, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	if (object.m_misc_state.m_fb_size == 2) // 16-bit framebuffer
	{
		const UINT16 fword = RREADIDX16((object.m_misc_state.m_fb_address >> 1) + curpixel);

		userdata->m_memory_color.set(0, GETHICOL(fword), GETMEDCOL(fword), GETLOWCOL(fword));
		if (object.m_other_modes.image_read_en)
		{
			UINT8 hbyte = HREADADDR8((object.m_misc_state.m_fb_address >> 1) + curpixel);
			userdata->m_memory_color.set_a(userdata->m_current_mem_cvg << 5);
			userdata->m_current_mem_cvg = ((fword & 1) << 2) | (hbyte & 3);
		}
		else
		{
			userdata->m_memory_color.set_a(0xff);
			userdata->m_current_mem_cvg = 7;
		}
	}
	else // 32-bit framebuffer
	{
		const UINT32 mem = RREADIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel);
		userdata->m_memory_color.set(0, (mem >> 24) & 0xff, (mem >> 16) & 0xff, (mem >> 8) & 0xff);
		if (object.m_other_modes.image_read_en)
		{
			userdata->m_memory_color.set_a(mem & 0xff);
			userdata->m_current_mem_cvg = (mem >> 5) & 7;
		}
		else
		{
			userdata->m_memory_color.set_a(0xff);
			userdata->m_current_mem_cvg = 7;
		}
	}
}

inline void n64_rdp::copy_pixel(UINT32 curpixel, color_t& color, const rdp_poly_state &object)
{
	const UINT32 current_pix_cvg = color.get_a() ? 7 : 0;
	const UINT8 r = color.get_r(); // Vectorize me
	const UINT8 g = color.get_g();
	const UINT8 b = color.get_b();
	if (object.m_misc_state.m_fb_size == 2) // 16-bit framebuffer
	{
		RWRITEIDX16((object.m_misc_state.m_fb_address >> 1) + curpixel, ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((current_pix_cvg >> 2) & 1));
		HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + curpixel, current_pix_cvg & 3);
	}
	else // 32-bit framebuffer
	{
		RWRITEIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel, (r << 24) | (g << 16) | (b << 8) | (current_pix_cvg << 5));
	}
}

inline void n64_rdp::fill_pixel(UINT32 curpixel, const rdp_poly_state &object)
{
	if (object.m_misc_state.m_fb_size == 2) // 16-bit framebuffer
	{
		UINT16 val;
		if (curpixel & 1)
		{
			val = object.m_fill_color & 0xffff;
		}
		else
		{
			val = (object.m_fill_color >> 16) & 0xffff;
		}
		RWRITEIDX16((object.m_misc_state.m_fb_address >> 1) + curpixel, val);
		HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + curpixel, ((val & 1) << 1) | (val & 1));
	}
	else // 32-bit framebuffer
	{
		RWRITEIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel, object.m_fill_color);
		HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + (curpixel << 1), (object.m_fill_color & 0x10000) ? 3 : 0);
		HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + (curpixel << 1) + 1, (object.m_fill_color & 0x1) ? 3 : 0);
	}
}

void n64_rdp::span_draw_1cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid)
{
	assert(object.m_misc_state.m_fb_size >= 2 && object.m_misc_state.m_fb_size < 4);

	const INT32 clipx1 = object.m_scissor.m_xh;
	const INT32 clipx2 = object.m_scissor.m_xl;
	const INT32 tilenum = object.tilenum;
	const bool flip = object.flip;

	span_param_t r; r.w = extent.param[SPAN_R].start;
	span_param_t g; g.w = extent.param[SPAN_G].start;
	span_param_t b; b.w = extent.param[SPAN_B].start;
	span_param_t a; a.w = extent.param[SPAN_A].start;
	span_param_t z; z.w = extent.param[SPAN_Z].start;
	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;
	span_param_t w; w.w = extent.param[SPAN_W].start;

	const UINT32 zb = object.m_misc_state.m_zb_address >> 1;
	const UINT32 zhb = object.m_misc_state.m_zb_address;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;

	m_tex_pipe.calculate_clamp_diffs(tilenum, userdata, object);

	const bool partialreject = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[0] == &userdata->m_pixel_color);
	const INT32 sel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color) ? 1 : 0;

	INT32 drinc, dginc, dbinc, dainc;
	INT32 dzinc, dzpix;
	INT32 dsinc, dtinc, dwinc;
	INT32 xinc;

	if (!flip)
	{
		drinc = -object.m_span_base.m_span_dr;
		dginc = -object.m_span_base.m_span_dg;
		dbinc = -object.m_span_base.m_span_db;
		dainc = -object.m_span_base.m_span_da;
		dzinc = -object.m_span_base.m_span_dz;
		dsinc = -object.m_span_base.m_span_ds;
		dtinc = -object.m_span_base.m_span_dt;
		dwinc = -object.m_span_base.m_span_dw;
		xinc = -1;
	}
	else
	{
		drinc = object.m_span_base.m_span_dr;
		dginc = object.m_span_base.m_span_dg;
		dbinc = object.m_span_base.m_span_db;
		dainc = object.m_span_base.m_span_da;
		dzinc = object.m_span_base.m_span_dz;
		dsinc = object.m_span_base.m_span_ds;
		dtinc = object.m_span_base.m_span_dt;
		dwinc = object.m_span_base.m_span_dw;
		xinc = 1;
	}

	const INT32 fb_index = object.m_misc_state.m_fb_width * scanline;

	const INT32 xstart = extent.startx;
	const INT32 xend = userdata->m_unscissored_rx;
	const INT32 xend_scissored = extent.stopx;

	INT32 x = xend;

	const INT32 length = flip ? (xstart - xend) : (xend - xstart);

	if(object.m_other_modes.z_source_sel)
	{
		z.w = object.m_misc_state.m_primitive_z;
		dzpix = object.m_misc_state.m_primitive_dz;
		dzinc = 0;
	}
	else
	{
		dzpix = object.m_span_base.m_span_dzpix;
	}

	if (object.m_misc_state.m_fb_size < 2 || object.m_misc_state.m_fb_size > 4)
		fatalerror("unsupported m_fb_size %d\n", object.m_misc_state.m_fb_size);

	const INT32 blend_index = (object.m_other_modes.alpha_cvg_select ? 2 : 0) | ((object.m_other_modes.rgb_dither_sel < 3) ? 1 : 0);
	const INT32 cycle0 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp0 & 1);

	INT32 sss = 0;
	INT32 sst = 0;

	if (object.m_other_modes.persp_tex_en)
	{
		tc_div(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		tc_div_no_perspective(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (INT32 j = 0; j <= length; j++)
	{
		INT32 sr = r.w >> 14;
		INT32 sg = g.w >> 14;
		INT32 sb = b.w >> 14;
		INT32 sa = a.w >> 14;
		INT32 sz = (z.w >> 10) & 0x3fffff;
		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			UINT8 offx, offy;
			lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			m_tex_pipe.lod_1cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, userdata, object);

			rgbaz_correct_triangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			rgbaz_clip(sr, sg, sb, sa, &sz, userdata);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tilenum, 0, userdata, object);
			UINT32 t0a = userdata->m_texel0_color.get_a();
			userdata->m_texel0_alpha.set(t0a, t0a, t0a, t0a);

			const UINT8 noise = rand() << 3; // Not accurate
			userdata->m_noise_color.set(0, noise, noise, noise);

			rgbaint_t rgbsub_a(*userdata->m_color_inputs.combiner_rgbsub_a[1]);
			rgbaint_t rgbsub_b(*userdata->m_color_inputs.combiner_rgbsub_b[1]);
			rgbaint_t rgbmul(*userdata->m_color_inputs.combiner_rgbmul[1]);
			rgbaint_t rgbadd(*userdata->m_color_inputs.combiner_rgbadd[1]);

			rgbsub_a.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_a[1]);
			rgbsub_b.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_b[1]);
			rgbmul.merge_alpha(*userdata->m_color_inputs.combiner_alphamul[1]);
			rgbadd.merge_alpha(*userdata->m_color_inputs.combiner_alphaadd[1]);

			rgbsub_a.sign_extend(0x180, 0xfffffe00);
			rgbsub_b.sign_extend(0x180, 0xfffffe00);
			rgbadd.sign_extend(0x180, 0xfffffe00);

			rgbadd.shl_imm(8);
			rgbsub_a.sub(rgbsub_b);
			rgbsub_a.mul(rgbmul);
			rgbsub_a.add(rgbadd);
			rgbsub_a.add_imm(0x0080);
			rgbsub_a.sra_imm(8);
			rgbsub_a.clamp_and_clear(0xfffffe00);

			userdata->m_pixel_color = rgbsub_a;

			//Alpha coverage combiner
			userdata->m_pixel_color.set_a(get_alpha_cvg(userdata->m_pixel_color.get_a(), userdata, object));

			const UINT32 curpixel = fb_index + x;
			const UINT32 zbcur = zb + curpixel;
			const UINT32 zhbcur = zhb + curpixel;

			read_pixel(curpixel, userdata, object);

			if(z_compare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				INT32 cdith = 0;
				INT32 adith = 0;
				get_dither_values(scanline, j, &cdith, &adith, object);

				color_t blended_pixel;
				bool rendered = ((&m_blender)->*(m_blender.blend1[(userdata->m_blend_enable << 2) | blend_index]))(blended_pixel, cdith, adith, partialreject, sel0, userdata, object);

				if (rendered)
				{
					write_pixel(curpixel, blended_pixel, userdata, object);
					if (object.m_other_modes.z_update_en)
					{
						z_store(object, zbcur, zhbcur, sz, userdata->m_dzpix_enc);
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

void n64_rdp::span_draw_2cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid)
{
	assert(object.m_misc_state.m_fb_size >= 2 && object.m_misc_state.m_fb_size < 4);

	const INT32 clipx1 = object.m_scissor.m_xh;
	const INT32 clipx2 = object.m_scissor.m_xl;
	const INT32 tilenum = object.tilenum;
	const bool flip = object.flip;

	span_param_t r; r.w = extent.param[SPAN_R].start;
	span_param_t g; g.w = extent.param[SPAN_G].start;
	span_param_t b; b.w = extent.param[SPAN_B].start;
	span_param_t a; a.w = extent.param[SPAN_A].start;
	span_param_t z; z.w = extent.param[SPAN_Z].start;
	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;
	span_param_t w; w.w = extent.param[SPAN_W].start;

	const UINT32 zb = object.m_misc_state.m_zb_address >> 1;
	const UINT32 zhb = object.m_misc_state.m_zb_address;

	INT32 tile2 = (tilenum + 1) & 7;
	INT32 tile1 = tilenum;
	const UINT32 prim_tile = tilenum;

	INT32 newtile1 = tile1;
	INT32 news = 0;
	INT32 newt = 0;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;

	m_tex_pipe.calculate_clamp_diffs(tile1, userdata, object);

	bool partialreject = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[1] == &userdata->m_pixel_color);
	INT32 sel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color) ? 1 : 0;
	INT32 sel1 = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_memory_color) ? 1 : 0;

	INT32 drinc, dginc, dbinc, dainc;
	INT32 dzinc, dzpix;
	INT32 dsinc, dtinc, dwinc;
	INT32 xinc;

	if (!flip)
	{
		drinc = -object.m_span_base.m_span_dr;
		dginc = -object.m_span_base.m_span_dg;
		dbinc = -object.m_span_base.m_span_db;
		dainc = -object.m_span_base.m_span_da;
		dzinc = -object.m_span_base.m_span_dz;
		dsinc = -object.m_span_base.m_span_ds;
		dtinc = -object.m_span_base.m_span_dt;
		dwinc = -object.m_span_base.m_span_dw;
		xinc = -1;
	}
	else
	{
		drinc = object.m_span_base.m_span_dr;
		dginc = object.m_span_base.m_span_dg;
		dbinc = object.m_span_base.m_span_db;
		dainc = object.m_span_base.m_span_da;
		dzinc = object.m_span_base.m_span_dz;
		dsinc = object.m_span_base.m_span_ds;
		dtinc = object.m_span_base.m_span_dt;
		dwinc = object.m_span_base.m_span_dw;
		xinc = 1;
	}

	const INT32 fb_index = object.m_misc_state.m_fb_width * scanline;

	INT32 cdith = 0;
	INT32 adith = 0;

	const INT32 xstart = extent.startx;
	const INT32 xend = userdata->m_unscissored_rx;
	const INT32 xend_scissored = extent.stopx;

	INT32 x = xend;

	const INT32 length = flip ? (xstart - xend) : (xend - xstart);

	if(object.m_other_modes.z_source_sel)
	{
		z.w = object.m_misc_state.m_primitive_z;
		dzpix = object.m_misc_state.m_primitive_dz;
		dzinc = 0;
	}
	else
	{
		dzpix = object.m_span_base.m_span_dzpix;
	}

	if (object.m_misc_state.m_fb_size < 2 || object.m_misc_state.m_fb_size > 4)
		fatalerror("unsupported m_fb_size %d\n", object.m_misc_state.m_fb_size);

	const INT32 blend_index = (object.m_other_modes.alpha_cvg_select ? 2 : 0) | ((object.m_other_modes.rgb_dither_sel < 3) ? 1 : 0);
	const INT32 cycle0 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp0 & 1);
	const INT32 cycle1 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp1 & 1);

	INT32 sss = 0;
	INT32 sst = 0;

	if (object.m_other_modes.persp_tex_en)
	{
		tc_div(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		tc_div_no_perspective(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (INT32 j = 0; j <= length; j++)
	{
		INT32 sr = r.w >> 14;
		INT32 sg = g.w >> 14;
		INT32 sb = b.w >> 14;
		INT32 sa = a.w >> 14;
		INT32 sz = (z.w >> 10) & 0x3fffff;
		color_t c1;
		color_t c2;

		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			const UINT32 compidx = m_compressed_cvmasks[userdata->m_cvg[x]];
			userdata->m_current_pix_cvg = cvarray[compidx].cvg;
			userdata->m_current_cvg_bit = cvarray[compidx].cvbit;
			const UINT8 offx = cvarray[compidx].xoff;
			const UINT8 offy = cvarray[compidx].yoff;
			//lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			m_tex_pipe.lod_2cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2, userdata, object);

			news = userdata->m_precomp_s;
			newt = userdata->m_precomp_t;
			m_tex_pipe.lod_2cycle_limited(&news, &newt, s.w + dsinc, t.w + dtinc, w.w + dwinc, dsinc, dtinc, dwinc, prim_tile, &newtile1, object);

			rgbaz_correct_triangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			rgbaz_clip(sr, sg, sb, sa, &sz, userdata);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tile1, 0, userdata, object);
			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle1]))(&userdata->m_texel1_color, &userdata->m_texel0_color, sss, sst, tile2, 1, userdata, object);
			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle1]))(&userdata->m_next_texel_color, &userdata->m_next_texel_color, sss, sst, tile2, 1, userdata, object);

			UINT32 t0a = userdata->m_texel0_color.get_a();
			UINT32 t1a = userdata->m_texel1_color.get_a();
			UINT32 tna = userdata->m_next_texel_color.get_a();
			userdata->m_texel0_alpha.set(t0a, t0a, t0a, t0a);
			userdata->m_texel1_alpha.set(t1a, t1a, t1a, t1a);
			userdata->m_next_texel_alpha.set(tna, tna, tna, tna);

			const UINT8 noise = rand() << 3; // Not accurate
			userdata->m_noise_color.set(0, noise, noise, noise);

			rgbaint_t rgbsub_a(*userdata->m_color_inputs.combiner_rgbsub_a[0]);
			rgbaint_t rgbsub_b(*userdata->m_color_inputs.combiner_rgbsub_b[0]);
			rgbaint_t rgbmul(*userdata->m_color_inputs.combiner_rgbmul[0]);
			rgbaint_t rgbadd(*userdata->m_color_inputs.combiner_rgbadd[0]);

			rgbsub_a.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_a[0]);
			rgbsub_b.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_b[0]);
			rgbmul.merge_alpha(*userdata->m_color_inputs.combiner_alphamul[0]);
			rgbadd.merge_alpha(*userdata->m_color_inputs.combiner_alphaadd[0]);

			rgbsub_a.sign_extend(0x180, 0xfffffe00);
			rgbsub_b.sign_extend(0x180, 0xfffffe00);
			rgbadd.sign_extend(0x180, 0xfffffe00);

			rgbadd.shl_imm(8);
			rgbsub_a.sub(rgbsub_b);
			rgbsub_a.mul(rgbmul);

			rgbsub_a.add(rgbadd);
			rgbsub_a.add_imm(0x0080);
			rgbsub_a.sra_imm(8);
			rgbsub_a.clamp_and_clear(0xfffffe00);

			userdata->m_combined_color.set(rgbsub_a);
			userdata->m_texel0_color.set(userdata->m_texel1_color);
			userdata->m_texel1_color.set(userdata->m_next_texel_color);

			UINT32 ca = userdata->m_combined_color.get_a();
			userdata->m_combined_alpha.set(ca, ca, ca, ca);
			userdata->m_texel0_alpha.set(userdata->m_texel1_alpha);
			userdata->m_texel1_alpha.set(userdata->m_next_texel_alpha);

			rgbsub_a.set(*userdata->m_color_inputs.combiner_rgbsub_a[1]);
			rgbsub_b.set(*userdata->m_color_inputs.combiner_rgbsub_b[1]);
			rgbmul.set(*userdata->m_color_inputs.combiner_rgbmul[1]);
			rgbadd.set(*userdata->m_color_inputs.combiner_rgbadd[1]);

			rgbsub_a.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_a[1]);
			rgbsub_b.merge_alpha(*userdata->m_color_inputs.combiner_alphasub_b[1]);
			rgbmul.merge_alpha(*userdata->m_color_inputs.combiner_alphamul[1]);
			rgbadd.merge_alpha(*userdata->m_color_inputs.combiner_alphaadd[1]);

			rgbsub_a.sign_extend(0x180, 0xfffffe00);
			rgbsub_b.sign_extend(0x180, 0xfffffe00);
			rgbadd.sign_extend(0x180, 0xfffffe00);

			rgbadd.shl_imm(8);
			rgbsub_a.sub(rgbsub_b);
			rgbsub_a.mul(rgbmul);
			rgbsub_a.add(rgbadd);
			rgbsub_a.add_imm(0x0080);
			rgbsub_a.sra_imm(8);
			rgbsub_a.clamp_and_clear(0xfffffe00);

			userdata->m_pixel_color.set(rgbsub_a);

			//Alpha coverage combiner
			userdata->m_pixel_color.set_a(get_alpha_cvg(userdata->m_pixel_color.get_a(), userdata, object));

			const UINT32 curpixel = fb_index + x;
			const UINT32 zbcur = zb + curpixel;
			const UINT32 zhbcur = zhb + curpixel;

			read_pixel(curpixel, userdata, object);

			if(z_compare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				get_dither_values(scanline, j, &cdith, &adith, object);

				color_t blended_pixel;
				bool rendered = ((&m_blender)->*(m_blender.blend2[(userdata->m_blend_enable << 2) | blend_index]))(blended_pixel, cdith, adith, partialreject, sel0, sel1, userdata, object);

				if (rendered)
				{
					write_pixel(curpixel, blended_pixel, userdata, object);
					if (object.m_other_modes.z_update_en)
					{
						z_store(object, zbcur, zhbcur, sz, userdata->m_dzpix_enc);
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

void n64_rdp::span_draw_copy(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid)
{
	const INT32 clipx1 = object.m_scissor.m_xh;
	const INT32 clipx2 = object.m_scissor.m_xl;
	const INT32 tilenum = object.tilenum;
	const bool flip = object.flip;

	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;
	const INT32 xstart = extent.startx;
	const INT32 xend = userdata->m_unscissored_rx;
	const INT32 xend_scissored = extent.stopx;
	const INT32 xinc = flip ? 1 : -1;
	const INT32 length = flip ? (xstart - xend) : (xend - xstart);

	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;

	const INT32 ds = object.m_span_base.m_span_ds / 4;
	const INT32 dt = object.m_span_base.m_span_dt / 4;
	const INT32 dsinc = flip ? (ds) : -ds;
	const INT32 dtinc = flip ? (dt) : -dt;

	const INT32 fb_index = object.m_misc_state.m_fb_width * scanline;

	INT32 x = xend;

	for (INT32 j = 0; j <= length; j++)
	{
		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			INT32 sss = s.h.h;
			INT32 sst = t.h.h;
			m_tex_pipe.copy(&userdata->m_texel0_color, sss, sst, tilenum, object, userdata);

			UINT32 curpixel = fb_index + x;
			if ((userdata->m_texel0_color.get_a() != 0) || (!object.m_other_modes.alpha_compare_en))
			{
				copy_pixel(curpixel, userdata->m_texel0_color, object);
			}
		}

		s.w += dsinc;
		t.w += dtinc;
		x += xinc;
	}
}

void n64_rdp::span_draw_fill(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid)
{
	assert(object.m_misc_state.m_fb_size >= 2 && object.m_misc_state.m_fb_size < 4);

	const bool flip = object.flip;

	const INT32 clipx1 = object.m_scissor.m_xh;
	const INT32 clipx2 = object.m_scissor.m_xl;

	const INT32 xinc = flip ? 1 : -1;

	const INT32 fb_index = object.m_misc_state.m_fb_width * scanline;

	const INT32 xstart = extent.startx;
	const INT32 xend_scissored = extent.stopx;

	INT32 x = xend_scissored;

	const INT32 length = flip ? (xstart - xend_scissored) : (xend_scissored - xstart);

	for (INT32 j = 0; j <= length; j++)
	{
		if (x >= clipx1 && x < clipx2)
		{
			fill_pixel(fb_index + x, object);
		}

		x += xinc;
	}
}
