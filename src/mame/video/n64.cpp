// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor
    -------------------

    by Ryan Holtz
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


*******************************************************************************

STATUS:

Much behavior needs verification against real hardware.  Many edge cases must
be verified on real hardware as well.

TODO:

- Further re-work class structure to avoid dependencies

*******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/rdpblend.h"
#include "video/rdptpipe.h"
#include "screen.h"

#include <algorithm>

#define LOG_RDP_EXECUTION       0
#define DEBUG_RDP_PIXEL         0
#define DRAW_FRAME_COUNTER      0

#if DEBUG_RDP_PIXEL
static bool s_debug_drawing = false;
#endif

static FILE* rdp_exec;

uint32_t n64_rdp::s_special_9bit_clamptable[512];

bool n64_rdp::rdp_range_check(uint32_t addr)
{
	if(m_misc_state.m_fb_size == 0) return false;

	int32_t fbcount = ((m_misc_state.m_fb_width * m_scissor.m_yl) << (m_misc_state.m_fb_size - 1)) * 3;
	int32_t fbaddr = m_misc_state.m_fb_address & 0x007fffff;
	if ((addr >= fbaddr) && (addr < (fbaddr + fbcount)))
	{
		return false;
	}

	int32_t zbcount = m_misc_state.m_fb_width * m_scissor.m_yl * 2;
	int32_t zbaddr = m_misc_state.m_zb_address & 0x007fffff;
	if ((addr >= zbaddr) && (addr < (zbaddr + zbcount)))
	{
		return false;
	}

	printf("Check failed: %08x vs. %08x-%08x, %08x-%08x (%d, %d)\n", addr, fbaddr, fbaddr + fbcount, zbaddr, zbaddr + zbcount, m_misc_state.m_fb_width, m_scissor.m_yl);
	fflush(stdout);
	return true;
}

/*****************************************************************************/

// The functions in this file should be moved into the parent Processor class.
#include "rdpfiltr.hxx"

int32_t n64_rdp::get_alpha_cvg(int32_t comb_alpha, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	int32_t temp = comb_alpha;
	int32_t temp2 = userdata->m_current_pix_cvg;
	int32_t temp3 = 0;

	if (object.m_other_modes.cvg_times_alpha)
	{
		temp3 = (temp * temp2) + 4;
		userdata->m_current_pix_cvg = (temp3 >> 8) & 0xf;
	}
	if (object.m_other_modes.alpha_cvg_select)
	{
		temp = (m_other_modes.cvg_times_alpha) ? (temp3 >> 3) : (temp2 << 5);
	}
	if (temp > 0xff)
	{
		temp = 0xff;
	}
	return temp;
}

/*****************************************************************************/

void n64_state::video_start()
{
	m_rdp = std::make_unique<n64_rdp>(*this, m_rdram, m_rsp_dmem);

	m_rdp->set_machine(machine());
	m_rdp->init_internal_state();
	m_rdp->set_n64_periphs(m_rcp_periphs);

	m_rdp->m_blender.set_machine(machine());
	m_rdp->m_blender.set_processor(m_rdp.get());

	m_rdp->m_tex_pipe.set_machine(machine());

	m_rdp->m_aux_buf = make_unique_clear<uint8_t[]>(EXTENT_AUX_COUNT);

	if (LOG_RDP_EXECUTION)
	{
		rdp_exec = fopen("rdp_execute.txt", "wt");
	}
}

uint32_t n64_state::screen_update_n64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//uint16_t* frame_buffer = (uint16_t*)&rdram[(m_rcp_periphs->vi_origin & 0xffffff) >> 2];
	//uint8_t* cvg_buffer = &m_rdp.m_hidden_bits[((m_rcp_periphs->vi_origin & 0xffffff) >> 2) >> 1];
	//int32_t vibuffering = ((m_rcp_periphs->vi_control & 2) && fsaa && divot);

	//vibuffering = 0; // Disabled for now

	/*
	if (vibuffering && ((m_rcp_periphs->vi_control & 3) == 2))
	{
	    if (frame_buffer)
	    {
	        for (j=0; j < vres; j++)
	        {
	            for (i=0; i < hres; i++)
	            {
	                uint16_t pix;
	                pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
	                curpixel_cvg = ((pix & 1) << 2) | (cvg_buffer[pixels ^ BYTE_ADDR_XOR] & 3); // Reuse of this variable
	                if (curpixel_cvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2) && fsaa)
	                {
	                    newc = video_filter16(&frame_buffer[pixels ^ WORD_ADDR_XOR], &cvg_buffer[pixels ^ BYTE_ADDR_XOR], m_rcp_periphs->vi_width);
	                    ViBuffer[i][j] = newc;
	                }
	                else
	                {
	                    newc.i.r = ((pix >> 8) & 0xf8) | (pix >> 13);
	                    newc.i.g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
	                    newc.i.b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
	                    ViBuffer[i][j] = newc;
	                }
	                pixels++;
	            }
	            pixels += invisiblewidth;
	        }
	    }
	}
	*/

	m_rdp->mark_frame();

	if (m_rcp_periphs->vi_blank)
	{
		bitmap.fill(0, screen.visible_area());
		return 0;
	}

	m_rcp_periphs->video_update(bitmap);

	return 0;
}

WRITE_LINE_MEMBER(n64_state::screen_vblank_n64)
{
}

void n64_periphs::video_update(bitmap_rgb32 &bitmap)
{

	if (vi_control & 0x40) /* Interlace */
	{
		field ^= 1;
	}
	else
	{
		field = 0;
	}

	switch (vi_control & 0x3)
	{
		case PIXEL_SIZE_16BIT:
			video_update16(bitmap);
			break;

		case PIXEL_SIZE_32BIT:
			video_update32(bitmap);
			break;

		default:
			//fatalerror("Unsupported framebuffer depth: m_fb_size=%d\n", m_misc_state.m_fb_size);
			break;
	}
}

void n64_periphs::video_update16(bitmap_rgb32 &bitmap)
{
	//int32_t fsaa = (((n64->vi_control >> 8) & 3) < 2);
	//int32_t divot = (n64->vi_control >> 4) & 1;

	//uint32_t prev_cvg = 0;
	//uint32_t next_cvg = 0;
	//int32_t dither_filter = (n64->vi_control >> 16) & 1;
	//int32_t vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	uint16_t* frame_buffer = (uint16_t*)&m_rdram[(vi_origin & 0xffffff) >> 2];
	//uint32_t hb = ((n64->vi_origin & 0xffffff) >> 2) >> 1;
	//uint8_t* hidden_buffer = &m_hidden_bits[hb];

	int32_t hend = vi_hstart & 0x3ff;
	int32_t hstart = (vi_hstart >> 16) & 0x3ff;
	int32_t hdiff = hend - hstart;
	float hcoeff = ((float)(vi_xscale & 0xfff) / (1 << 10));
	uint32_t hres = ((float)hdiff * hcoeff);

	int32_t vend = (vi_vstart & 0x3ff) >> 1;
	int32_t vstart = ((vi_vstart >> 16) & 0x3ff) >> 1;
	int32_t vdiff = vend - vstart;
	float vcoeff = ((float)(vi_yscale & 0xfff) / (1 << 10));
	uint32_t vres = ((float)vdiff * vcoeff);

	fflush(stdout);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return;
	}

	if (vres > bitmap.height()) // makes Perfect Dark boot w/o crashing
	{
		vres = bitmap.height();
	}

#if DRAW_FRAME_COUNTER
	static uint32_t frame_num = 0;
	static const uint8_t s_numbers[10][9] = {
		{ 0x00, 0x3c, 0x66, 0x6e, 0x7e, 0x76, 0x66, 0x3c, 0x00 },
		{ 0x00, 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00 },
		{ 0x00, 0x3c, 0x66, 0x06, 0x3c, 0x60, 0x60, 0x7e, 0x00 },
		{ 0x00, 0x3c, 0x66, 0x06, 0x0c, 0x06, 0x66, 0x3c, 0x00 },
		{ 0x00, 0x66, 0x66, 0x66, 0x7e, 0x06, 0x06, 0x06, 0x00 },
		{ 0x00, 0x7e, 0x60, 0x60, 0x7c, 0x06, 0x66, 0x3c, 0x00 },
		{ 0x00, 0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00 },
		{ 0x00, 0x7e, 0x66, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00 },
		{ 0x00, 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00 },
		{ 0x00, 0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00 }
	};
#endif

	if (frame_buffer)
	{
#if DRAW_FRAME_COUNTER
		uint32_t digits[4] = { (frame_num / 1000) % 10, (frame_num / 100) % 10, (frame_num / 10) % 10, frame_num % 10 };

		for (int32_t d = 0; d < 4; d++)
		{
			for (int32_t y = 0; y < 9; y++)
			{
				const uint8_t *pixdata = s_numbers[digits[d]];
				for (int32_t x = 0; x < 8; x++)
				{
					frame_buffer[((y + 16) * vi_width + d * 8 + x + 16) ^ WORD_ADDR_XOR] = BIT(pixdata[y], 7 - x) ? 0x0000 : 0xffff;
				}
			}
		}
#if DEBUG_RDP_PIXEL
		s_debug_drawing = (frame_num == 1392);
#endif
		frame_num++;
#endif

		const uint32_t aa_control = (vi_control >> 8) & 3;
		float v0 = 0.0f;
		if (aa_control < 3) // Resample pixels
		{
			for (int32_t j = 0; j < vdiff; j++, v0 += vcoeff)
			{
				uint32_t *const d = &bitmap.pix(j);

				float u0 = (float)0.0f;

				int iv0 = (int)v0;
				int pix_v0_line = iv0 * vi_width;

				int iv1 = (iv0 >= (vres - 1) ? iv0 : (iv0 + 1));
				int pix_v1_line = iv1 * vi_width;

				for (int32_t i = 0; i < hdiff; i++)
				{
					int iu0 = (int)u0;
					int iu1 = (iu0 >= (hres - 1) ? iu0 : (iu0 + 1));
					uint16_t pix00 = frame_buffer[(pix_v0_line + iu0) ^ WORD_ADDR_XOR];
					uint16_t pix10 = frame_buffer[(pix_v0_line + iu1) ^ WORD_ADDR_XOR];
					uint16_t pix01 = frame_buffer[(pix_v1_line + iu0) ^ WORD_ADDR_XOR];
					uint16_t pix11 = frame_buffer[(pix_v1_line + iu1) ^ WORD_ADDR_XOR];

					const uint8_t r00 = ((pix00 >> 8) & 0xf8) | (pix00 >> 13);
					const uint8_t g00 = ((pix00 >> 3) & 0xf8) | ((pix00 >>  8) & 0x07);
					const uint8_t b00 = ((pix00 << 2) & 0xf8) | ((pix00 >>  3) & 0x07);

					const uint8_t r10 = ((pix10 >> 8) & 0xf8) | (pix10 >> 13);
					const uint8_t g10 = ((pix10 >> 3) & 0xf8) | ((pix10 >>  8) & 0x07);
					const uint8_t b10 = ((pix10 << 2) & 0xf8) | ((pix10 >>  3) & 0x07);

					const uint8_t r01 = ((pix01 >> 8) & 0xf8) | (pix01 >> 13);
					const uint8_t g01 = ((pix01 >> 3) & 0xf8) | ((pix01 >>  8) & 0x07);
					const uint8_t b01 = ((pix01 << 2) & 0xf8) | ((pix01 >>  3) & 0x07);

					const uint8_t r11 = ((pix11 >> 8) & 0xf8) | (pix11 >> 13);
					const uint8_t g11 = ((pix11 >> 3) & 0xf8) | ((pix11 >>  8) & 0x07);
					const uint8_t b11 = ((pix11 << 2) & 0xf8) | ((pix11 >>  3) & 0x07);

					const float ut = u0 - (int)u0;
					const float vt = v0 - (int)v0;

					float ur0 = (1.0f - ut) * r00 + ut * r10;
					float ug0 = (1.0f - ut) * g00 + ut * g10;
					float ub0 = (1.0f - ut) * b00 + ut * b10;

					float ur1 = (1.0f - ut) * r01 + ut * r11;
					float ug1 = (1.0f - ut) * g01 + ut * g11;
					float ub1 = (1.0f - ut) * b01 + ut * b11;

					float r = (1.0f - vt) * ur0 + vt * ur1;
					float g = (1.0f - vt) * ug0 + vt * ug1;
					float b = (1.0f - vt) * ub0 + vt * ub1;

					uint8_t r8 = std::clamp((uint8_t)r, (uint8_t)0, (uint8_t)255);
					uint8_t g8 = std::clamp((uint8_t)g, (uint8_t)0, (uint8_t)255);
					uint8_t b8 = std::clamp((uint8_t)b, (uint8_t)0, (uint8_t)255);

					d[iu0] = (r8 << 16) | (g8 << 8) | b8;

					u0 += hcoeff;
				}
			}
		}
		else // Replicate pixels
		{
			for (int32_t j = 0; j < vdiff; j++, v0 += vcoeff)
			{
				uint32_t *const d = &bitmap.pix(j);

				int iv0 = (int)v0;
				int pix_v0_line = iv0 * vi_width;

				for (int32_t i = 0; i < hdiff; i++)
				{
					int u0 = (int)(i * hcoeff);
					uint16_t pix = frame_buffer[(pix_v0_line + u0) ^ WORD_ADDR_XOR];

					const uint8_t r = ((pix >> 8) & 0xf8) | (pix >> 13);
					const uint8_t g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
					const uint8_t b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
					d[u0] = (r << 16) | (g << 8) | b;
				}
			}
		}
	}
}

void n64_periphs::video_update32(bitmap_rgb32 &bitmap)
{
	//int32_t gamma = (vi_control >> 3) & 1;
	//int32_t gamma_dither = (vi_control >> 2) & 1;
	//int32_t vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	uint32_t* frame_buffer32 = (uint32_t*)&m_rdram[(vi_origin & 0xffffff) >> 2];

	int32_t hend = vi_hstart & 0x3ff;
	int32_t hstart = (vi_hstart >> 16) & 0x3ff;
	int32_t hdiff = hend - hstart;
	const float hcoeff = ((float)(vi_xscale & 0xfff) / (1 << 10));
	uint32_t hres = ((float)hdiff * hcoeff);

	int32_t vend = (vi_vstart & 0x3ff) >> 1;
	int32_t vstart = ((vi_vstart >> 16) & 0x3ff) >> 1;
	int32_t vdiff = vend - vstart;
	const float vcoeff = ((float)(vi_yscale & 0xfff) / (1 << 10));
	const uint32_t vres = ((float)vdiff * vcoeff);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return;
	}

	//printf("hd,vd: %d,%d hc,vc: %f,%f hs,he: %d,%d vs,ve: %d,%d hr,vr: %d, %d viw: %d\n", hdiff, vdiff, hcoeff, vcoeff, hstart, hend, vstart, vend, hres, vres, vi_width);

	if (frame_buffer32)
	{
		const uint32_t aa_control = (vi_control >> 8) & 3;
		float v0 = 0.0f;
		if (aa_control < 3) // Resample pixels
		{
			for (int32_t j = 0; j < vres; j++, v0 += 1.0f)
			{
				uint32_t *const d = &bitmap.pix(j);

				float u0 = 0.0f;

				int iv0 = (int)v0;
				int pix_v0_line = iv0 * vi_width;

				int iv1 = (iv0 >= (vres - 1) ? iv0 : (iv0 + 1));
				int pix_v1_line = iv1 * vi_width;

				for (int32_t i = 0; i < hdiff; i++)
				{
					int iu0 = (int)u0;
					int iu1 = (iu0 >= (hres - 1) ? iu0 : (iu0 + 1));
					uint32_t pix00 = frame_buffer32[pix_v0_line + iu0];
					uint32_t pix10 = frame_buffer32[pix_v0_line + iu1];
					uint32_t pix01 = frame_buffer32[pix_v1_line + iu0];
					uint32_t pix11 = frame_buffer32[pix_v1_line + iu1];

					const uint8_t r00 = (uint8_t)(pix00 >> 24);
					const uint8_t g00 = (uint8_t)(pix00 >> 16);
					const uint8_t b00 = (uint8_t)(pix00 >> 8);

					const uint8_t r10 = (uint8_t)(pix01 >> 24);
					const uint8_t g10 = (uint8_t)(pix01 >> 16);
					const uint8_t b10 = (uint8_t)(pix01 >> 8);

					const uint8_t r01 = (uint8_t)(pix10 >> 24);
					const uint8_t g01 = (uint8_t)(pix10 >> 16);
					const uint8_t b01 = (uint8_t)(pix10 >> 8);

					const uint8_t r11 = (uint8_t)(pix11 >> 24);
					const uint8_t g11 = (uint8_t)(pix11 >> 16);
					const uint8_t b11 = (uint8_t)(pix11 >> 8);

					const float ut = u0 - (int)u0;
					const float vt = v0 - (int)v0;

					float ur0 = (1.0f - ut) * r00 + ut * r10;
					float ug0 = (1.0f - ut) * g00 + ut * g10;
					float ub0 = (1.0f - ut) * b00 + ut * b10;

					float ur1 = (1.0f - ut) * r01 + ut * r11;
					float ug1 = (1.0f - ut) * g01 + ut * g11;
					float ub1 = (1.0f - ut) * b01 + ut * b11;

					float r = (1.0f - vt) * ur0 + vt * ur1;
					float g = (1.0f - vt) * ug0 + vt * ug1;
					float b = (1.0f - vt) * ub0 + vt * ub1;

					uint8_t r8 = std::clamp((uint8_t)r, (uint8_t)0, (uint8_t)255);
					uint8_t g8 = std::clamp((uint8_t)g, (uint8_t)0, (uint8_t)255);
					uint8_t b8 = std::clamp((uint8_t)b, (uint8_t)0, (uint8_t)255);

					d[iu0] = (r8 << 16) | (g8 << 8) | b8;

					u0 += hcoeff;
				}
			}
		}
		else // Replicate pixels
		{
			for (int32_t j = 0; j < vdiff; j++, v0 += vcoeff)
			{
				uint32_t *const d = &bitmap.pix(j);

				int iv0 = (int)v0;
				int pix_v0_line = iv0 * vi_width;

				for (int32_t i = 0; i < hdiff; i++)
				{
					int u0 = (int)(i * hcoeff);
					d[u0] = (frame_buffer32[pix_v0_line + u0] >> 8);
				}
			}
		}
	}
}

/*****************************************************************************/

void n64_rdp::tc_div_no_perspective(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst)
{
	*sss = (SIGN16(ss)) & 0x1ffff;
	*sst = (SIGN16(st)) & 0x1ffff;
}

void n64_rdp::tc_div(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst)
{
	int32_t w_carry = 0;
	if ((sw & 0x8000) || !(sw & 0x7fff))
	{
		w_carry = 1;
	}

	sw &= 0x7fff;

	int32_t shift;
	for (shift = 1; shift <= 14 && !((sw << shift) & 0x8000); shift++);
	shift -= 1;

	int32_t normout = (sw << shift) & 0x3fff;
	int32_t wnorm = (normout & 0xff) << 2;
	normout >>= 8;

	int32_t temppoint = m_norm_point_rom[normout];
	int32_t tempslope = m_norm_slope_rom[normout];

	int32_t tlu_rcp = ((-(tempslope * wnorm)) >> 10) + temppoint;

	int32_t sprod = SIGN16(ss) * tlu_rcp;
	int32_t tprod = SIGN16(st) * tlu_rcp;
	int32_t tempmask = ((1 << (shift + 1)) - 1) << (29 - shift);
	int32_t shift_value = 13 - shift;

	int32_t outofbounds_s = sprod & tempmask;
	int32_t outofbounds_t = tprod & tempmask;
	if (shift == 0xe)
	{
		*sss = sprod << 1;
		*sst = tprod << 1;
	}
	else
	{
		*sss = sprod = (sprod >> shift_value);
		*sst = tprod = (tprod >> shift_value);
	}
	//compute clamp flags
	int32_t under_s = 0;
	int32_t under_t = 0;
	int32_t over_s = 0;
	int32_t over_t = 0;

	if (outofbounds_s != tempmask && outofbounds_s != 0)
	{
		if (sprod & (1 << 29))
		{
			under_s = 1;
		}
		else
		{
			over_s = 1;
		}
	}

	if (outofbounds_t != tempmask && outofbounds_t != 0)
	{
		if (tprod & (1 << 29))
		{
			under_t = 1;
		}
		else
		{
			over_t = 1;
		}
	}

	over_s |= w_carry;
	over_t |= w_carry;

	*sss = (*sss & 0x1ffff) | (over_s << 18) | (under_s << 17);
	*sst = (*sst & 0x1ffff) | (over_t << 18) | (under_t << 17);
}

int32_t n64_rdp::color_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d)
{
	a = KURT_AKELEY_SIGN9(a);
	b = KURT_AKELEY_SIGN9(b);
	c = SIGN9(c);
	d = KURT_AKELEY_SIGN9(d);
	a = (((a - b) * c) + (d << 8) + 0x80);
	a = SIGN17(a) >> 8;
	a = s_special_9bit_clamptable[a & 0x1ff];
	return a;
}

int32_t n64_rdp::alpha_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d)
{
	a = KURT_AKELEY_SIGN9(a);
	b = KURT_AKELEY_SIGN9(b);
	c = SIGN9(c);
	d = KURT_AKELEY_SIGN9(d);
	a = (((a - b) * c) + (d << 8) + 0x80) >> 8;
	a = SIGN9(a);
	a = s_special_9bit_clamptable[a & 0x1ff];
	return a;
}

void n64_rdp::set_suba_input_rgb(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0xf)
	{
		case 0:     *input = &userdata->m_combined_color; break;
		case 1:     *input = &userdata->m_texel0_color; break;
		case 2:     *input = &userdata->m_texel1_color; break;
		case 3:     *input = &userdata->m_prim_color; break;
		case 4:     *input = &userdata->m_shade_color; break;
		case 5:     *input = &userdata->m_env_color; break;
		case 6:     *input = &m_one; break;
		case 7:     *input = &userdata->m_noise_color; break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
					*input = &m_zero; break;
		}
	}
}

void n64_rdp::set_subb_input_rgb(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0xf)
	{
		case 0:     *input = &userdata->m_combined_color; break;
		case 1:     *input = &userdata->m_texel0_color; break;
		case 2:     *input = &userdata->m_texel1_color; break;
		case 3:     *input = &userdata->m_prim_color; break;
		case 4:     *input = &userdata->m_shade_color; break;
		case 5:     *input = &userdata->m_env_color; break;
		case 6:     fatalerror("SET_SUBB_RGB_INPUT: key_center\n");
		case 7:     *input = &userdata->m_k4; break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
					*input = &m_zero; break;
		}
	}
}

void n64_rdp::set_mul_input_rgb(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0x1f)
	{
		case 0:     *input = &userdata->m_combined_color; break;
		case 1:     *input = &userdata->m_texel0_color; break;
		case 2:     *input = &userdata->m_texel1_color; break;
		case 3:     *input = &userdata->m_prim_color; break;
		case 4:     *input = &userdata->m_shade_color; break;
		case 5:     *input = &userdata->m_env_color; break;
		case 6:     *input = &userdata->m_key_scale; break;
		case 7:     *input = &userdata->m_combined_alpha; break;
		case 8:     *input = &userdata->m_texel0_alpha; break;
		case 9:     *input = &userdata->m_texel1_alpha; break;
		case 10:    *input = &userdata->m_prim_alpha; break;
		case 11:    *input = &userdata->m_shade_alpha; break;
		case 12:    *input = &userdata->m_env_alpha; break;
		case 13:    *input = &userdata->m_lod_fraction; break;
		case 14:    *input = &userdata->m_prim_lod_fraction; break;
		case 15:    *input = &userdata->m_k5; break;
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
		{
					*input = &m_zero; break;
		}
	}
}

void n64_rdp::set_add_input_rgb(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0x7)
	{
		case 0:     *input = &userdata->m_combined_color; break;
		case 1:     *input = &userdata->m_texel0_color; break;
		case 2:     *input = &userdata->m_texel1_color; break;
		case 3:     *input = &userdata->m_prim_color; break;
		case 4:     *input = &userdata->m_shade_color; break;
		case 5:     *input = &userdata->m_env_color; break;
		case 6:     *input = &m_one; break;
		case 7:     *input = &m_zero; break;
	}
}

void n64_rdp::set_sub_input_alpha(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0x7)
	{
		case 0:     *input = &userdata->m_combined_alpha; break;
		case 1:     *input = &userdata->m_texel0_alpha; break;
		case 2:     *input = &userdata->m_texel1_alpha; break;
		case 3:     *input = &userdata->m_prim_alpha; break;
		case 4:     *input = &userdata->m_shade_alpha; break;
		case 5:     *input = &userdata->m_env_alpha; break;
		case 6:     *input = &m_one; break;
		case 7:     *input = &m_zero; break;
	}
}

void n64_rdp::set_mul_input_alpha(color_t** input, int32_t code, rdp_span_aux* userdata)
{
	switch (code & 0x7)
	{
		case 0:     *input = &userdata->m_lod_fraction; break;
		case 1:     *input = &userdata->m_texel0_alpha; break;
		case 2:     *input = &userdata->m_texel1_alpha; break;
		case 3:     *input = &userdata->m_prim_alpha; break;
		case 4:     *input = &userdata->m_shade_alpha; break;
		case 5:     *input = &userdata->m_env_alpha; break;
		case 6:     *input = &userdata->m_prim_lod_fraction; break;
		case 7:     *input = &m_zero; break;
	}
}

void n64_rdp::set_blender_input(int32_t cycle, int32_t which, color_t** input_rgb, color_t** input_a, int32_t a, int32_t b, rdp_span_aux* userdata)
{
	switch (a & 0x3)
	{
		case 0:
			*input_rgb = cycle == 0 ? &userdata->m_pixel_color : &userdata->m_blended_pixel_color;
			break;

		case 1:
			*input_rgb = &userdata->m_memory_color;
			break;

		case 2:
			*input_rgb = &userdata->m_blend_color;
			break;

		case 3:
			*input_rgb = &userdata->m_fog_color;
			break;
	}

	if (which == 0)
	{
		switch (b & 0x3)
		{
			case 0:     *input_a = &userdata->m_pixel_color; break;
			case 1:     *input_a = &userdata->m_fog_color; break;
			case 2:     *input_a = &userdata->m_shade_color; break;
			case 3:     *input_a = &m_zero; break;
		}
	}
	else
	{
		switch (b & 0x3)
		{
			case 0:     *input_a = &userdata->m_inv_pixel_color; break;
			case 1:     *input_a = &userdata->m_memory_color; break;
			case 2:     *input_a = &m_one; break;
			case 3:     *input_a = &m_zero; break;
		}
	}
}

uint8_t const n64_rdp::s_bayer_matrix[16] =
{ /* Bayer matrix */
	0, 4, 1, 5,
	6, 2, 7, 3,
	1, 5, 0, 4,
	7, 3, 6, 2
};

uint8_t const n64_rdp::s_magic_matrix[16] =
{ /* Magic square matrix */
	0, 4, 3, 7,
	6, 2, 5, 1,
	1, 5, 2, 6,
	7, 3, 4, 0
};

z_decompress_entry_t const n64_rdp::m_z_dec_table[8] =
{
	{ 6, 0x00000 },
	{ 5, 0x20000 },
	{ 4, 0x30000 },
	{ 3, 0x38000 },
	{ 2, 0x3c000 },
	{ 1, 0x3e000 },
	{ 0, 0x3f000 },
	{ 0, 0x3f800 },
};

/*****************************************************************************/

void n64_rdp::z_build_com_table(void)
{
	uint16_t altmem = 0;
	for(int32_t z = 0; z < 0x40000; z++)
	{
		switch((z >> 11) & 0x7f)
		{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1a:
			case 0x1b:
			case 0x1c:
			case 0x1d:
			case 0x1e:
			case 0x1f:
			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
			case 0x28:
			case 0x29:
			case 0x2a:
			case 0x2b:
			case 0x2c:
			case 0x2d:
			case 0x2e:
			case 0x2f:
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
			case 0x3a:
			case 0x3b:
			case 0x3c:
			case 0x3d:
			case 0x3e:
			case 0x3f:
				altmem = (z >> 4) & 0x1ffc;
				break;
			case 0x40:
			case 0x41:
			case 0x42:
			case 0x43:
			case 0x44:
			case 0x45:
			case 0x46:
			case 0x47:
			case 0x48:
			case 0x49:
			case 0x4a:
			case 0x4b:
			case 0x4c:
			case 0x4d:
			case 0x4e:
			case 0x4f:
			case 0x50:
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
			case 0x58:
			case 0x59:
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
				altmem = ((z >> 3) & 0x1ffc) | 0x2000;
				break;
			case 0x60:
			case 0x61:
			case 0x62:
			case 0x63:
			case 0x64:
			case 0x65:
			case 0x66:
			case 0x67:
			case 0x68:
			case 0x69:
			case 0x6a:
			case 0x6b:
			case 0x6c:
			case 0x6d:
			case 0x6e:
			case 0x6f:
				altmem = ((z >> 2) & 0x1ffc) | 0x4000;
				break;
			case 0x70:
			case 0x71:
			case 0x72:
			case 0x73:
			case 0x74:
			case 0x75:
			case 0x76:
			case 0x77:
				altmem = ((z >> 1) & 0x1ffc) | 0x6000;
				break;
			case 0x78://uncompressed z = 0x3c000
			case 0x79:
			case 0x7a:
			case 0x7b:
				altmem = (z & 0x1ffc) | 0x8000;
				break;
			case 0x7c://uncompressed z = 0x3e000
			case 0x7d:
				altmem = ((z << 1) & 0x1ffc) | 0xa000;
				break;
			case 0x7e://uncompressed z = 0x3f000
				altmem = ((z << 2) & 0x1ffc) | 0xc000;
				break;
			case 0x7f://uncompressed z = 0x3f000
				altmem = ((z << 2) & 0x1ffc) | 0xe000;
				break;
		}

	m_z_com_table[z] = altmem;

	}
}

void n64_rdp::precalc_cvmask_derivatives(void)
{
	const uint8_t yarray[16] = {0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const uint8_t xarray[16] = {0, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	for (int32_t i = 0; i < 0x10000; i++)
	{
		m_compressed_cvmasks[i] = (i & 1) | ((i & 4) >> 1) | ((i & 0x20) >> 3) | ((i & 0x80) >> 4) |
		((i & 0x100) >> 4) | ((i & 0x400) >> 5) | ((i & 0x2000) >> 7) | ((i & 0x8000) >> 8);
	}

	for (int32_t i = 0; i < 0x100; i++)
	{
		uint16_t mask = decompress_cvmask_frombyte(i);
		cvarray[i].cvg = cvarray[i].cvbit = 0;
		cvarray[i].cvbit = (i >> 7) & 1;
		for (int32_t k = 0; k < 8; k++)
		{
			cvarray[i].cvg += ((i >> k) & 1);
		}

		uint16_t masky = 0;
		for (int32_t k = 0; k < 4; k++)
		{
			masky |= ((mask & (0xf000 >> (k << 2))) > 0) << k;
		}
		uint8_t offy = yarray[masky];

		uint16_t maskx = (mask & (0xf000 >> (offy << 2))) >> ((offy ^ 3) << 2);
		uint8_t offx = xarray[maskx];

		cvarray[i].xoff = offx;
		cvarray[i].yoff = offy;
	}
}

uint16_t n64_rdp::decompress_cvmask_frombyte(uint8_t x)
{
	uint16_t y = (x & 1) | ((x & 2) << 1) | ((x & 4) << 3) | ((x & 8) << 4) |
		((x & 0x10) << 4) | ((x & 0x20) << 5) | ((x & 0x40) << 7) | ((x & 0x80) << 8);
	return y;
}

void n64_rdp::lookup_cvmask_derivatives(uint32_t mask, uint8_t* offx, uint8_t* offy, rdp_span_aux* userdata)
{
	const uint32_t index = m_compressed_cvmasks[mask];
	userdata->m_current_pix_cvg = cvarray[index].cvg;
	userdata->m_current_cvg_bit = cvarray[index].cvbit;
	*offx = cvarray[index].xoff;
	*offy = cvarray[index].yoff;
}

void n64_rdp::z_store(const rdp_poly_state &object, uint32_t zcurpixel, uint32_t dzcurpixel, uint32_t z, uint32_t enc)
{
	uint16_t zval = m_z_com_table[z & 0x3ffff]|(enc >> 2);
	if(zcurpixel <= MEM16_LIMIT)
	{
		((uint16_t*)m_rdram)[zcurpixel ^ WORD_ADDR_XOR] = zval;
	}
	if(dzcurpixel <= MEM8_LIMIT)
	{
		m_hidden_bits[dzcurpixel ^ BYTE_ADDR_XOR] = enc & 3;
	}
}

int32_t n64_rdp::normalize_dzpix(int32_t sum)
{
	if (sum & 0xc000)
	{
		return 0x8000;
	}
	if (!(sum & 0xffff))
	{
		return 1;
	}
	for(int32_t count = 0x2000; count > 0; count >>= 1)
	{
		if (sum & count)
		{
			return(count << 1);
		}
	}
	return 0;
}

uint32_t n64_rdp::z_decompress(uint32_t zcurpixel)
{
	return m_z_complete_dec_table[(RREADIDX16(zcurpixel) >> 2) & 0x3fff];
}

uint32_t n64_rdp::dz_decompress(uint32_t zcurpixel, uint32_t dzcurpixel)
{
	const uint16_t zval = RREADIDX16(zcurpixel);
	const uint8_t dzval = (((dzcurpixel) <= 0x7fffff) ? (m_hidden_bits[(dzcurpixel) ^ BYTE_ADDR_XOR]) : 0);
	const uint32_t dz_compressed = ((zval & 3) << 2) | (dzval & 3);
	return (1 << dz_compressed);
}

uint32_t n64_rdp::dz_compress(uint32_t value)
{
	int32_t j = 0;
	for (; value > 1; j++, value >>= 1);
	return j;
}

void n64_rdp::get_dither_values(int32_t x, int32_t y, int32_t* cdith, int32_t* adith, const rdp_poly_state& object)
{
	const int32_t dithindex = ((y & 3) << 2) | (x & 3);
	switch((object.m_other_modes.rgb_dither_sel << 2) | object.m_other_modes.alpha_dither_sel)
	{
	case 0:
		*adith = *cdith = s_magic_matrix[dithindex];
		break;
	case 1:
		*cdith = s_magic_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 2:
		*cdith = s_magic_matrix[dithindex];
		*adith = machine().rand() & 7;
		break;
	case 3:
		*cdith = s_magic_matrix[dithindex];
		*adith = 0;
		break;
	case 4:
		*adith = *cdith = s_bayer_matrix[dithindex];
		break;
	case 5:
		*cdith = s_bayer_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 6:
		*cdith = s_bayer_matrix[dithindex];
		*adith = machine().rand() & 7;
		break;
	case 7:
		*cdith = s_bayer_matrix[dithindex];
		*adith = 0;
		break;
	case 8:
		*cdith = machine().rand() & 7;
		*adith = s_magic_matrix[dithindex];
		break;
	case 9:
		*cdith = machine().rand() & 7;
		*adith = (~s_magic_matrix[dithindex]) & 7;
		break;
	case 10:
		*cdith = machine().rand() & 7;
		*adith = (*cdith + 17) & 7;
		break;
	case 11:
		*cdith = machine().rand() & 7;
		*adith = 0;
		break;
	case 12:
		*cdith = 0;
		*adith = s_bayer_matrix[dithindex];
		break;
	case 13:
		*cdith = 0;
		*adith = (~s_bayer_matrix[dithindex]) & 7;
		break;
	case 14:
		*cdith = 0;
		*adith = machine().rand() & 7;
		break;
	case 15:
		*adith = *cdith = 0;
		break;
	}
}

int32_t CLAMP(int32_t in, int32_t min, int32_t max)
{
	if(in < min) return min;
	if(in > max) return max;
	return in;
}

bool n64_rdp::z_compare(uint32_t zcurpixel, uint32_t dzcurpixel, uint32_t sz, uint16_t dzpix, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	bool force_coplanar = false;
	sz &= 0x3ffff;

	uint32_t oz;
	uint32_t dzmem;
	uint32_t zval;
	int32_t rawdzmem;

	if (object.m_other_modes.z_compare_en)
	{
		oz = z_decompress(zcurpixel);
		dzmem = dz_decompress(zcurpixel, dzcurpixel);
		zval = RREADIDX16(zcurpixel);
		rawdzmem = ((zval & 3) << 2) | ((((dzcurpixel) <= 0x3fffff) ? (m_hidden_bits[(dzcurpixel) ^ BYTE_ADDR_XOR]) : 0) & 3);
	}
	else
	{
		oz = 0;
		dzmem = 1 << 0xf;
		zval = 0x3;
		rawdzmem = 0xf;
	}

	userdata->m_dzpix_enc = dz_compress(dzpix & 0xffff);
	userdata->m_shift_a = CLAMP(userdata->m_dzpix_enc - rawdzmem, 0, 4);
	userdata->m_shift_b = CLAMP(rawdzmem - userdata->m_dzpix_enc, 0, 4);

	int32_t precision_factor = (zval >> 13) & 0xf;
	if (precision_factor < 3)
	{
		int32_t dzmemmodifier = 16 >> precision_factor;
		if (dzmem == 0x8000)
		{
			force_coplanar = true;
		}
		dzmem <<= 1;
		if (dzmem <= dzmemmodifier)
		{
			dzmem = dzmemmodifier;
		}
		if (!dzmem)
		{
			dzmem = 0xffff;
		}
	}
	if (dzmem > 0x8000)
	{
		dzmem = 0xffff;
	}

	uint32_t dznew = (dzmem > dzpix) ? dzmem : (uint32_t)dzpix;
	uint32_t dznotshift = dznew;
	dznew <<= 3;

	bool farther = (sz + dznew) >= oz;
	bool infront = sz < oz;

	if (force_coplanar)
	{
		farther = true;
	}

	bool overflow = ((userdata->m_current_mem_cvg + userdata->m_current_pix_cvg) & 8) > 0;
	userdata->m_blend_enable = (object.m_other_modes.force_blend || (!overflow && object.m_other_modes.antialias_en && farther)) ? 1 : 0;
	userdata->m_pre_wrap = overflow;

	int32_t cvgcoeff = 0;
	uint32_t dzenc = 0;

	if (object.m_other_modes.z_mode == 1 && infront && farther && overflow)
	{
		dzenc = dz_compress(dznotshift & 0xffff);
		cvgcoeff = ((oz >> dzenc) - (sz >> dzenc)) & 0xf;
		userdata->m_current_pix_cvg = ((cvgcoeff * userdata->m_current_pix_cvg) >> 3) & 0xf;
	}

	if (!object.m_other_modes.z_compare_en)
	{
		return true;
	}

	int32_t diff = (int32_t)sz - (int32_t)dznew;
	bool nearer = diff <= (int32_t)oz;
	bool max = (oz == 0x3ffff);
	if (force_coplanar)
	{
		nearer = true;
	}

	switch(object.m_other_modes.z_mode)
	{
	case 0:
		return (max || (overflow ? infront : nearer));
	case 1:
		return (max || (overflow ? infront : nearer));
	case 2:
		return (infront || max);
	case 3:
		return (farther && nearer && !max);
	}

	return false;
}

uint32_t n64_rdp::get_log2(uint32_t lod_clamp)
{
	if (lod_clamp < 2)
	{
		return 0;
	}
	else
	{
		for (int32_t i = 7; i > 0; i--)
		{
			if ((lod_clamp >> i) & 1)
			{
				return i;
			}
		}
	}

	return 0;
}

/*****************************************************************************/

uint64_t n64_rdp::read_data(uint32_t address)
{
	if (m_status & 0x1)     // XBUS_DMEM_DMA enabled
	{
		return (uint64_t(m_dmem[(address & 0xfff) / 4]) << 32) | m_dmem[((address + 4) & 0xfff) / 4];
	}
	else
	{
		return (uint64_t(m_rdram[((address & 0xffffff) / 4)]) << 32) | m_rdram[(((address + 4) & 0xffffff) / 4)];
	}
}

char const *const  n64_rdp::s_image_format[] = { "RGBA", "YUV", "CI", "IA", "I", "???", "???", "???" };
char const *const  n64_rdp::s_image_size[] = { "4-bit", "8-bit", "16-bit", "32-bit" };

int32_t const n64_rdp::s_rdp_command_length[64] =
{
	8,          // 0x00, No Op
	8,          // 0x01, ???
	8,          // 0x02, ???
	8,          // 0x03, ???
	8,          // 0x04, ???
	8,          // 0x05, ???
	8,          // 0x06, ???
	8,          // 0x07, ???
	32,         // 0x08, Non-Shaded Triangle
	32+16,      // 0x09, Non-Shaded, Z-Buffered Triangle
	32+64,      // 0x0a, Textured Triangle
	32+64+16,   // 0x0b, Textured, Z-Buffered Triangle
	32+64,      // 0x0c, Shaded Triangle
	32+64+16,   // 0x0d, Shaded, Z-Buffered Triangle
	32+64+64,   // 0x0e, Shaded+Textured Triangle
	32+64+64+16,// 0x0f, Shaded+Textured, Z-Buffered Triangle
	8,          // 0x10, ???
	8,          // 0x11, ???
	8,          // 0x12, ???
	8,          // 0x13, ???
	8,          // 0x14, ???
	8,          // 0x15, ???
	8,          // 0x16, ???
	8,          // 0x17, ???
	8,          // 0x18, ???
	8,          // 0x19, ???
	8,          // 0x1a, ???
	8,          // 0x1b, ???
	8,          // 0x1c, ???
	8,          // 0x1d, ???
	8,          // 0x1e, ???
	8,          // 0x1f, ???
	8,          // 0x20, ???
	8,          // 0x21, ???
	8,          // 0x22, ???
	8,          // 0x23, ???
	16,         // 0x24, Texture_Rectangle
	16,         // 0x25, Texture_Rectangle_Flip
	8,          // 0x26, Sync_Load
	8,          // 0x27, Sync_Pipe
	8,          // 0x28, Sync_Tile
	8,          // 0x29, Sync_Full
	8,          // 0x2a, Set_Key_GB
	8,          // 0x2b, Set_Key_R
	8,          // 0x2c, Set_Convert
	8,          // 0x2d, Set_Scissor
	8,          // 0x2e, Set_Prim_Depth
	8,          // 0x2f, Set_Other_Modes
	8,          // 0x30, Load_TLUT
	8,          // 0x31, ???
	8,          // 0x32, Set_Tile_Size
	8,          // 0x33, Load_Block
	8,          // 0x34, Load_Tile
	8,          // 0x35, Set_Tile
	8,          // 0x36, Fill_Rectangle
	8,          // 0x37, Set_Fill_Color
	8,          // 0x38, Set_Fog_Color
	8,          // 0x39, Set_Blend_Color
	8,          // 0x3a, Set_Prim_Color
	8,          // 0x3b, Set_Env_Color
	8,          // 0x3c, Set_Combine
	8,          // 0x3d, Set_Texture_Image
	8,          // 0x3e, Set_Mask_Image
	8           // 0x3f, Set_Color_Image
};

void n64_rdp::disassemble(uint64_t *cmd_buf, char* buffer)
{
	char sl[32], tl[32], sh[32], th[32];
	char s[32], t[32], w[32];
	char dsdx[32], dtdx[32], dwdx[32];
	char dsdy[32], dtdy[32], dwdy[32];
	char dsde[32], dtde[32], dwde[32];
	char yl[32], yh[32], ym[32], xl[32], xh[32], xm[32];
	char dxldy[32], dxhdy[32], dxmdy[32];
	char rt[32], gt[32], bt[32], at[32];
	char drdx[32], dgdx[32], dbdx[32], dadx[32];
	char drdy[32], dgdy[32], dbdy[32], dady[32];
	char drde[32], dgde[32], dbde[32], dade[32];

	const int32_t tile = (cmd_buf[0] >> 56) & 0x7;
	sprintf(sl, "%4.2f", (float)((cmd_buf[0] >> 44) & 0xfff) / 4.0f);
	sprintf(tl, "%4.2f", (float)((cmd_buf[0] >> 32) & 0xfff) / 4.0f);
	sprintf(sh, "%4.2f", (float)((cmd_buf[0] >> 12) & 0xfff) / 4.0f);
	sprintf(th, "%4.2f", (float)((cmd_buf[0] >>  0) & 0xfff) / 4.0f);

	const char* format = s_image_format[(cmd_buf[0] >> 53) & 0x7];
	const char* size = s_image_size[(cmd_buf[0] >> 51) & 0x3];

	const uint32_t r = (cmd_buf[0] >> 24) & 0xff;
	const uint32_t g = (cmd_buf[0] >> 16) & 0xff;
	const uint32_t b = (cmd_buf[0] >>  8) & 0xff;
	const uint32_t a = (cmd_buf[0] >>  0) & 0xff;

	const uint32_t command = (cmd_buf[0] >> 56) & 0x3f;
	switch (command)
	{
		case 0x00:  sprintf(buffer, "No Op"); break;
		case 0x08:      // Tri_NoShade
		{
			const int32_t lft = (cmd_buf[0] >> 55) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(buffer, "Tri_NoShade            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x09:      // Tri_NoShadeZ
		{
			const int32_t lft = (cmd_buf[0] >> 55) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(buffer, "Tri_NoShadeZ            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x0a:      // Tri_Tex
		{
			const int32_t lft = (cmd_buf[0] >> 55) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(s,      "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_Tex               %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x0b:      // Tri_TexZ
		{
			const int32_t lft = (cmd_buf[0] >> 55) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(s,      "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_TexZ               %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x0c:      // Tri_Shade
		{
			const int32_t lft = (cmd_buf[0] >> 23) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(rt,     "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)int32_t( ((cmd_buf[4]        & 0x0000ffff) << 16) | ( cmd_buf[ 6]        & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)int32_t( ((cmd_buf[5]        & 0x0000ffff) << 16) | ( cmd_buf[ 7]        & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)int32_t( ((cmd_buf[8]        & 0x0000ffff) << 16) | ( cmd_buf[10]        & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)int32_t( ((cmd_buf[9]        & 0x0000ffff) << 16) | ( cmd_buf[11]        & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_Shade              %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);
			break;
		}
		case 0x0d:      // Tri_ShadeZ
		{
			const int32_t lft = (cmd_buf[0] >> 23) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(rt,     "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)int32_t( ((cmd_buf[4]        & 0x0000ffff) << 16) | ( cmd_buf[ 6]        & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)int32_t( ((cmd_buf[5]        & 0x0000ffff) << 16) | ( cmd_buf[ 7]        & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)int32_t( ((cmd_buf[8]        & 0x0000ffff) << 16) | ( cmd_buf[10]        & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)int32_t( ((cmd_buf[9]        & 0x0000ffff) << 16) | ( cmd_buf[11]        & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_ShadeZ              %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);
			break;
		}
		case 0x0e:      // Tri_TexShade
		{
			const int32_t lft = (cmd_buf[0] >> 23) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(rt,     "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)int32_t( ((cmd_buf[4]        & 0x0000ffff) << 16) | ( cmd_buf[ 6]        & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)int32_t( ((cmd_buf[5]        & 0x0000ffff) << 16) | ( cmd_buf[ 7]        & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)int32_t( ((cmd_buf[8]        & 0x0000ffff) << 16) | ( cmd_buf[10]        & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)int32_t( ((cmd_buf[9]        & 0x0000ffff) << 16) | ( cmd_buf[11]        & 0xffff)) / 65536.0f);

			sprintf(s,      "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_TexShade           %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);

			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x0f:      // Tri_TexShadeZ
		{
			const int32_t lft = (cmd_buf[0] >> 23) & 0x1;

			sprintf(yl,     "%4.4f", (float)((cmd_buf[0] >> 32) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd_buf[0] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd_buf[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)int32_t(cmd_buf[1] >> 32) / 65536.0f);
			sprintf(dxldy,  "%4.4f", (float)int32_t(cmd_buf[1])       / 65536.0f);
			sprintf(xh,     "%4.4f", (float)int32_t(cmd_buf[2] >> 32) / 65536.0f);
			sprintf(dxhdy,  "%4.4f", (float)int32_t(cmd_buf[2])       / 65536.0f);
			sprintf(xm,     "%4.4f", (float)int32_t(cmd_buf[3] >> 32) / 65536.0f);
			sprintf(dxmdy,  "%4.4f", (float)int32_t(cmd_buf[3])       / 65536.0f);

			sprintf(rt,     "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)int32_t( ((cmd_buf[4]        & 0x0000ffff) << 16) | ( cmd_buf[ 6]        & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)int32_t( ((cmd_buf[5]        & 0x0000ffff) << 16) | ( cmd_buf[ 7]        & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)int32_t( ((cmd_buf[8]        & 0x0000ffff) << 16) | ( cmd_buf[10]        & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)int32_t( ((cmd_buf[9]        & 0x0000ffff) << 16) | ( cmd_buf[11]        & 0xffff)) / 65536.0f);

			sprintf(s,      "%4.4f", (float)int32_t( ((cmd_buf[4] >> 32) & 0xffff0000)        | ((cmd_buf[ 6] >> 48) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)int32_t((((cmd_buf[4] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 6] >> 32) & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)int32_t(  (cmd_buf[4]        & 0xffff0000)        | ((cmd_buf[ 6] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)int32_t( ((cmd_buf[5] >> 32) & 0xffff0000)        | ((cmd_buf[ 7] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)int32_t((((cmd_buf[5] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[ 7] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)int32_t(  (cmd_buf[5]        & 0xffff0000)        | ((cmd_buf[ 7] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)int32_t( ((cmd_buf[8] >> 32) & 0xffff0000)        | ((cmd_buf[10] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)int32_t((((cmd_buf[8] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[10] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)int32_t(  (cmd_buf[8]        & 0xffff0000)        | ((cmd_buf[10] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)int32_t( ((cmd_buf[9] >> 32) & 0xffff0000)        | ((cmd_buf[11] >> 48) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)int32_t((((cmd_buf[9] >> 32) & 0x0000ffff) << 16) | ((cmd_buf[11] >> 32) & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)int32_t(  (cmd_buf[9]        & 0xffff0000)        | ((cmd_buf[11] >> 16) & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_TexShadeZ           %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);

			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x24:
		case 0x25:
		{
			sprintf(s,    "%4.4f", (float)int16_t((cmd_buf[1] >> 48) & 0xffff) / 32.0f);
			sprintf(t,    "%4.4f", (float)int16_t((cmd_buf[1] >> 32) & 0xffff) / 32.0f);
			sprintf(dsdx, "%4.4f", (float)int16_t((cmd_buf[1] >> 16) & 0xffff) / 1024.0f);
			sprintf(dtdy, "%4.4f", (float)int16_t((cmd_buf[1] >>  0) & 0xffff) / 1024.0f);

			if (command == 0x24)
					sprintf(buffer, "Texture_Rectangle      %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);
			else
					sprintf(buffer, "Texture_Rectangle_Flip %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);

			break;
		}
		case 0x26:  sprintf(buffer, "Sync_Load"); break;
		case 0x27:  sprintf(buffer, "Sync_Pipe"); break;
		case 0x28:  sprintf(buffer, "Sync_Tile"); break;
		case 0x29:  sprintf(buffer, "Sync_Full"); break;
		case 0x2d:  sprintf(buffer, "Set_Scissor            %s, %s, %s, %s", sl, tl, sh, th); break;
		case 0x2e:  sprintf(buffer, "Set_Prim_Depth         %04X, %04X", uint32_t(cmd_buf[0] >> 16) & 0xffff, (uint32_t)cmd_buf[0] & 0xffff); break;
		case 0x2f:  sprintf(buffer, "Set_Other_Modes        %08X %08X", uint32_t(cmd_buf[0] >> 32), (uint32_t)cmd_buf[0]); break;
		case 0x30:  sprintf(buffer, "Load_TLUT              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x32:  sprintf(buffer, "Set_Tile_Size          %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x33:  sprintf(buffer, "Load_Block             %d, %03X, %03X, %03X, %03X", tile, uint32_t(cmd_buf[0] >> 44) & 0xfff, uint32_t(cmd_buf[0] >> 32) & 0xfff, uint32_t(cmd_buf[0] >> 12) & 0xfff, uint32_t(cmd_buf[0]) & 0xfff); break;
		case 0x34:  sprintf(buffer, "Load_Tile              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x35:  sprintf(buffer, "Set_Tile               %d, %s, %s, %d, %04X", tile, format, size, (uint32_t(cmd_buf[0] >> 41) & 0x1ff) * 8, (uint32_t(cmd_buf[0] >> 32) & 0x1ff) * 8); break;
		case 0x36:  sprintf(buffer, "Fill_Rectangle         %s, %s, %s, %s", sh, th, sl, tl); break;
		case 0x37:  sprintf(buffer, "Set_Fill_Color         R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x38:  sprintf(buffer, "Set_Fog_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x39:  sprintf(buffer, "Set_Blend_Color        R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3a:  sprintf(buffer, "Set_Prim_Color         %d, %d, R: %d, G: %d, B: %d, A: %d", uint32_t(cmd_buf[0] >> 40) & 0x1f, uint32_t(cmd_buf[0] >> 32) & 0xff, r, g, b, a); break;
		case 0x3b:  sprintf(buffer, "Set_Env_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3c:  sprintf(buffer, "Set_Combine            %08X %08X", uint32_t(cmd_buf[0] >> 32), (uint32_t)cmd_buf[0]); break;
		case 0x3d:  sprintf(buffer, "Set_Texture_Image      %s, %s, %d, %08X", format, size, (uint32_t(cmd_buf[0] >> 32) & 0x1ff) + 1, (uint32_t)cmd_buf[0]); break;
		case 0x3e:  sprintf(buffer, "Set_Mask_Image         %08X", (uint32_t)cmd_buf[0]); break;
		case 0x3f:  sprintf(buffer, "Set_Color_Image        %s, %s, %d, %08X", format, size, (uint32_t(cmd_buf[0] >> 32) & 0x1ff) + 1, (uint32_t)cmd_buf[0]); break;
		default:    sprintf(buffer, "Unknown (%08X %08X)", uint32_t(cmd_buf[0] >> 32), (uint32_t)cmd_buf[0]); break;
	}
}

/*****************************************************************************/

static uint32_t rightcvghex(uint32_t x, uint32_t fmask)
{
	uint32_t stickybit = ((x >> 1) & 0x1fff) > 0;
	uint32_t covered = ((x >> 14) & 3) + stickybit;
	covered = (0xf0 >> covered) & 0xf;
	return (covered & fmask);
}

static uint32_t leftcvghex(uint32_t x, uint32_t fmask)
{
	uint32_t stickybit = ((x >> 1) & 0x1fff) > 0;
	uint32_t covered = ((x >> 14) & 3) + stickybit;
	covered = 0xf >> covered;
	return (covered & fmask);
}

static int32_t CLIP(int32_t value,int32_t min,int32_t max)
{
	if (value < min)
	{
		return min;
	}
	else if (value > max)
	{
		return max;
	}
	else
	{
		return value;
	}
}

void n64_rdp::compute_cvg_noflip(extent_t* spans, int32_t* majorx, int32_t* minorx, int32_t* majorxint, int32_t* minorxint, int32_t scanline, int32_t yh, int32_t yl, int32_t base)
{
	int32_t purgestart = 0xfff;
	int32_t purgeend = 0;
	const bool writablescanline = !(scanline & ~0x3ff);
	const int32_t scanlinespx = scanline << 2;

	if (!writablescanline) return;

	for(int32_t i = 0; i < 4; i++)
	{
		if (minorxint[i] < purgestart)
		{
			purgestart = minorxint[i];
		}
		if (majorxint[i] > purgeend)
		{
			purgeend = majorxint[i];
		}
	}

	purgestart = CLIP(purgestart, 0, 1023);
	purgeend = CLIP(purgeend, 0, 1023);
	int32_t length = purgeend - purgestart;

	if (length < 0) return;

	rdp_span_aux* userdata = (rdp_span_aux*)spans[scanline - base].userdata;
	memset(&userdata->m_cvg[purgestart], 0, (length + 1) << 1);

	for(int32_t i = 0; i < 4; i++)
	{
		int32_t minorcur = minorx[i];
		int32_t majorcur = majorx[i];
		int32_t minorcurint = minorxint[i];
		int32_t majorcurint = majorxint[i];
		length = majorcurint - minorcurint;

		int32_t fmask = (i & 1) ? 5 : 0xa;
		int32_t maskshift = (i ^ 3) << 2;
		int32_t fmaskshifted = fmask << maskshift;
		int32_t fleft = CLIP(minorcurint + 1, 0, 647);
		int32_t fright = CLIP(majorcurint - 1, 0, 647);
		bool valid_y = ((scanlinespx + i) >= yh && (scanlinespx + i) < yl);
		if (valid_y && length >= 0)
		{
			if (minorcurint != majorcurint)
			{
				if (!(minorcurint & ~0x3ff))
				{
					userdata->m_cvg[minorcurint] |= (leftcvghex(minorcur, fmask) << maskshift);
				}
				if (!(majorcurint & ~0x3ff))
				{
					userdata->m_cvg[majorcurint] |= (rightcvghex(majorcur, fmask) << maskshift);
				}
			}
			else
			{
				if (!(majorcurint & ~0x3ff))
				{
					int32_t samecvg = leftcvghex(minorcur, fmask) & rightcvghex(majorcur, fmask);
					userdata->m_cvg[majorcurint] |= (samecvg << maskshift);
				}
			}
			for (; fleft <= fright; fleft++)
			{
				userdata->m_cvg[fleft] |= fmaskshifted;
			}
		}
	}
}

void n64_rdp::compute_cvg_flip(extent_t* spans, int32_t* majorx, int32_t* minorx, int32_t* majorxint, int32_t* minorxint, int32_t scanline, int32_t yh, int32_t yl, int32_t base)
{
	int32_t purgestart = 0xfff;
	int32_t purgeend = 0;
	const bool writablescanline = !(scanline & ~0x3ff);
	const int32_t scanlinespx = scanline << 2;

	if(!writablescanline) return;

	for(int32_t i = 0; i < 4; i++)
	{
		if (majorxint[i] < purgestart)
		{
			purgestart = majorxint[i];
		}
		if (minorxint[i] > purgeend)
		{
			purgeend = minorxint[i];
		}
	}

	purgestart = CLIP(purgestart, 0, 1023);
	purgeend = CLIP(purgeend, 0, 1023);

	int32_t length = purgeend - purgestart;

	if (length < 0) return;

	rdp_span_aux* userdata = (rdp_span_aux*)spans[scanline - base].userdata;
	memset(&userdata->m_cvg[purgestart], 0, (length + 1) << 1);

	for(int32_t i = 0; i < 4; i++)
	{
		int32_t minorcur = minorx[i];
		int32_t majorcur = majorx[i];
		int32_t minorcurint = minorxint[i];
		int32_t majorcurint = majorxint[i];
		length = minorcurint - majorcurint;

		int32_t fmask = (i & 1) ? 5 : 0xa;
		int32_t maskshift = (i ^ 3) << 2;
		int32_t fmaskshifted = fmask << maskshift;
		int32_t fleft = CLIP(majorcurint + 1, 0, 647);
		int32_t fright = CLIP(minorcurint - 1, 0, 647);
		bool valid_y = ((scanlinespx + i) >= yh && (scanlinespx + i) < yl);
		if (valid_y && length >= 0)
		{
			if (minorcurint != majorcurint)
			{
				if (!(minorcurint & ~0x3ff))
				{
					userdata->m_cvg[minorcurint] |= (rightcvghex(minorcur, fmask) << maskshift);
				}
				if (!(majorcurint & ~0x3ff))
				{
					userdata->m_cvg[majorcurint] |= (leftcvghex(majorcur, fmask) << maskshift);
				}
			}
			else
			{
				if (!(majorcurint & ~0x3ff))
				{
					int32_t samecvg = rightcvghex(minorcur, fmask) & leftcvghex(majorcur, fmask);
					userdata->m_cvg[majorcurint] |= (samecvg << maskshift);
				}
			}
			for (; fleft <= fright; fleft++)
			{
				userdata->m_cvg[fleft] |= fmaskshifted;
			}
		}
	}
}

#define SIGN(x, numb)   (((x) & ((1 << numb) - 1)) | -((x) & (1 << (numb - 1))))

void n64_rdp::draw_triangle(uint64_t *cmd_buf, bool shade, bool texture, bool zbuffer, bool rect)
{
	const uint64_t* cmd_data = rect ? m_temp_rect_data : cmd_buf;
	const uint64_t w1 = cmd_data[0];

	int32_t flip = int32_t(w1 >> 55) & 1;
	m_misc_state.m_max_level = uint32_t(w1 >> 51) & 7;
	int32_t tilenum = int32_t(w1 >> 48) & 0x7;

	int32_t dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	int32_t dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0;
	int32_t dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;
	int32_t dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;

	int32_t maxxmx = 0; // maxxmx / minxhx very opaque names, consider re-naming
	int32_t minxmx = 0;
	int32_t maxxhx = 0;
	int32_t minxhx = 0;

	int32_t shade_base = 4;
	int32_t texture_base = 4;
	int32_t zbuffer_base = 4;
	if(shade)
	{
		texture_base += 8;
		zbuffer_base += 8;
	}
	if(texture)
	{
		zbuffer_base += 8;
	}

	uint64_t w2 = cmd_data[1];
	uint64_t w3 = cmd_data[2];
	uint64_t w4 = cmd_data[3];

	int32_t yl = int32_t(w1 >> 32) & 0x3fff;
	int32_t ym = int32_t(w1 >> 16) & 0x3fff;
	int32_t yh = int32_t(w1 >>  0) & 0x3fff;
	int32_t xl = (int32_t)(w2 >> 32) & 0x3fffffff;
	int32_t xh = (int32_t)(w3 >> 32) & 0x3fffffff;
	int32_t xm = (int32_t)(w4 >> 32) & 0x3fffffff;
	// Inverse slopes in 16.16 format
	int32_t dxldy = (int32_t)w2;
	int32_t dxhdy = (int32_t)w3;
	int32_t dxmdy = (int32_t)w4;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	int32_t r    = int32_t(((cmd_data[shade_base] >> 32) & 0xffff0000) | ((cmd_data[shade_base + 2] >> 48) & 0x0000ffff));
	int32_t g    = int32_t(((cmd_data[shade_base] >> 16) & 0xffff0000) | ((cmd_data[shade_base + 2] >> 32) & 0x0000ffff));
	int32_t b    = int32_t( (cmd_data[shade_base]        & 0xffff0000) | ((cmd_data[shade_base + 2] >> 16) & 0x0000ffff));
	int32_t a    = int32_t(((cmd_data[shade_base] << 16) & 0xffff0000) |  (cmd_data[shade_base + 2]        & 0x0000ffff));
	const int32_t drdx = int32_t(((cmd_data[shade_base + 1] >> 32) & 0xffff0000) | ((cmd_data[shade_base + 3] >> 48) & 0x0000ffff));
	const int32_t dgdx = int32_t(((cmd_data[shade_base + 1] >> 16) & 0xffff0000) | ((cmd_data[shade_base + 3] >> 32) & 0x0000ffff));
	const int32_t dbdx = int32_t( (cmd_data[shade_base + 1]        & 0xffff0000) | ((cmd_data[shade_base + 3] >> 16) & 0x0000ffff));
	const int32_t dadx = int32_t(((cmd_data[shade_base + 1] << 16) & 0xffff0000) |  (cmd_data[shade_base + 3]        & 0x0000ffff));
	const int32_t drde = int32_t(((cmd_data[shade_base + 4] >> 32) & 0xffff0000) | ((cmd_data[shade_base + 6] >> 48) & 0x0000ffff));
	const int32_t dgde = int32_t(((cmd_data[shade_base + 4] >> 16) & 0xffff0000) | ((cmd_data[shade_base + 6] >> 32) & 0x0000ffff));
	const int32_t dbde = int32_t( (cmd_data[shade_base + 4]        & 0xffff0000) | ((cmd_data[shade_base + 6] >> 16) & 0x0000ffff));
	const int32_t dade = int32_t(((cmd_data[shade_base + 4] << 16) & 0xffff0000) |  (cmd_data[shade_base + 6]        & 0x0000ffff));
	const int32_t drdy = int32_t(((cmd_data[shade_base + 5] >> 32) & 0xffff0000) | ((cmd_data[shade_base + 7] >> 48) & 0x0000ffff));
	const int32_t dgdy = int32_t(((cmd_data[shade_base + 5] >> 16) & 0xffff0000) | ((cmd_data[shade_base + 7] >> 32) & 0x0000ffff));
	const int32_t dbdy = int32_t( (cmd_data[shade_base + 5]        & 0xffff0000) | ((cmd_data[shade_base + 7] >> 16) & 0x0000ffff));
	const int32_t dady = int32_t(((cmd_data[shade_base + 5] << 16) & 0xffff0000) |  (cmd_data[shade_base + 7]        & 0x0000ffff));

	int32_t s    = int32_t(((cmd_data[texture_base] >> 32) & 0xffff0000) | ((cmd_data[texture_base+ 2 ] >> 48) & 0x0000ffff));
	int32_t t    = int32_t(((cmd_data[texture_base] >> 16) & 0xffff0000) | ((cmd_data[texture_base+ 2 ] >> 32) & 0x0000ffff));
	int32_t w    = int32_t( (cmd_data[texture_base]        & 0xffff0000) | ((cmd_data[texture_base+ 2 ] >> 16) & 0x0000ffff));
	const int32_t dsdx = int32_t(((cmd_data[texture_base + 1] >> 32) & 0xffff0000) | ((cmd_data[texture_base + 3] >> 48) & 0x0000ffff));
	const int32_t dtdx = int32_t(((cmd_data[texture_base + 1] >> 16) & 0xffff0000) | ((cmd_data[texture_base + 3] >> 32) & 0x0000ffff));
	const int32_t dwdx = int32_t( (cmd_data[texture_base + 1]        & 0xffff0000) | ((cmd_data[texture_base + 3] >> 16) & 0x0000ffff));
	const int32_t dsde = int32_t(((cmd_data[texture_base + 4] >> 32) & 0xffff0000) | ((cmd_data[texture_base + 6] >> 48) & 0x0000ffff));
	const int32_t dtde = int32_t(((cmd_data[texture_base + 4] >> 16) & 0xffff0000) | ((cmd_data[texture_base + 6] >> 32) & 0x0000ffff));
	const int32_t dwde = int32_t( (cmd_data[texture_base + 4]        & 0xffff0000) | ((cmd_data[texture_base + 6] >> 16) & 0x0000ffff));
	const int32_t dsdy = int32_t(((cmd_data[texture_base + 5] >> 32) & 0xffff0000) | ((cmd_data[texture_base + 7] >> 48) & 0x0000ffff));
	const int32_t dtdy = int32_t(((cmd_data[texture_base + 5] >> 16) & 0xffff0000) | ((cmd_data[texture_base + 7] >> 32) & 0x0000ffff));
	const int32_t dwdy = int32_t( (cmd_data[texture_base + 5]        & 0xffff0000) | ((cmd_data[texture_base + 7] >> 16) & 0x0000ffff));

	int32_t z    = int32_t(cmd_data[zbuffer_base] >> 32);
	const int32_t dzdx = int32_t(cmd_data[zbuffer_base]);
	const int32_t dzde = int32_t(cmd_data[zbuffer_base+1] >> 32);
	const int32_t dzdy = int32_t(cmd_data[zbuffer_base+1]);

	const int32_t dzdy_dz = (dzdy >> 16) & 0xffff;
	const int32_t dzdx_dz = (dzdx >> 16) & 0xffff;

	extent_t spans[2048];
#ifdef MAME_DEBUG
	memset(spans, 0xcc, sizeof(spans));
#endif

	m_span_base.m_span_drdy = drdy;
	m_span_base.m_span_dgdy = dgdy;
	m_span_base.m_span_dbdy = dbdy;
	m_span_base.m_span_dady = dady;
	m_span_base.m_span_dzdy = m_other_modes.z_source_sel ? 0 : dzdy;

	uint32_t temp_dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	m_span_base.m_span_dr = drdx & ~0x1f;
	m_span_base.m_span_dg = dgdx & ~0x1f;
	m_span_base.m_span_db = dbdx & ~0x1f;
	m_span_base.m_span_da = dadx & ~0x1f;
	m_span_base.m_span_ds = dsdx;
	m_span_base.m_span_dt = dtdx;
	m_span_base.m_span_dw = dwdx;
	m_span_base.m_span_dz = m_other_modes.z_source_sel ? 0 : dzdx;
	m_span_base.m_span_dymax = 0;
	m_span_base.m_span_dzpix = m_dzpix_normalize[temp_dzpix & 0xffff];

	int32_t xleft_inc = (dxmdy >> 2) & ~1;
	int32_t xright_inc = (dxhdy >> 2) & ~1;

	int32_t xright = xh & ~1;
	int32_t xleft = xm & ~1;

	const int32_t sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;
	const int32_t do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;  dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;  dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;  dwdyh = dwdy >> 9;
		drdeh = drde >> 9;  drdyh = drdy >> 9;
		dgdeh = dgde >> 9;  dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;  dbdyh = dbdy >> 9;
		dadeh = dade >> 9;  dadyh = dady >> 9;
		dzdeh = dzde >> 9;  dzdyh = dzdy >> 9;

		dsdiff = (dsdeh << 8) + (dsdeh << 7) - (dsdyh << 8) - (dsdyh << 7);
		dtdiff = (dtdeh << 8) + (dtdeh << 7) - (dtdyh << 8) - (dtdyh << 7);
		dwdiff = (dwdeh << 8) + (dwdeh << 7) - (dwdyh << 8) - (dwdyh << 7);
		drdiff = (drdeh << 8) + (drdeh << 7) - (drdyh << 8) - (drdyh << 7);
		dgdiff = (dgdeh << 8) + (dgdeh << 7) - (dgdyh << 8) - (dgdyh << 7);
		dbdiff = (dbdeh << 8) + (dbdeh << 7) - (dbdyh << 8) - (dbdyh << 7);
		dadiff = (dadeh << 8) + (dadeh << 7) - (dadyh << 8) - (dadyh << 7);
		dzdiff = (dzdeh << 8) + (dzdeh << 7) - (dzdyh << 8) - (dzdyh << 7);
	}
	else
	{
		dsdiff = dtdiff = dwdiff = drdiff = dgdiff = dbdiff = dadiff = dzdiff = 0;
	}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;
	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;
	dzdxh = dzdx >> 8;

	const int32_t ycur = yh & ~3;
	const int32_t ylfar = yl | 3;
	const int32_t ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	int32_t majorx[4];
	int32_t minorx[4];
	int32_t majorxint[4];
	int32_t minorxint[4];

	int32_t xfrac = ((xright >> 8) & 0xff);

	const int32_t clipy1 = m_scissor.m_yh;
	const int32_t clipy2 = m_scissor.m_yl;

	// Trivial reject
	if((ycur >> 2) >= clipy2 && (ylfar >> 2) >= clipy2)
	{
		return;
	}
	if((ycur >> 2) < clipy1 && (ylfar >> 2) < clipy1)
	{
		return;
	}

	bool new_object = true;
	rdp_poly_state* object = nullptr;
	bool valid = false;

	int32_t* minx = flip ? &minxhx : &minxmx;
	int32_t* maxx = flip ? &maxxmx : &maxxhx;
	int32_t* startx = flip ? maxx : minx;
	int32_t* endx = flip ? minx : maxx;

	for (int32_t k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl & ~1;
			xleft_inc = (dxldy >> 2) & ~1;
		}

		const int32_t xstart = xleft >> 16;
		const int32_t xend = xright >> 16;
		const int32_t j = k >> 2;
		const int32_t spanidx = (k - ycur) >> 2;
		const int32_t  spix = k & 3;
		bool valid_y = !(k < yh || k >= yl);

		if (spanidx >= 0 && spanidx < 2048)
		{
			majorxint[spix] = xend;
			minorxint[spix] = xstart;
			majorx[spix] = xright;
			minorx[spix] = xleft;

			if (spix == 0)
			{
				*maxx = 0;
				*minx = 0xfff;
			}

			if (valid_y)
			{
				if (flip)
				{
					*maxx = std::max(xstart, *maxx);
					*minx = std::min(xend, *minx);
				}
				else
				{
					*minx = std::min(xstart, *minx);
					*maxx = std::max(xend, *maxx);
				}
			}

			if (spix == 0)
			{
				if(new_object)
				{
					object = &object_data().next();
					memcpy(object->m_tmem, m_tmem.get(), 0x1000);
					new_object = false;
				}

				spans[spanidx].userdata = (void*)((uint8_t*)m_aux_buf.get() + m_aux_buf_ptr);
				valid = true;
				m_aux_buf_ptr += sizeof(rdp_span_aux);

				if(m_aux_buf_ptr >= EXTENT_AUX_COUNT)
				{
					fatalerror("n64_rdp::draw_triangle: span aux buffer overflow\n");
				}

				rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
				memcpy(&userdata->m_combine, &m_combine, sizeof(combine_modes_t));
				userdata->m_tmem = object->m_tmem;

				userdata->m_blend_color = m_blend_color;
				userdata->m_prim_color = m_prim_color;
				userdata->m_env_color = m_env_color;
				userdata->m_fog_color = m_fog_color;
				userdata->m_prim_alpha = m_prim_alpha;
				userdata->m_env_alpha = m_env_alpha;
				userdata->m_key_scale = m_key_scale;
				userdata->m_lod_fraction = m_lod_fraction;
				userdata->m_prim_lod_fraction = m_prim_lod_fraction;
				userdata->m_k4 = m_k4;
				userdata->m_k5 = m_k5;

				// Setup blender data for this scanline
				set_blender_input(0, 0, &userdata->m_color_inputs.blender1a_rgb[0], &userdata->m_color_inputs.blender1b_a[0], m_other_modes.blend_m1a_0, m_other_modes.blend_m1b_0, userdata);
				set_blender_input(0, 1, &userdata->m_color_inputs.blender2a_rgb[0], &userdata->m_color_inputs.blender2b_a[0], m_other_modes.blend_m2a_0, m_other_modes.blend_m2b_0, userdata);
				set_blender_input(1, 0, &userdata->m_color_inputs.blender1a_rgb[1], &userdata->m_color_inputs.blender1b_a[1], m_other_modes.blend_m1a_1, m_other_modes.blend_m1b_1, userdata);
				set_blender_input(1, 1, &userdata->m_color_inputs.blender2a_rgb[1], &userdata->m_color_inputs.blender2b_a[1], m_other_modes.blend_m2a_1, m_other_modes.blend_m2b_1, userdata);

				// Setup color combiner data for this scanline
				set_suba_input_rgb(&userdata->m_color_inputs.combiner_rgbsub_a[0], m_combine.sub_a_rgb0, userdata);
				set_subb_input_rgb(&userdata->m_color_inputs.combiner_rgbsub_b[0], m_combine.sub_b_rgb0, userdata);
				set_mul_input_rgb(&userdata->m_color_inputs.combiner_rgbmul[0], m_combine.mul_rgb0, userdata);
				set_add_input_rgb(&userdata->m_color_inputs.combiner_rgbadd[0], m_combine.add_rgb0, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphasub_a[0], m_combine.sub_a_a0, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphasub_b[0], m_combine.sub_b_a0, userdata);
				set_mul_input_alpha(&userdata->m_color_inputs.combiner_alphamul[0], m_combine.mul_a0, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphaadd[0], m_combine.add_a0, userdata);

				set_suba_input_rgb(&userdata->m_color_inputs.combiner_rgbsub_a[1], m_combine.sub_a_rgb1, userdata);
				set_subb_input_rgb(&userdata->m_color_inputs.combiner_rgbsub_b[1], m_combine.sub_b_rgb1, userdata);
				set_mul_input_rgb(&userdata->m_color_inputs.combiner_rgbmul[1], m_combine.mul_rgb1, userdata);
				set_add_input_rgb(&userdata->m_color_inputs.combiner_rgbadd[1], m_combine.add_rgb1, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphasub_a[1], m_combine.sub_a_a1, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphasub_b[1], m_combine.sub_b_a1, userdata);
				set_mul_input_alpha(&userdata->m_color_inputs.combiner_alphamul[1], m_combine.mul_a1, userdata);
				set_sub_input_alpha(&userdata->m_color_inputs.combiner_alphaadd[1], m_combine.add_a1, userdata);
			}

			if (spix == 3)
			{
				spans[spanidx].startx = *startx;
				spans[spanidx].stopx = *endx;
				((this)->*(m_compute_cvg[flip]))(spans, majorx, minorx, majorxint, minorxint, j, yh, yl, ycur >> 2);
			}

			if (spix == ldflag)
			{
				((rdp_span_aux*)spans[spanidx].userdata)->m_unscissored_rx = xend;
				xfrac = ((xright >> 8) & 0xff);
				spans[spanidx].param[SPAN_R].start = ((r >> 9) << 9) + drdiff - (xfrac * drdxh);
				spans[spanidx].param[SPAN_G].start = ((g >> 9) << 9) + dgdiff - (xfrac * dgdxh);
				spans[spanidx].param[SPAN_B].start = ((b >> 9) << 9) + dbdiff - (xfrac * dbdxh);
				spans[spanidx].param[SPAN_A].start = ((a >> 9) << 9) + dadiff - (xfrac * dadxh);
				spans[spanidx].param[SPAN_S].start = (((s >> 9) << 9)  + dsdiff - (xfrac * dsdxh)) & ~0x1f;
				spans[spanidx].param[SPAN_T].start = (((t >> 9) << 9)  + dtdiff - (xfrac * dtdxh)) & ~0x1f;
				spans[spanidx].param[SPAN_W].start = (((w >> 9) << 9)  + dwdiff - (xfrac * dwdxh)) & ~0x1f;
				spans[spanidx].param[SPAN_Z].start = ((z >> 9) << 9)  + dzdiff - (xfrac * dzdxh);
			}
		}

		if (spix == 3)
		{
			r += drde;
			g += dgde;
			b += dbde;
			a += dade;
			s += dsde;
			t += dtde;
			w += dwde;
			z += dzde;
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	if(!new_object && valid)
	{
		render_spans(yh >> 2, yl >> 2, tilenum, flip ? true : false, spans, rect, object);
	}
	m_aux_buf_ptr = 0;  // Spans can be reused once render completes
	//wait("draw_triangle");
}

/*****************************************************************************/

////////////////////////
// RDP COMMANDS
////////////////////////

void n64_rdp::triangle(uint64_t *cmd_buf, bool shade, bool texture, bool zbuffer)
{
	draw_triangle(cmd_buf, shade, texture, zbuffer, false);
	m_pipe_clean = false;
}

void n64_rdp::cmd_tex_rect(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	const uint64_t w2 = cmd_buf[1];

	const uint64_t tilenum = (w1 >> 24) & 0x7;
	const uint64_t xh = (w1 >> 12) & 0xfff;
	const uint64_t xl = (w1 >> 44) & 0xfff;
	const uint64_t yh = (w1 >>  0) & 0xfff;
	uint64_t yl       = (w1 >> 32) & 0xfff;

	const uint64_t s  = (w2 >> 48) & 0xffff;
	const uint64_t t  = (w2 >> 32) & 0xffff;
	const uint64_t dsdx = SIGN16((w2 >> 16) & 0xffff);
	const uint64_t dtdy = SIGN16((w2 >>  0) & 0xffff);

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const uint64_t xlint = (xl >> 2) & 0x3ff;
	const uint64_t xhint = (xh >> 2) & 0x3ff;

	uint64_t* ewdata = m_temp_rect_data;
	ewdata[0] = ((uint64_t)0x24 << 56) | ((0x80L | tilenum) << 48) | (yl << 32) | (yl << 16) | yh;   // command, flipped, tile, yl
	ewdata[1] = (xlint << 48) | ((xl & 3) << 46);               // xl, xl frac, dxldy (0), dxldy frac (0)
	ewdata[2] = (xhint << 48) | ((xh & 3) << 46);               // xh, xh frac, dxhdy (0), dxhdy frac (0)
	ewdata[3] = (xlint << 48) | ((xl & 3) << 46);               // xm, xm frac, dxmdy (0), dxmdy frac (0)
	memset(&ewdata[4], 0, 8 * sizeof(uint64_t));                // shade
	ewdata[12] = (s << 48) | (t << 32);                         // s, t, w (0)
	ewdata[13] = (dsdx >> 5) << 48;                             // dsdx, dtdx, dwdx (0)
	ewdata[14] = 0;                                             // s frac (0), t frac (0), w frac (0)
	ewdata[15] = (dsdx & 0x1f) << 59;                           // dsdx frac, dtdx frac, dwdx frac (0)
	ewdata[16] = ((dtdy >> 5) & 0xffff) << 32;                  // dsde, dtde, dwde (0)
	ewdata[17] = ((dtdy >> 5) & 0xffff) << 32;                  // dsdy, dtdy, dwdy (0)
	ewdata[18] = ((dtdy & 0x1f) << 11) << 32;                   // dsde frac, dtde frac, dwde frac (0)
	ewdata[38] = ((dtdy & 0x1f) << 11) << 32;                   // dsdy frac, dtdy frac, dwdy frac (0)
	// ewdata[40-43] = 0;                                       // depth

	draw_triangle(cmd_buf, true, true, false, true);
}

void n64_rdp::cmd_tex_rect_flip(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	const uint64_t w2 = cmd_buf[1];

	const uint64_t tilenum  = (w1 >> 56) & 0x7;
	const uint64_t xh = (w1 >> 12) & 0xfff;
	const uint64_t xl = (w1 >> 44) & 0xfff;
	const uint64_t yh = (w1 >>  0) & 0xfff;
	uint64_t yl       = (w1 >> 32) & 0xfff;

	const uint64_t s  = (w2 >> 48) & 0xffff;
	const uint64_t t  = (w2 >> 32) & 0xffff;
	const uint64_t dsdx = SIGN16((w2 >> 16) & 0xffff);
	const uint64_t dtdy = SIGN16((w2 >>  0) & 0xffff);

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const uint64_t xlint = (xl >> 2) & 0x3ff;
	const uint64_t xhint = (xh >> 2) & 0x3ff;

	uint64_t* ewdata = m_temp_rect_data;
	ewdata[0] = ((uint64_t)0x25 << 56) | ((0x80L | tilenum) << 48) | (yl << 32) | (yl << 16) | yh;   // command, flipped, tile, yl
	ewdata[1] = (xlint << 48) | ((xl & 3) << 46);               // xl, xl frac, dxldy (0), dxldy frac (0)
	ewdata[2] = (xhint << 48) | ((xh & 3) << 46);               // xh, xh frac, dxhdy (0), dxhdy frac (0)
	ewdata[3] = (xlint << 48) | ((xl & 3) << 46);               // xm, xm frac, dxmdy (0), dxmdy frac (0)
	memset(&ewdata[4], 0, 8 * sizeof(uint64_t));                // shade
	ewdata[12] = (s << 48) | (t << 32);                         // s, t, w (0)
	ewdata[13] = ((dtdy >> 5) & 0xffff) << 32;                  // dsdx, dtdx, dwdx (0)
	ewdata[14] = 0;                                             // s frac (0), t frac (0), w frac (0)
	ewdata[15] = ((dtdy & 0x1f) << 43);                         // dsdx frac, dtdx frac, dwdx frac (0)
	ewdata[16] = (dsdx >> 5) << 48;                             // dsde, dtde, dwde (0)
	ewdata[17] = (dsdx >> 5) << 48;                             // dsdy, dtdy, dwdy (0)
	ewdata[18] = (dsdx & 0x1f) << 59;                           // dsde frac, dtde frac, dwde frac (0)
	ewdata[19] = (dsdx & 0x1f) << 59;                           // dsdy frac, dtdy frac, dwdy frac (0)

	draw_triangle(cmd_buf, true, true, false, true);
}

void n64_rdp::cmd_sync_load(uint64_t *cmd_buf)
{
	//wait("SyncLoad");
}

void n64_rdp::cmd_sync_pipe(uint64_t *cmd_buf)
{
	//wait("SyncPipe");
}

void n64_rdp::cmd_sync_tile(uint64_t *cmd_buf)
{
	//wait("SyncTile");
}

void n64_rdp::cmd_sync_full(uint64_t *cmd_buf)
{
	//wait("SyncFull");
	m_n64_periphs->dp_full_sync();
}

void n64_rdp::cmd_set_key_gb(uint64_t *cmd_buf)
{
	m_key_scale.set_b(uint32_t(cmd_buf[0] >>  0) & 0xff);
	m_key_scale.set_g(uint32_t(cmd_buf[0] >> 16) & 0xff);
}

void n64_rdp::cmd_set_key_r(uint64_t *cmd_buf)
{
	m_key_scale.set_r(uint32_t(cmd_buf[0] & 0xff));
}

void n64_rdp::cmd_set_fill_color32(uint64_t *cmd_buf)
{
	//wait("SetFillColor");
	m_fill_color = (uint32_t)cmd_buf[0];
}

void n64_rdp::cmd_set_convert(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];

	if(!m_pipe_clean) { m_pipe_clean = true; wait("SetConvert"); }
	int32_t k0 = int32_t(w1 >> 45) & 0x1ff;
	int32_t k1 = int32_t(w1 >> 36) & 0x1ff;
	int32_t k2 = int32_t(w1 >> 27) & 0x1ff;
	int32_t k3 = int32_t(w1 >> 18) & 0x1ff;
	int32_t k4 = int32_t(w1 >>  9) & 0x1ff;
	int32_t k5 = int32_t(w1 >>  0) & 0x1ff;

	k0 = (SIGN9(k0) << 1) + 1;
	k1 = (SIGN9(k1) << 1) + 1;
	k2 = (SIGN9(k2) << 1) + 1;
	k3 = (SIGN9(k3) << 1) + 1;

	set_yuv_factors(rgbaint_t(0, k0, k2, k3), rgbaint_t(0, 0, k1, 0), rgbaint_t(k4, k4, k4, k4), rgbaint_t(k5, k5, k5, k5));
}

void n64_rdp::cmd_set_scissor(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];

	m_scissor.m_xh = ((w1 >> 44) & 0xfff) >> 2;
	m_scissor.m_yh = ((w1 >> 32) & 0xfff) >> 2;
	m_scissor.m_xl = ((w1 >> 12) & 0xfff) >> 2;
	m_scissor.m_yl = ((w1 >>  0) & 0xfff) >> 2;

	// TODO: handle f & o?
}

void n64_rdp::cmd_set_prim_depth(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_misc_state.m_primitive_z = (uint32_t)(w1 & 0x7fff0000);
	m_misc_state.m_primitive_dz = (uint16_t)(w1 >> 32);
}

void n64_rdp::cmd_set_other_modes(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	//wait("SetOtherModes");
	m_other_modes.cycle_type       = (w1 >> 52) & 0x3; // 01
	m_other_modes.persp_tex_en     = (w1 >> 51) & 1; // 1
	m_other_modes.detail_tex_en    = (w1 >> 50) & 1; // 0
	m_other_modes.sharpen_tex_en   = (w1 >> 49) & 1; // 0
	m_other_modes.tex_lod_en       = (w1 >> 48) & 1; // 0
	m_other_modes.en_tlut          = (w1 >> 47) & 1; // 0
	m_other_modes.tlut_type        = (w1 >> 46) & 1; // 0
	m_other_modes.sample_type      = (w1 >> 45) & 1; // 1
	m_other_modes.mid_texel        = (w1 >> 44) & 1; // 0
	m_other_modes.bi_lerp0         = (w1 >> 43) & 1; // 1
	m_other_modes.bi_lerp1         = (w1 >> 42) & 1; // 1
	m_other_modes.convert_one      = (w1 >> 41) & 1; // 0
	m_other_modes.key_en           = (w1 >> 40) & 1; // 0
	m_other_modes.rgb_dither_sel   = (w1 >> 38) & 0x3; // 00
	m_other_modes.alpha_dither_sel = (w1 >> 36) & 0x3; // 01
	m_other_modes.blend_m1a_0      = (w1 >> 30) & 0x3; // 11
	m_other_modes.blend_m1a_1      = (w1 >> 28) & 0x3; // 00
	m_other_modes.blend_m1b_0      = (w1 >> 26) & 0x3; // 10
	m_other_modes.blend_m1b_1      = (w1 >> 24) & 0x3; // 00
	m_other_modes.blend_m2a_0      = (w1 >> 22) & 0x3; // 00
	m_other_modes.blend_m2a_1      = (w1 >> 20) & 0x3; // 01
	m_other_modes.blend_m2b_0      = (w1 >> 18) & 0x3; // 00
	m_other_modes.blend_m2b_1      = (w1 >> 16) & 0x3; // 01
	m_other_modes.force_blend      = (w1 >> 14) & 1; // 0
	m_other_modes.blend_shift      = m_other_modes.force_blend ? 5 : 2;
	m_other_modes.alpha_cvg_select = (w1 >> 13) & 1; // 1
	m_other_modes.cvg_times_alpha  = (w1 >> 12) & 1; // 0
	m_other_modes.z_mode           = (w1 >> 10) & 0x3; // 00
	m_other_modes.cvg_dest         = (w1 >> 8) & 0x3; // 00
	m_other_modes.color_on_cvg     = (w1 >> 7) & 1; // 0
	m_other_modes.image_read_en    = (w1 >> 6) & 1; // 1
	m_other_modes.z_update_en      = (w1 >> 5) & 1; // 1
	m_other_modes.z_compare_en     = (w1 >> 4) & 1; // 1
	m_other_modes.antialias_en     = (w1 >> 3) & 1; // 1
	m_other_modes.z_source_sel     = (w1 >> 2) & 1; // 0
	m_other_modes.dither_alpha_en  = (w1 >> 1) & 1; // 0
	m_other_modes.alpha_compare_en = (w1 >> 0) & 1; // 0
	m_other_modes.alpha_dither_mode = (m_other_modes.alpha_compare_en << 1) | m_other_modes.dither_alpha_en;
}

void n64_rdp::cmd_load_tlut(uint64_t *cmd_buf)
{
	//wait("LoadTLUT");
	n64_tile_t* tile = m_tiles;
	const uint64_t w1 = cmd_buf[0];

	const int32_t tilenum = (w1 >> 24) & 0x7;
	const int32_t sl = tile[tilenum].sl = int32_t(w1 >> 44) & 0xfff;
	const int32_t tl = tile[tilenum].tl = int32_t(w1 >> 32) & 0xfff;
	const int32_t sh = tile[tilenum].sh = int32_t(w1 >> 12) & 0xfff;
	const int32_t th = tile[tilenum].th = int32_t(w1 >>  0) & 0xfff;

	if (tl != th)
	{
		fatalerror("Load tlut: tl=%d, th=%d\n",tl,th);
	}

	m_capture.data_begin();

	const int32_t count = ((sh >> 2) - (sl >> 2) + 1) << 2;

	switch (m_misc_state.m_ti_size)
	{
		case PIXEL_SIZE_16BIT:
		{
			if (tile[tilenum].tmem < 256)
			{
				fatalerror("rdp_load_tlut: loading tlut into low half at %d qwords\n",tile[tilenum].tmem);
			}
			int32_t srcstart = (m_misc_state.m_ti_address + (tl >> 2) * (m_misc_state.m_ti_width << 1) + (sl >> 1)) >> 1;
			int32_t dststart = tile[tilenum].tmem << 2;
			uint16_t* dst = get_tmem16();

			for (int32_t i = 0; i < count; i += 4)
			{
				if (dststart < 2048)
				{
					dst[dststart] = U_RREADIDX16(srcstart);
					m_capture.data_block()->put16(dst[dststart]);
					dst[dststart + 1] = dst[dststart];
					dst[dststart + 2] = dst[dststart];
					dst[dststart + 3] = dst[dststart];
					dststart += 4;
					srcstart += 1;
				}
			}
			break;
		}
		default:    fatalerror("RDP: load_tlut: size = %d\n", m_misc_state.m_ti_size);
	}

	m_capture.data_end();

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_set_tile_size(uint64_t *cmd_buf)
{
	//wait("SetTileSize");
	const uint64_t w1 = cmd_buf[0];
	const int32_t tilenum = int32_t(w1 >> 24) & 0x7;

	m_tiles[tilenum].sl = int32_t(w1 >> 44) & 0xfff;
	m_tiles[tilenum].tl = int32_t(w1 >> 32) & 0xfff;
	m_tiles[tilenum].sh = int32_t(w1 >> 12) & 0xfff;
	m_tiles[tilenum].th = int32_t(w1 >>  0) & 0xfff;

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_load_block(uint64_t *cmd_buf)
{
	//wait("LoadBlock");
	n64_tile_t* tile = m_tiles;
	const uint64_t w1 = cmd_buf[0];

	const uint8_t tilenum = uint8_t(w1 >> 24) & 0x7;
	uint16_t* tc = get_tmem16();

	int32_t sl, tl, sh, dxt;
	tile[tilenum].sl =  sl = int32_t((w1 >> 44) & 0xfff);
	tile[tilenum].tl =  tl = int32_t((w1 >> 32) & 0xfff);
	tile[tilenum].sh =  sh = int32_t((w1 >> 12) & 0xfff);
	tile[tilenum].th = dxt = int32_t((w1 >>  0) & 0xfff);

	/*uint16_t tl_masked = tl & 0x3ff;

	int32_t load_edge_walker_data[10] = {
	    ((cmd_buf[0] >> 32) & 0xff000000) | (0x10 << 19) | (tilenum << 16) | ((tl_masked << 2) | 3),
	    (((tl_masked << 2) | 3) << 16) | (tl_masked << 2),
	    sh << 16,
	    sl << 16,
	    sh << 16,
	    ((sl << 3) << 16) | (tl << 3),
	    (dxt & 0xff) << 8,
	    ((0x80 >> wstate->ti_size) << 16) | (dxt >> 8),
	    0x20,
	    0x20
	};

	do_load_edge_walker(load_edge_walker_data);*/

	int32_t width = (sh - sl) + 1;

	width = (width << m_misc_state.m_ti_size) >> 1;
	if (width & 7)
	{
		width = (width & ~7) + 8;
	}
	width >>= 3;

	const int32_t tb = tile[tilenum].tmem << 2;

	const int32_t tiwinwords = (m_misc_state.m_ti_width << m_misc_state.m_ti_size) >> 2;
	const int32_t slinwords = (sl << m_misc_state.m_ti_size) >> 2;

	const uint32_t src = (m_misc_state.m_ti_address >> 1) + (tl * tiwinwords) + slinwords;

	m_capture.data_begin();

	if (dxt != 0)
	{
		int32_t j = 0;
		int32_t t = 0;
		int32_t oldt = 0;

		if (tile[tilenum].size != PIXEL_SIZE_32BIT && tile[tilenum].format != FORMAT_YUV)
		{
			for (int32_t i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				int32_t ptr = tb + (i << 2);
				int32_t srcptr = src + (i << 2);

				tc[(ptr ^ t) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 3);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));

				j += dxt;
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (int32_t i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				int32_t ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				int32_t srcptr = src + (i << 2);

				int32_t first = U_RREADIDX16(srcptr);
				int32_t sec = U_RREADIDX16(srcptr + 1);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				ptr = ((tb + (i << 1) + 1) ^ t) & 0x3ff;
				first = U_RREADIDX16(srcptr + 2);
				sec = U_RREADIDX16(srcptr + 3);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));
				j += dxt;
			}
		}
		else
		{
			for (int32_t i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
					i += tile[tilenum].line;

				int32_t ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				int32_t srcptr = src + (i << 2);
				tc[ptr] = U_RREADIDX16(srcptr);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 1);

				ptr = ((tb + (i << 1) + 1) ^ t) & 0x3ff;
				tc[ptr] = U_RREADIDX16(srcptr + 2);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 3);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));

				j += dxt;
			}
		}
		tile[tilenum].th = tl + (j >> 11);
	}
	else
	{
		if (tile[tilenum].size != PIXEL_SIZE_32BIT && tile[tilenum].format != FORMAT_YUV)
		{
			for (int32_t i = 0; i < width; i ++)
			{
				int32_t ptr = tb + (i << 2);
				int32_t srcptr = src + (i << 2);
				tc[(ptr ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 3);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (int32_t i = 0; i < width; i ++)
			{
				int32_t ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				int32_t srcptr = src + (i << 2);
				int32_t first = U_RREADIDX16(srcptr);
				int32_t sec = U_RREADIDX16(srcptr + 1);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);//UV pair
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				ptr = ((tb + (i << 1) + 1) ^ WORD_ADDR_XOR) & 0x3ff;
				first = U_RREADIDX16(srcptr + 2);
				sec = U_RREADIDX16(srcptr + 3);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));
			}
		}
		else
		{
			for (int32_t i = 0; i < width; i ++)
			{
				int32_t ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				int32_t srcptr = src + (i << 2);
				tc[ptr] = U_RREADIDX16(srcptr);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 1);

				ptr = ((tb + (i << 1) + 1) ^ WORD_ADDR_XOR) & 0x3ff;
				tc[ptr] = U_RREADIDX16(srcptr + 2);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 3);

				m_capture.data_block()->put16(U_RREADIDX16(srcptr));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+1));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+2));
				m_capture.data_block()->put16(U_RREADIDX16(srcptr+3));
			}
		}
		tile[tilenum].th = tl;
	}

	m_capture.data_end();

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_load_tile(uint64_t *cmd_buf)
{
	//wait("LoadTile");
	n64_tile_t* tile = m_tiles;
	const uint64_t w1 = cmd_buf[0];
	const int32_t tilenum = int32_t(w1 >> 24) & 0x7;

	tile[tilenum].sl    = int32_t(w1 >> 44) & 0xfff;
	tile[tilenum].tl    = int32_t(w1 >> 32) & 0xfff;
	tile[tilenum].sh    = int32_t(w1 >> 12) & 0xfff;
	tile[tilenum].th    = int32_t(w1 >>  0) & 0xfff;

	const int32_t sl = tile[tilenum].sl >> 2;
	const int32_t tl = tile[tilenum].tl >> 2;
	const int32_t sh = tile[tilenum].sh >> 2;
	const int32_t th = tile[tilenum].th >> 2;

	const int32_t width = (sh - sl) + 1;
	const int32_t height = (th - tl) + 1;
/*
    int32_t topad;
    if (m_misc_state.m_ti_size < 3)
    {
        topad = (width * m_misc_state.m_ti_size) & 0x7;
    }
    else
    {
        topad = (width << 2) & 0x7;
    }
    topad = 0; // ????
*/

	m_capture.data_begin();

	switch (m_misc_state.m_ti_size)
	{
		case PIXEL_SIZE_8BIT:
		{
			const uint32_t src = m_misc_state.m_ti_address;
			const int32_t tb = tile[tilenum].tmem << 3;
			uint8_t* tc = get_tmem8();

			for (int32_t j = 0; j < height; j++)
			{
				const int32_t tline = tb + ((tile[tilenum].line << 3) * j);
				const int32_t s = ((j + tl) * m_misc_state.m_ti_width) + sl;
				const int32_t xorval8 = ((j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);

				for (int32_t i = 0; i < width; i++)
				{
					const uint8_t data = U_RREADADDR8(src + s + i);
					m_capture.data_block()->put8(data);
					tc[((tline + i) ^ xorval8) & 0xfff] = data;
				}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			const uint32_t src = m_misc_state.m_ti_address >> 1;
			uint16_t* tc = get_tmem16();

			if (tile[tilenum].format != FORMAT_YUV)
			{
				for (int32_t j = 0; j < height; j++)
				{
					const int32_t tb = tile[tilenum].tmem << 2;
					const int32_t tline = tb + ((tile[tilenum].line << 2) * j);
					const int32_t s = ((j + tl) * m_misc_state.m_ti_width) + sl;
					const int32_t xorval16 = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;

					for (int32_t i = 0; i < width; i++)
					{
						const uint32_t taddr = (tline + i) ^ xorval16;
						const uint16_t data = U_RREADIDX16(src + s + i);
						m_capture.data_block()->put16(data);
						tc[taddr & 0x7ff] = data;
					}
				}
			}
			else
			{
				for (int32_t j = 0; j < height; j++)
				{
					const int32_t tb = tile[tilenum].tmem << 3;
					const int32_t tline = tb + ((tile[tilenum].line << 3) * j);
					const int32_t s = ((j + tl) * m_misc_state.m_ti_width) + sl;
					const int32_t xorval8 = (j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;

					for (int32_t i = 0; i < width; i++)
					{
						uint32_t taddr = ((tline + i) ^ xorval8) & 0x7ff;
						uint16_t yuvword = U_RREADIDX16(src + s + i);
						m_capture.data_block()->put16(yuvword);
						get_tmem8()[taddr] = yuvword >> 8;
						get_tmem8()[taddr | 0x800] = yuvword & 0xff;
					}
				}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			const uint32_t src = m_misc_state.m_ti_address >> 2;
			const int32_t tb = (tile[tilenum].tmem << 2);
			uint16_t* tc16 = get_tmem16();

			for (int32_t j = 0; j < height; j++)
			{
				const int32_t tline = tb + ((tile[tilenum].line << 2) * j);

				const int32_t s = ((j + tl) * m_misc_state.m_ti_width) + sl;
				const int32_t xorval32cur = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				for (int32_t i = 0; i < width; i++)
				{
					uint32_t c = U_RREADIDX32(src + s + i);
					m_capture.data_block()->put32(c);
					uint32_t ptr = ((tline + i) ^ xorval32cur) & 0x3ff;
					tc16[ptr] = c >> 16;
					tc16[ptr | 0x400] = c & 0xffff;
				}
			}
			break;
		}

		default:    fatalerror("RDP: load_tile: size = %d\n", m_misc_state.m_ti_size);
	}

	m_capture.data_end();

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_set_tile(uint64_t *cmd_buf)
{
	//wait("SetTile");
	const uint64_t w1 = cmd_buf[0];
	const int32_t tilenum = int32_t(w1 >> 24) & 0x7;
	n64_tile_t* tex_tile = &m_tiles[tilenum];

	tex_tile->format    = int32_t(w1 >> 53) & 0x7;
	tex_tile->size      = int32_t(w1 >> 51) & 0x3;
	tex_tile->line      = int32_t(w1 >> 41) & 0x1ff;
	tex_tile->tmem      = int32_t(w1 >> 32) & 0x1ff;
	tex_tile->palette   = int32_t(w1 >> 20) & 0xf;
	tex_tile->ct        = int32_t(w1 >> 19) & 0x1;
	tex_tile->mt        = int32_t(w1 >> 18) & 0x1;
	tex_tile->mask_t    = int32_t(w1 >> 14) & 0xf;
	tex_tile->shift_t   = int32_t(w1 >> 10) & 0xf;
	tex_tile->cs        = int32_t(w1 >>  9) & 0x1;
	tex_tile->ms        = int32_t(w1 >>  8) & 0x1;
	tex_tile->mask_s    = int32_t(w1 >>  4) & 0xf;
	tex_tile->shift_s   = int32_t(w1 >>  0) & 0xf;

	tex_tile->lshift_s  = (tex_tile->shift_s >= 11) ? (16 - tex_tile->shift_s) : 0;
	tex_tile->rshift_s  = (tex_tile->shift_s < 11) ? tex_tile->shift_s : 0;
	tex_tile->lshift_t  = (tex_tile->shift_t >= 11) ? (16 - tex_tile->shift_t) : 0;
	tex_tile->rshift_t  = (tex_tile->shift_t < 11) ? tex_tile->shift_t : 0;
	tex_tile->wrapped_mask_s = (tex_tile->mask_s > 10 ? 10 : tex_tile->mask_s);
	tex_tile->wrapped_mask_t = (tex_tile->mask_t > 10 ? 10 : tex_tile->mask_t);
	tex_tile->wrapped_mask = rgbaint_t(tex_tile->wrapped_mask_s, tex_tile->wrapped_mask_s, tex_tile->wrapped_mask_t, tex_tile->wrapped_mask_t);
	tex_tile->clamp_s = tex_tile->cs || !tex_tile->mask_s;
	tex_tile->clamp_t = tex_tile->ct || !tex_tile->mask_t;
	tex_tile->mm = rgbaint_t(tex_tile->ms ? ~0 : 0, tex_tile->ms ? ~0 : 0, tex_tile->mt ? ~0 : 0, tex_tile->mt ? ~0 : 0);
	tex_tile->invmm = rgbaint_t(tex_tile->ms ? 0 : ~0, tex_tile->ms ? 0 : ~0, tex_tile->mt ? 0 : ~0, tex_tile->mt ? 0 : ~0);
	tex_tile->mask = rgbaint_t(tex_tile->mask_s ? ~0 : 0, tex_tile->mask_s ? ~0 : 0, tex_tile->mask_t ? ~0 : 0, tex_tile->mask_t ? ~0 : 0);
	tex_tile->invmask = rgbaint_t(tex_tile->mask_s ? 0 : ~0, tex_tile->mask_s ? 0 : ~0, tex_tile->mask_t ? 0 : ~0, tex_tile->mask_t ? 0 : ~0);
	tex_tile->lshift = rgbaint_t(tex_tile->lshift_s, tex_tile->lshift_s, tex_tile->lshift_t, tex_tile->lshift_t);
	tex_tile->rshift = rgbaint_t(tex_tile->rshift_s, tex_tile->rshift_s, tex_tile->rshift_t, tex_tile->rshift_t);
	tex_tile->clamp_st = rgbaint_t(tex_tile->clamp_s ? ~0 : 0, tex_tile->clamp_s ? ~0 : 0, tex_tile->clamp_t ? ~0 : 0, tex_tile->clamp_t ? ~0 : 0);

	if (tex_tile->format == FORMAT_I && tex_tile->size > PIXEL_SIZE_8BIT)
	{
		tex_tile->format = FORMAT_RGBA; // Used by Supercross 2000 (in-game)
	}
	if (tex_tile->format == FORMAT_CI && tex_tile->size > PIXEL_SIZE_8BIT)
	{
		tex_tile->format = FORMAT_RGBA; // Used by Clay Fighter - Sculptor's Cut
	}

	if (tex_tile->format == FORMAT_RGBA && tex_tile->size < PIXEL_SIZE_16BIT)
	{
		tex_tile->format = FORMAT_CI; // Used by Exterem-G2, Madden Football 64, and Rat Attack
	}

	//m_pending_mode_block = true;
}

void n64_rdp::cmd_fill_rect(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	//if(m_pending_mode_block) { wait("Block on pending mode-change"); m_pending_mode_block = false; }
	const uint64_t xh = (w1 >> 12) & 0xfff;
	const uint64_t xl = (w1 >> 44) & 0xfff;
	const uint64_t yh = (w1 >>  0) & 0xfff;
	uint64_t yl       = (w1 >> 32) & 0xfff;

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const uint64_t xlint = (xl >> 2) & 0x3ff;
	const uint64_t xhint = (xh >> 2) & 0x3ff;

	uint64_t* ewdata = m_temp_rect_data;
	ewdata[0] = ((uint64_t)0x3680 << 48) | (yl << 32) | (yl << 16) | yh; // command, flipped, tile, yl, ym, yh
	ewdata[1] = (xlint << 48) | ((xl & 3) << 46); // xl, xl frac, dxldy (0), dxldy frac (0)
	ewdata[2] = (xhint << 48) | ((xh & 3) << 46); // xh, xh frac, dxhdy (0), dxhdy frac (0)
	ewdata[3] = (xlint << 48) | ((xl & 3) << 46); // xm, xm frac, dxmdy (0), dxmdy frac (0)
	memset(&ewdata[4], 0, 18 * sizeof(uint64_t));//shade, texture, depth

	draw_triangle(cmd_buf, false, false, false, true);
}

void n64_rdp::cmd_set_fog_color(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_fog_color.set(uint8_t(w1), uint8_t(w1 >> 24), uint8_t(w1 >> 16), uint8_t(w1 >> 8));
}

void n64_rdp::cmd_set_blend_color(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_blend_color.set(uint8_t(w1), uint8_t(w1 >> 24), uint8_t(w1 >> 16), uint8_t(w1 >> 8));
}

void n64_rdp::cmd_set_prim_color(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_misc_state.m_min_level = uint32_t(w1 >> 40) & 0x1f;
	const uint8_t prim_lod_fraction(w1 >> 32);
	m_prim_lod_fraction.set(prim_lod_fraction, prim_lod_fraction, prim_lod_fraction, prim_lod_fraction);

	const uint8_t alpha(w1);
	m_prim_color.set(alpha, uint8_t(w1 >> 24), uint8_t(w1 >> 16), uint8_t(w1 >> 8));
	m_prim_alpha.set(alpha, alpha, alpha, alpha);
}

void n64_rdp::cmd_set_env_color(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	const uint8_t alpha(w1);
	m_env_color.set(alpha, uint8_t(w1 >> 24), uint8_t(w1 >> 16), uint8_t(w1 >> 8));
	m_env_alpha.set(alpha, alpha, alpha, alpha);
}

void n64_rdp::cmd_set_combine(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_combine.sub_a_rgb0    = uint32_t(w1 >> 52) & 0xf;
	m_combine.mul_rgb0      = uint32_t(w1 >> 47) & 0x1f;
	m_combine.sub_a_a0      = uint32_t(w1 >> 44) & 0x7;
	m_combine.mul_a0        = uint32_t(w1 >> 41) & 0x7;
	m_combine.sub_a_rgb1    = uint32_t(w1 >> 37) & 0xf;
	m_combine.mul_rgb1      = uint32_t(w1 >> 32) & 0x1f;

	m_combine.sub_b_rgb0    = uint32_t(w1 >> 28) & 0xf;
	m_combine.sub_b_rgb1    = uint32_t(w1 >> 24) & 0xf;
	m_combine.sub_a_a1      = uint32_t(w1 >> 21) & 0x7;
	m_combine.mul_a1        = uint32_t(w1 >> 18) & 0x7;
	m_combine.add_rgb0      = uint32_t(w1 >> 15) & 0x7;
	m_combine.sub_b_a0      = uint32_t(w1 >> 12) & 0x7;
	m_combine.add_a0        = uint32_t(w1 >>  9) & 0x7;
	m_combine.add_rgb1      = uint32_t(w1 >>  6) & 0x7;
	m_combine.sub_b_a1      = uint32_t(w1 >>  3) & 0x7;
	m_combine.add_a1        = uint32_t(w1 >>  0) & 0x7;

	/*static const char *s_suba_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "NOISE", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
	static const char *s_subb_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYC", "K4", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
	static const char *s_mul_rgb[32] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYS", "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "LODF", "PLODF", "K5",
	    "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
	static const char *s_add_rgb[8] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "ZERO" };
	static const char *s_sub_a[16] = { "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "ONE", "ZERO" };
	static const char *s_mul_a[16] = { "LODF", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "PLODF", "ZERO" };
	printf("Cycle 0, Color: (%s - %s) * %s + %s\n", s_suba_rgb[m_combine.sub_a_rgb0], s_subb_rgb[m_combine.sub_b_rgb0], s_mul_rgb[m_combine.mul_rgb0], s_add_rgb[m_combine.add_rgb0]);
	printf("Cycle 0, Alpha: (%s - %s) * %s + %s\n", s_sub_a[m_combine.sub_a_a0], s_sub_a[m_combine.sub_b_a0], s_mul_a[m_combine.mul_a0], s_add_rgb[m_combine.add_a0]);
	printf("Cycle 1, Color: (%s - %s) * %s + %s\n", s_suba_rgb[m_combine.sub_a_rgb1], s_subb_rgb[m_combine.sub_b_rgb1], s_mul_rgb[m_combine.mul_rgb1], s_add_rgb[m_combine.add_rgb1]);
	printf("Cycle 1, Alpha: (%s - %s) * %s + %s\n\n", s_sub_a[m_combine.sub_a_a1], s_sub_a[m_combine.sub_b_a1], s_mul_a[m_combine.mul_a1], s_add_rgb[m_combine.add_a1]);*/
}

void n64_rdp::cmd_set_texture_image(uint64_t *cmd_buf)
{
	const uint64_t w1 = cmd_buf[0];
	m_misc_state.m_ti_format  = uint32_t(w1 >> 53) & 0x7;
	m_misc_state.m_ti_size    = uint32_t(w1 >> 51) & 0x3;
	m_misc_state.m_ti_width   = (uint32_t(w1 >> 32) & 0x3ff) + 1;
	m_misc_state.m_ti_address = uint32_t(w1) & 0x01ffffff;
}

void n64_rdp::cmd_set_mask_image(uint64_t *cmd_buf)
{
	//wait("SetMaskImage");
	const uint64_t w1 = cmd_buf[0];
	m_misc_state.m_zb_address = uint32_t(w1) & 0x01ffffff;
}

void n64_rdp::cmd_set_color_image(uint64_t *cmd_buf)
{
	//wait("SetColorImage");
	const uint64_t w1 = cmd_buf[0];
	m_misc_state.m_fb_format  = uint32_t(w1 >> 53) & 0x7;
	m_misc_state.m_fb_size    = uint32_t(w1 >> 51) & 0x3;
	m_misc_state.m_fb_width   = (uint32_t(w1 >> 32) & 0x3ff) + 1;
	m_misc_state.m_fb_address = uint32_t(w1) & 0x01ffffff;
}

/*****************************************************************************/

void n64_rdp::cmd_noop(uint64_t *cmd_buf)
{
	// Do nothing
}


void n64_rdp::process_command_list()
{
	int32_t length = m_end - m_current;

	if (length <= 0)
	{
		m_current = m_end;
		return;
	}

	//printf("length: %08x\n", (uint32_t)length); fflush(stdout);

	set_status(get_status() &~ DP_STATUS_FREEZE);

	uint64_t curr_cmd_buf[176];

	while (m_current < m_end)
	{
		uint32_t start = m_current;
		uint32_t buf_index = 0;
		curr_cmd_buf[buf_index++] = read_data(m_current & 0x1fffffff);
		uint8_t cmd = (curr_cmd_buf[0] >> 56) & 0x3f;

		if ((m_end - m_current) < s_rdp_command_length[cmd])
		{
			// Not enough data, continue waiting.
			break;
		}
		m_current += 8;

		while ((buf_index << 3) < s_rdp_command_length[cmd])
		{
			curr_cmd_buf[buf_index++] = read_data(m_current & 0x1fffffff);
			m_current += 8;
		}

		m_capture.command(&curr_cmd_buf[0], s_rdp_command_length[cmd] / 8);

		if (LOG_RDP_EXECUTION)
		{
			char string[4000];
			disassemble(curr_cmd_buf, string);

			fprintf(rdp_exec, "%08X: %08X%08X   %s\n", start, (uint32_t)(curr_cmd_buf[0] >> 32), (uint32_t)curr_cmd_buf[0], string);
			fflush(rdp_exec);
		}

		// execute the command
		switch(cmd)
		{
			case 0x00:  cmd_noop(curr_cmd_buf);           break;

			case 0x08:  triangle(curr_cmd_buf, false, false, false); break;
			case 0x09:  triangle(curr_cmd_buf, false, false,  true); break;
			case 0x0a:  triangle(curr_cmd_buf, false,  true, false); break;
			case 0x0b:  triangle(curr_cmd_buf, false,  true,  true); break;
			case 0x0c:  triangle(curr_cmd_buf,  true, false, false); break;
			case 0x0d:  triangle(curr_cmd_buf,  true, false,  true); break;
			case 0x0e:  triangle(curr_cmd_buf,  true,  true, false); break;
			case 0x0f:  triangle(curr_cmd_buf,  true,  true,  true); break;

			case 0x24:  cmd_tex_rect(curr_cmd_buf);       break;
			case 0x25:  cmd_tex_rect_flip(curr_cmd_buf);  break;

			case 0x26:  cmd_sync_load(curr_cmd_buf);      break;
			case 0x27:  cmd_sync_pipe(curr_cmd_buf);      break;
			case 0x28:  cmd_sync_tile(curr_cmd_buf);      break;
			case 0x29:  cmd_sync_full(curr_cmd_buf);      break;

			case 0x2a:  cmd_set_key_gb(curr_cmd_buf);     break;
			case 0x2b:  cmd_set_key_r(curr_cmd_buf);      break;

			case 0x2c:  cmd_set_convert(curr_cmd_buf);    break;
			case 0x3c:  cmd_set_combine(curr_cmd_buf);    break;
			case 0x2d:  cmd_set_scissor(curr_cmd_buf);    break;
			case 0x2e:  cmd_set_prim_depth(curr_cmd_buf); break;
			case 0x2f:  cmd_set_other_modes(curr_cmd_buf);break;

			case 0x30:  cmd_load_tlut(curr_cmd_buf);      break;
			case 0x33:  cmd_load_block(curr_cmd_buf);     break;
			case 0x34:  cmd_load_tile(curr_cmd_buf);      break;

			case 0x32:  cmd_set_tile_size(curr_cmd_buf);  break;
			case 0x35:  cmd_set_tile(curr_cmd_buf);       break;

			case 0x36:  cmd_fill_rect(curr_cmd_buf);      break;

			case 0x37:  cmd_set_fill_color32(curr_cmd_buf); break;
			case 0x38:  cmd_set_fog_color(curr_cmd_buf);  break;
			case 0x39:  cmd_set_blend_color(curr_cmd_buf);break;
			case 0x3a:  cmd_set_prim_color(curr_cmd_buf); break;
			case 0x3b:  cmd_set_env_color(curr_cmd_buf);  break;

			case 0x3d:  cmd_set_texture_image(curr_cmd_buf); break;
			case 0x3e:  cmd_set_mask_image(curr_cmd_buf);  break;
			case 0x3f:  cmd_set_color_image(curr_cmd_buf); break;
		}
	};
}

/*****************************************************************************/

n64_rdp::n64_rdp(n64_state &state, uint32_t* rdram, uint32_t* dmem) : poly_manager<uint32_t, rdp_poly_state, 8>(state.machine())
{
	ignore = false;
	dolog = false;

	m_rdram = rdram;
	m_dmem = dmem;

	m_aux_buf_ptr = 0;
	m_aux_buf = nullptr;
	m_pipe_clean = true;

	m_pending_mode_block = false;

	m_start = 0;
	m_end = 0;
	m_current = 0;
	m_status = 0x88;

	m_one.set(0xff, 0xff, 0xff, 0xff);
	m_zero.set(0, 0, 0, 0);

	m_tmem = nullptr;

	m_machine = nullptr;
	m_n64_periphs = nullptr;

	//memset(m_hidden_bits, 3, 8388608);

	m_prim_lod_fraction.set(0, 0, 0, 0);
	z_build_com_table();

	memset(m_temp_rect_data, 0, sizeof(uint32_t) * 0x1000);

	for (int32_t i = 0; i < 0x4000; i++)
	{
		uint32_t exponent = (i >> 11) & 7;
		uint32_t mantissa = i & 0x7ff;
		m_z_complete_dec_table[i] = ((mantissa << m_z_dec_table[exponent].shift) + m_z_dec_table[exponent].add) & 0x3fffff;
	}

	precalc_cvmask_derivatives();

	for(int32_t i = 0; i < 0x200; i++)
	{
		switch((i >> 7) & 3)
		{
		case 0:
		case 1:
			s_special_9bit_clamptable[i] = i & 0xff;
			break;
		case 2:
			s_special_9bit_clamptable[i] = 0xff;
			break;
		case 3:
			s_special_9bit_clamptable[i] = 0;
			break;
		}
	}

	for(int32_t i = 0; i < 32; i++)
	{
		m_replicated_rgba[i] = (i << 3) | ((i >> 2) & 7);
	}

	for(int32_t i = 0; i < 0x10000; i++)
	{
		m_dzpix_normalize[i] = (uint16_t)normalize_dzpix(i & 0xffff);
	}

	m_compute_cvg[0] = &n64_rdp::compute_cvg_noflip;
	m_compute_cvg[1] = &n64_rdp::compute_cvg_flip;

	m_write_pixel[0] = &n64_rdp::write_pixel4;
	m_write_pixel[1] = &n64_rdp::write_pixel8;
	m_write_pixel[2] = &n64_rdp::write_pixel16;
	m_write_pixel[3] = &n64_rdp::write_pixel32;

	m_read_pixel[0] = &n64_rdp::read_pixel4;
	m_read_pixel[1] = &n64_rdp::read_pixel8;
	m_read_pixel[2] = &n64_rdp::read_pixel16;
	m_read_pixel[3] = &n64_rdp::read_pixel32;

	m_copy_pixel[0] = &n64_rdp::copy_pixel4;
	m_copy_pixel[1] = &n64_rdp::copy_pixel8;
	m_copy_pixel[2] = &n64_rdp::copy_pixel16;
	m_copy_pixel[3] = &n64_rdp::copy_pixel32;

	m_fill_pixel[0] = &n64_rdp::fill_pixel4;
	m_fill_pixel[1] = &n64_rdp::fill_pixel8;
	m_fill_pixel[2] = &n64_rdp::fill_pixel16;
	m_fill_pixel[3] = &n64_rdp::fill_pixel32;
}

void n64_rdp::render_spans(int32_t start, int32_t end, int32_t tilenum, bool flip, extent_t* spans, bool rect, rdp_poly_state* object)
{
	const int32_t clipy1 = m_scissor.m_yh;
	const int32_t clipy2 = m_scissor.m_yl;
	const rectangle clip(m_scissor.m_xh, m_scissor.m_xl, m_scissor.m_yh, m_scissor.m_yl);

	int32_t offset = 0;

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
			render_extents<8>(clip, render_delegate(&n64_rdp::span_draw_1cycle, this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_2:
			render_extents<8>(clip, render_delegate(&n64_rdp::span_draw_2cycle, this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_COPY:
			render_extents<8>(clip, render_delegate(&n64_rdp::span_draw_copy, this), start, (end - start) + 1, spans + offset);
			break;

		case CYCLE_TYPE_FILL:
			render_extents<8>(clip, render_delegate(&n64_rdp::span_draw_fill, this), start, (end - start) + 1, spans + offset);
			break;
	}
	wait("render spans");
}

void n64_rdp::rgbaz_clip(int32_t sr, int32_t sg, int32_t sb, int32_t sa, int32_t* sz, rdp_span_aux* userdata)
{
	userdata->m_shade_color.set(sa, sr, sg, sb);
	userdata->m_shade_color.clamp_and_clear(0xfffffe00);
	uint32_t a = userdata->m_shade_color.get_a();
	userdata->m_shade_alpha.set(a, a, a, a);

	int32_t zanded = (*sz) & 0x60000;

	zanded >>= 17;
	switch(zanded)
	{
		case 0: *sz &= 0x3ffff;                                         break;
		case 1: *sz &= 0x3ffff;                                         break;
		case 2: *sz = 0x3ffff;                                          break;
		case 3: *sz = 0x3ffff;                                          break;
	}
}

void n64_rdp::rgbaz_correct_triangle(int32_t offx, int32_t offy, int32_t* r, int32_t* g, int32_t* b, int32_t* a, int32_t* z, rdp_span_aux* userdata, const rdp_poly_state &object)
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
		int32_t summand_xr = offx * SIGN13(object.m_span_base.m_span_dr >> 14);
		int32_t summand_yr = offy * SIGN13(object.m_span_base.m_span_drdy >> 14);
		int32_t summand_xb = offx * SIGN13(object.m_span_base.m_span_db >> 14);
		int32_t summand_yb = offy * SIGN13(object.m_span_base.m_span_dbdy >> 14);
		int32_t summand_xg = offx * SIGN13(object.m_span_base.m_span_dg >> 14);
		int32_t summand_yg = offy * SIGN13(object.m_span_base.m_span_dgdy >> 14);
		int32_t summand_xa = offx * SIGN13(object.m_span_base.m_span_da >> 14);
		int32_t summand_ya = offy * SIGN13(object.m_span_base.m_span_dady >> 14);

		int32_t summand_xz = offx * SIGN22(object.m_span_base.m_span_dz >> 10);
		int32_t summand_yz = offy * SIGN22(object.m_span_base.m_span_dzdy >> 10);

		*r = ((*r << 2) + summand_xr + summand_yr) >> 4;
		*g = ((*g << 2) + summand_xg + summand_yg) >> 4;
		*b = ((*b << 2) + summand_xb + summand_yb) >> 4;
		*a = ((*a << 2) + summand_xa + summand_ya) >> 4;
		*z = (((*z << 2) + summand_xz + summand_yz) >> 5) & 0x7ffff;
	}
}

void n64_rdp::write_pixel4(uint32_t curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	// Not yet implemented
#if DEBUG_RDP_PIXEL
	if (s_debug_drawing)
	{
		uint32_t y = curpixel / object.m_misc_state.m_fb_width;
		uint32_t x = curpixel % object.m_misc_state.m_fb_width;
		if (x == 157 && y == 89)
		{
			printf("Writing 4-bit final color: %08x\n", (uint32_t)color.to_rgba());
		}
	}
#endif
}

void n64_rdp::write_pixel8(uint32_t curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint8_t c = (color.get_r() & 0xf8) | ((color.get_g() & 0xf8) >> 5);
	if (c != 0)
		RWRITEADDR8(object.m_misc_state.m_fb_address + curpixel, c);

#if DEBUG_RDP_PIXEL
	if (s_debug_drawing)
	{
		uint32_t y = curpixel / object.m_misc_state.m_fb_width;
		uint32_t x = curpixel % object.m_misc_state.m_fb_width;
		if (x == 157 && y == 89)
		{
			printf("Writing 8-bit final color: %08x\n", (uint32_t)color.to_rgba());
		}
	}
#endif
}

void n64_rdp::write_pixel16(uint32_t curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint32_t fb = (object.m_misc_state.m_fb_address >> 1) + curpixel;

	uint16_t finalcolor;
	if (object.m_other_modes.color_on_cvg && !userdata->m_pre_wrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
	}
	else
	{
		color.shr_imm(3);
		finalcolor = (color.get_r() << 11) | (color.get_g() << 6) | (color.get_b() << 1);
	}

#if DEBUG_RDP_PIXEL
	if (s_debug_drawing)
	{
		uint32_t y = curpixel / object.m_misc_state.m_fb_width;
		uint32_t x = curpixel % object.m_misc_state.m_fb_width;
		if (x == 157 && y == 89)
		{
			printf("Writing 16-bit final color: %04x\n", finalcolor);
		}
	}
#endif

	switch (object.m_other_modes.cvg_dest)
	{
		case 0:
			if (userdata->m_blend_enable)
			{
				uint32_t finalcvg = userdata->m_current_pix_cvg + userdata->m_current_mem_cvg;
				if (finalcvg & 8)
				{
					finalcvg = 7;
				}
				RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
				HWRITEADDR8(fb, finalcvg & 3);
			}
			else
			{
				const uint32_t finalcvg = (userdata->m_current_pix_cvg - 1) & 7;
				RWRITEIDX16(fb, finalcolor | (finalcvg >> 2));
				HWRITEADDR8(fb, finalcvg & 3);
			}
			break;
		case 1:
		{
			const uint32_t finalcvg = (userdata->m_current_pix_cvg + userdata->m_current_mem_cvg) & 7;
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

void n64_rdp::write_pixel32(uint32_t curpixel, color_t& color, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint32_t fb = (object.m_misc_state.m_fb_address >> 2) + curpixel;

	uint32_t finalcolor;
	if (object.m_other_modes.color_on_cvg && !userdata->m_pre_wrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
	}
	else
	{
		finalcolor = (color.get_r() << 24) | (color.get_g() << 16) | (color.get_b() << 8);
	}

#if DEBUG_RDP_PIXEL
	if (s_debug_drawing)
	{
		uint32_t y = curpixel / object.m_misc_state.m_fb_width;
		uint32_t x = curpixel % object.m_misc_state.m_fb_width;
		if (x == 157 && y == 89)
		{
			printf("Writing 32-bit final color: %08x\n", finalcolor);
		}
	}
#endif

	switch (object.m_other_modes.cvg_dest)
	{
		case 0:
			if (userdata->m_blend_enable)
			{
				uint32_t finalcvg = userdata->m_current_pix_cvg + userdata->m_current_mem_cvg;
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

void n64_rdp::read_pixel4(uint32_t curpixel, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	userdata->m_memory_color.set(0, 0, 0, 0);
	userdata->m_current_mem_cvg = 7;
}

void n64_rdp::read_pixel8(uint32_t curpixel, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint8_t fbyte = RREADADDR8(object.m_misc_state.m_fb_address + curpixel);
	const uint8_t r8 = (fbyte & 0xf8) | (fbyte >> 5);
	uint8_t g8 = (fbyte & 0x07);
	g8 |= g8 << 3;
	g8 |= g8 << 6;
	userdata->m_memory_color.set(0, r8, g8, 0);
	userdata->m_memory_color.set_a(0xff);
	userdata->m_current_mem_cvg = 7;
}

void n64_rdp::read_pixel16(uint32_t curpixel, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint16_t fword = RREADIDX16((object.m_misc_state.m_fb_address >> 1) + curpixel);

	userdata->m_memory_color.set(0, GETHICOL(fword), GETMEDCOL(fword), GETLOWCOL(fword));
	if (object.m_other_modes.image_read_en)
	{
		uint8_t hbyte = HREADADDR8((object.m_misc_state.m_fb_address >> 1) + curpixel);
		userdata->m_memory_color.set_a(userdata->m_current_mem_cvg << 5);
		userdata->m_current_mem_cvg = ((fword & 1) << 2) | (hbyte & 3);
	}
	else
	{
		userdata->m_memory_color.set_a(0xff);
		userdata->m_current_mem_cvg = 7;
	}
}

void n64_rdp::read_pixel32(uint32_t curpixel, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	const uint32_t mem = RREADIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel);
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

void n64_rdp::copy_pixel4(uint32_t curpixel, color_t& color, const rdp_poly_state &object)
{
	// Not yet implemented
}

void n64_rdp::copy_pixel8(uint32_t curpixel, color_t& color, const rdp_poly_state &object)
{
	const uint8_t c = (color.get_r() & 0xf8) | ((color.get_g() & 0xf8) >> 5);
	if (c != 0)
		RWRITEADDR8(object.m_misc_state.m_fb_address + curpixel, c);
}

void n64_rdp::copy_pixel16(uint32_t curpixel, color_t& color, const rdp_poly_state &object)
{
	const uint32_t current_pix_cvg = color.get_a() ? 7 : 0;
	const uint8_t r = color.get_r(); // Vectorize me
	const uint8_t g = color.get_g();
	const uint8_t b = color.get_b();
	RWRITEIDX16((object.m_misc_state.m_fb_address >> 1) + curpixel, ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((current_pix_cvg >> 2) & 1));
	HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + curpixel, current_pix_cvg & 3);
}

void n64_rdp::copy_pixel32(uint32_t curpixel, color_t& color, const rdp_poly_state &object)
{
	const uint32_t current_pix_cvg = color.get_a() ? 7 : 0;
	const uint8_t r = color.get_r(); // Vectorize me
	const uint8_t g = color.get_g();
	const uint8_t b = color.get_b();
	RWRITEIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel, (r << 24) | (g << 16) | (b << 8) | (current_pix_cvg << 5));
}

void n64_rdp::fill_pixel4(uint32_t curpixel, const rdp_poly_state &object)
{
	// Not yet implemented
}

void n64_rdp::fill_pixel8(uint32_t curpixel, const rdp_poly_state &object)
{
	const uint8_t byte_shift = ((curpixel & 3) ^ BYTE_ADDR_XOR) << 3;
	RWRITEADDR8(object.m_misc_state.m_fb_address + curpixel, (uint8_t)(object.m_fill_color >> byte_shift));
}

void n64_rdp::fill_pixel16(uint32_t curpixel, const rdp_poly_state &object)
{
	uint16_t val;
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

void n64_rdp::fill_pixel32(uint32_t curpixel, const rdp_poly_state &object)
{
	RWRITEIDX32((object.m_misc_state.m_fb_address >> 2) + curpixel, object.m_fill_color);
	HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + (curpixel << 1), (object.m_fill_color & 0x10000) ? 3 : 0);
	HWRITEADDR8((object.m_misc_state.m_fb_address >> 1) + (curpixel << 1) + 1, (object.m_fill_color & 0x1) ? 3 : 0);
}

void n64_rdp::span_draw_1cycle(int32_t scanline, const extent_t &extent, const rdp_poly_state &object, int32_t threadid)
{
	assert(object.m_misc_state.m_fb_size < 4);

	const int32_t clipx1 = object.m_scissor.m_xh;
	const int32_t clipx2 = object.m_scissor.m_xl;
	const int32_t tilenum = object.tilenum;
	const bool flip = object.flip;

	span_param_t r; r.w = extent.param[SPAN_R].start;
	span_param_t g; g.w = extent.param[SPAN_G].start;
	span_param_t b; b.w = extent.param[SPAN_B].start;
	span_param_t a; a.w = extent.param[SPAN_A].start;
	span_param_t z; z.w = extent.param[SPAN_Z].start;
	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;
	span_param_t w; w.w = extent.param[SPAN_W].start;

	const uint32_t zb = object.m_misc_state.m_zb_address >> 1;
	const uint32_t zhb = object.m_misc_state.m_zb_address;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;

	m_tex_pipe.calculate_clamp_diffs(tilenum, userdata, object);

	const bool partialreject = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[0] == &userdata->m_pixel_color);
	const int32_t sel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color) ? 1 : 0;

	int32_t drinc, dginc, dbinc, dainc;
	int32_t dzinc, dzpix;
	int32_t dsinc, dtinc, dwinc;
	int32_t xinc;

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

	const int32_t fb_index = object.m_misc_state.m_fb_width * scanline;

	const int32_t xstart = extent.startx;
	const int32_t xend = userdata->m_unscissored_rx;
	const int32_t xend_scissored = extent.stopx;

	int32_t x = xend;

	const int32_t length = flip ? (xstart - xend) : (xend - xstart);

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

	if (object.m_misc_state.m_fb_size > 4)
		fatalerror("unsupported m_fb_size %d\n", object.m_misc_state.m_fb_size);

	const int32_t blend_index = (object.m_other_modes.alpha_cvg_select ? 2 : 0) | ((object.m_other_modes.rgb_dither_sel < 3) ? 1 : 0);
	const int32_t cycle0 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp0 & 1);

	int32_t sss = 0;
	int32_t sst = 0;

	if (object.m_other_modes.persp_tex_en)
	{
		tc_div(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		tc_div_no_perspective(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (int32_t j = 0; j <= length; j++)
	{
		int32_t sr = r.w >> 14;
		int32_t sg = g.w >> 14;
		int32_t sb = b.w >> 14;
		int32_t sa = a.w >> 14;
		int32_t sz = (z.w >> 10) & 0x3fffff;
		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			uint8_t offx, offy;
			lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			m_tex_pipe.lod_1cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, userdata, object);

			rgbaz_correct_triangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			rgbaz_clip(sr, sg, sb, sa, &sz, userdata);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tilenum, 0, userdata, object/*, false*/);
			uint32_t t0a = userdata->m_texel0_color.get_a();
			userdata->m_texel0_alpha.set(t0a, t0a, t0a, t0a);
			userdata->m_texel1_color = userdata->m_texel0_color;
			userdata->m_texel1_alpha = userdata->m_texel0_alpha;

			const uint8_t noise = machine().rand() << 3; // Not accurate
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

			const uint32_t curpixel = fb_index + x;
			const uint32_t zbcur = zb + curpixel;
			const uint32_t zhbcur = zhb + curpixel;

			((this)->*(m_read_pixel[object.m_misc_state.m_fb_size]))(curpixel, userdata, object);

#if DEBUG_RDP_PIXEL
			if (s_debug_drawing)
			{
				//uint32_t x = curpixel % m_n64_periphs->vi_width;
				//uint32_t y = curpixel / m_n64_periphs->vi_width;
				//printf("%d, %d  ", x, scanline);
				if (x == 157 && scanline == 89)
				{
					if (true)//finalcolor == 0)
					{
						static const char *s_fb_format[4] = { "I", "IA", "CI", "RGBA" };
						static const char *s_blend1a_c0[4] = { "PIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend1b_c0[4] = { "PIXA", "FOGA", "SHADEA", "ZERO" };
						static const char *s_blend2a_c0[4] = { "PIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend2b_c0[4] = { "INVPIXA", "MEMA", "ONE", "ZERO" };
						static const char *s_blend1a_c1[4] = { "BPIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend1b_c1[4] = { "PIXA", "FOGA", "SHADEA", "ZERO" };
						static const char *s_blend2a_c1[4] = { "BPIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend2b_c1[4] = { "INVPIXA", "MEMA", "ONE", "ZERO" };
						static const char *s_suba_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "NOISE", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_subb_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYC", "K4", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_mul_rgb[32] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYS", "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "LODF", "PLODF", "K5",
							"ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_add_rgb[8] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "ZERO" };
						static const char *s_sub_a[16] = { "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "ONE", "ZERO" };
						static const char *s_mul_a[16] = { "LODF", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "PLODF", "ZERO" };

						printf("Write to %08x: %d, %d\n", curpixel, x, scanline);
						printf("m_fb_size: %d\n", 4 << object.m_misc_state.m_fb_size);
						printf("m_fb_format: %s\n", s_fb_format[object.m_misc_state.m_fb_format]);
						printf("blend enable: %d\n", userdata->m_blend_enable);
						printf("other modes:\n");
						printf("    cycle_type: %d\n", object.m_other_modes.cycle_type);
						printf("    persp_tex_en: %d\n", object.m_other_modes.persp_tex_en);
						printf("    detail_tex_en: %d\n", object.m_other_modes.detail_tex_en);
						printf("    sharpen_tex_en: %d\n", object.m_other_modes.sharpen_tex_en);
						printf("    tex_lod_en: %d\n", object.m_other_modes.tex_lod_en);
						printf("    en_tlut: %d\n", object.m_other_modes.en_tlut);
						printf("    tlut_type: %d\n", object.m_other_modes.tlut_type);
						printf("    sample_type: %d\n", object.m_other_modes.sample_type);
						printf("    mid_texel: %d\n", object.m_other_modes.mid_texel);
						printf("    bi_lerp0: %d\n", object.m_other_modes.bi_lerp0);
						printf("    bi_lerp1: %d\n", object.m_other_modes.bi_lerp1);
						printf("    convert_one: %d\n", object.m_other_modes.convert_one);
						printf("    key_en: %d\n", object.m_other_modes.key_en);
						printf("    rgb_dither_sel: %d\n", object.m_other_modes.rgb_dither_sel);
						printf("    alpha_dither_sel: %d\n", object.m_other_modes.alpha_dither_sel);
						printf("    blend_m1a_0 (A Cycle 0, 1): %s\n", s_blend1a_c0[object.m_other_modes.blend_m1a_0]);
						printf("    blend_m1a_1 (A Cycle 1, 1): %s\n", s_blend1a_c1[object.m_other_modes.blend_m1a_1]);
						printf("    blend_m1b_0 (B Cycle 0, 1): %s\n", s_blend1b_c0[object.m_other_modes.blend_m1b_0]);
						printf("    blend_m1b_1 (B Cycle 1, 1): %s\n", s_blend1b_c1[object.m_other_modes.blend_m1b_1]);
						printf("    blend_m2a_0 (A Cycle 0, 2): %s\n", s_blend2a_c0[object.m_other_modes.blend_m2a_0]);
						printf("    blend_m2a_1 (A Cycle 1, 2): %s\n", s_blend2a_c1[object.m_other_modes.blend_m2a_1]);
						printf("    blend_m2b_0 (B Cycle 0, 2): %s\n", s_blend2b_c0[object.m_other_modes.blend_m2b_0]);
						printf("    blend_m2b_1 (B Cycle 1, 2): %s\n", s_blend2b_c1[object.m_other_modes.blend_m2b_1]);
						printf("    tex_edge: %d\n", object.m_other_modes.tex_edge);
						printf("    force_blend: %d\n", object.m_other_modes.force_blend);
						printf("    blend_shift: %d\n", object.m_other_modes.blend_shift);
						printf("    alpha_cvg_select: %d\n", object.m_other_modes.alpha_cvg_select);
						printf("    cvg_times_alpha: %d\n", object.m_other_modes.cvg_times_alpha);
						printf("    z_mode: %d\n", object.m_other_modes.z_mode);
						printf("    cvg_dest: %d\n", object.m_other_modes.cvg_dest);
						printf("    color_on_cvg: %d\n", object.m_other_modes.color_on_cvg);
						printf("    image_read_en: %d\n", object.m_other_modes.image_read_en);
						printf("    z_update_en: %d\n", object.m_other_modes.z_update_en);
						printf("    z_compare_en: %d\n", object.m_other_modes.z_compare_en);
						printf("    antialias_en: %d\n", object.m_other_modes.antialias_en);
						printf("    z_source_sel: %d\n", object.m_other_modes.z_source_sel);
						printf("    dither_alpha_en: %d\n", object.m_other_modes.dither_alpha_en);
						printf("    alpha_compare_en: %d\n", object.m_other_modes.alpha_compare_en);
						printf("    alpha_dither_mode: %d\n", object.m_other_modes.alpha_dither_mode);
						printf("combine:\n");
						printf("    RGB sub A, cycle 0: %s\n", s_suba_rgb[m_combine.sub_a_rgb0]);
						printf("    RGB sub B, cycle 0: %s\n", s_subb_rgb[m_combine.sub_b_rgb0]);
						printf("    RGB mul, cycle 0: %s\n", s_mul_rgb[m_combine.mul_rgb0]);
						printf("    RGB add, cycle 0: %s\n", s_add_rgb[m_combine.add_rgb0]);
						printf("    Alpha sub A, cycle 0: %s\n", s_sub_a[m_combine.sub_a_a0]);
						printf("    Alpha sub B, cycle 0: %s\n", s_sub_a[m_combine.sub_b_a0]);
						printf("    Alpha mul, cycle 0: %s\n", s_mul_a[m_combine.mul_a0]);
						printf("    Alpha add, cycle 0: %s\n\n", s_add_rgb[m_combine.add_a0]);
						printf("    RGB sub A, cycle 1: %s\n", s_suba_rgb[m_combine.sub_a_rgb1]);
						printf("    RGB sub B, cycle 1: %s\n", s_subb_rgb[m_combine.sub_b_rgb1]);
						printf("    RGB mul, cycle 1: %s\n", s_mul_rgb[m_combine.mul_rgb1]);
						printf("    RGB add, cycle 1: %s\n", s_add_rgb[m_combine.add_rgb1]);
						printf("    Alpha sub A, cycle 1: %s\n", s_sub_a[m_combine.sub_a_a1]);
						printf("    Alpha sub B, cycle 1: %s\n", s_sub_a[m_combine.sub_b_a1]);
						printf("    Alpha mul, cycle 1: %s\n", s_mul_a[m_combine.mul_a1]);
						printf("    Alpha add, cycle 1: %s\n\n", s_add_rgb[m_combine.add_a1]);
						printf("Texel 0: %08x\n", (uint32_t)userdata->m_texel0_color.to_rgba());
						printf("Texel 1: %08x\n", (uint32_t)userdata->m_texel1_color.to_rgba());
						printf("Env: %08x\n", (uint32_t)userdata->m_env_color.to_rgba());
						printf("Prim: %08x\n", (uint32_t)userdata->m_prim_color.to_rgba());
						printf("Mem: %08x\n", (uint32_t)userdata->m_memory_color.to_rgba());
						printf("Shade: %08x\n", (uint32_t)userdata->m_shade_color.to_rgba());
						printf("sargb: %08x, %08x, %08x, %08x\n", (uint32_t)sa, (uint32_t)sr, (uint32_t)sg, (uint32_t)sb);

						printf("Blend index: %d\n", (userdata->m_blend_enable << 2) | blend_index);
						int32_t cdith = 0;
						int32_t adith = 0;
						get_dither_values(scanline, j, &cdith, &adith, object);
						color_t reblended_pixel;
						((&m_blender)->*(m_blender.blend1[(userdata->m_blend_enable << 2) | blend_index]))(reblended_pixel, cdith, adith, partialreject, sel0, userdata, object/*, true*/);

						//((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tilenum, 0, userdata, object/*, true*/);
					}
				}
			}
#endif

			if (z_compare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				int32_t cdith = 0;
				int32_t adith = 0;
				get_dither_values(scanline, j, &cdith, &adith, object);

				color_t blended_pixel;
				bool rendered = ((&m_blender)->*(m_blender.blend1[(userdata->m_blend_enable << 2) | blend_index]))(blended_pixel, cdith, adith, partialreject, sel0, userdata, object/*, false*/);

				if (rendered)
				{
#if DEBUG_RDP_PIXEL
					if (x == 157 && scanline == 89 && s_debug_drawing)
					{
						printf("WRITE1: %08x\n", (uint32_t)blended_pixel.to_rgba());
					}
#endif
					((this)->*(m_write_pixel[object.m_misc_state.m_fb_size]))(curpixel, blended_pixel, userdata, object);
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

void n64_rdp::span_draw_2cycle(int32_t scanline, const extent_t &extent, const rdp_poly_state &object, int32_t threadid)
{
	assert(object.m_misc_state.m_fb_size < 4);

	const int32_t clipx1 = object.m_scissor.m_xh;
	const int32_t clipx2 = object.m_scissor.m_xl;
	const int32_t tilenum = object.tilenum;
	const bool flip = object.flip;

	span_param_t r; r.w = extent.param[SPAN_R].start;
	span_param_t g; g.w = extent.param[SPAN_G].start;
	span_param_t b; b.w = extent.param[SPAN_B].start;
	span_param_t a; a.w = extent.param[SPAN_A].start;
	span_param_t z; z.w = extent.param[SPAN_Z].start;
	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;
	span_param_t w; w.w = extent.param[SPAN_W].start;

	const uint32_t zb = object.m_misc_state.m_zb_address >> 1;
	const uint32_t zhb = object.m_misc_state.m_zb_address;

	int32_t tile2 = (tilenum + 1) & 7;
	int32_t tile1 = tilenum;
	const uint32_t prim_tile = tilenum;

	int32_t newtile1 = tile1;
	int32_t news = 0;
	int32_t newt = 0;

#ifdef PTR64
	assert(extent.userdata != (const void *)0xcccccccccccccccc);
#else
	assert(extent.userdata != (const void *)0xcccccccc);
#endif
	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;

	m_tex_pipe.calculate_clamp_diffs(tile1, userdata, object);

	bool partialreject = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[1] == &userdata->m_pixel_color);
	int32_t sel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color) ? 1 : 0;
	int32_t sel1 = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_memory_color) ? 1 : 0;

	int32_t drinc, dginc, dbinc, dainc;
	int32_t dzinc, dzpix;
	int32_t dsinc, dtinc, dwinc;
	int32_t xinc;

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

	const int32_t fb_index = object.m_misc_state.m_fb_width * scanline;

	int32_t cdith = 0;
	int32_t adith = 0;

	const int32_t xstart = extent.startx;
	const int32_t xend = userdata->m_unscissored_rx;
	const int32_t xend_scissored = extent.stopx;

	int32_t x = xend;

	const int32_t length = flip ? (xstart - xend) : (xend - xstart);

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

	if (object.m_misc_state.m_fb_size > 4)
		fatalerror("unsupported m_fb_size %d\n", object.m_misc_state.m_fb_size);

	const int32_t blend_index = (object.m_other_modes.alpha_cvg_select ? 2 : 0) | ((object.m_other_modes.rgb_dither_sel < 3) ? 1 : 0);
	const int32_t cycle0 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp0 & 1);
	const int32_t cycle1 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp1 & 1);

	int32_t sss = 0;
	int32_t sst = 0;

	if (object.m_other_modes.persp_tex_en)
	{
		tc_div(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}
	else
	{
		tc_div_no_perspective(s.w >> 16, t.w >> 16, w.w >> 16, &sss, &sst);
	}

	userdata->m_start_span = true;
	for (int32_t j = 0; j <= length; j++)
	{
		int32_t sr = r.w >> 14;
		int32_t sg = g.w >> 14;
		int32_t sb = b.w >> 14;
		int32_t sa = a.w >> 14;
		int32_t sz = (z.w >> 10) & 0x3fffff;

		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			const uint32_t compidx = m_compressed_cvmasks[userdata->m_cvg[x]];
			userdata->m_current_pix_cvg = cvarray[compidx].cvg;
			userdata->m_current_cvg_bit = cvarray[compidx].cvbit;
			const uint8_t offx = cvarray[compidx].xoff;
			const uint8_t offy = cvarray[compidx].yoff;
			//lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

			m_tex_pipe.lod_2cycle(&sss, &sst, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2, userdata, object);

			news = userdata->m_precomp_s;
			newt = userdata->m_precomp_t;
			m_tex_pipe.lod_2cycle_limited(&news, &newt, s.w + dsinc, t.w + dtinc, w.w + dwinc, dsinc, dtinc, dwinc, prim_tile, &newtile1, object);

			rgbaz_correct_triangle(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);
			rgbaz_clip(sr, sg, sb, sa, &sz, userdata);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tile1, 0, userdata, object/*, false*/);
			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle1]))(&userdata->m_texel1_color, &userdata->m_texel0_color, sss, sst, tile2, 1, userdata, object/*, false*/);

			uint32_t t0a = userdata->m_texel0_color.get_a();
			uint32_t t1a = userdata->m_texel1_color.get_a();
			uint32_t tna = userdata->m_next_texel_color.get_a();
			userdata->m_texel0_alpha.set(t0a, t0a, t0a, t0a);
			userdata->m_texel1_alpha.set(t1a, t1a, t1a, t1a);
			userdata->m_next_texel_alpha.set(tna, tna, tna, tna);

			const uint8_t noise = machine().rand() << 3; // Not accurate
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

			rgbaint_t temp_color(userdata->m_texel0_color);
			userdata->m_texel0_color = userdata->m_texel1_color;
			userdata->m_texel1_color = temp_color;

			uint32_t ca = userdata->m_combined_color.get_a();
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

			const uint32_t curpixel = fb_index + x;
			const uint32_t zbcur = zb + curpixel;
			const uint32_t zhbcur = zhb + curpixel;

			((this)->*(m_read_pixel[object.m_misc_state.m_fb_size]))(curpixel, userdata, object);

#if DEBUG_RDP_PIXEL
			if (s_debug_drawing)
			{
				//uint32_t x = curpixel % m_n64_periphs->vi_width;
				//uint32_t y = curpixel / m_n64_periphs->vi_width;
				//printf("%d, %d  ", x, scanline);
				if (x == 157 && scanline == 89)
				{
					if (true)//finalcolor == 0)
					{
						static const char *s_fb_format[4] = { "I", "IA", "CI", "RGBA" };
						static const char *s_blend1a_c0[4] = { "PIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend1b_c0[4] = { "PIXA", "FOGA", "SHADEA", "ZERO" };
						static const char *s_blend2a_c0[4] = { "PIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend2b_c0[4] = { "INVPIXA", "MEMA", "ONE", "ZERO" };
						static const char *s_blend1a_c1[4] = { "BPIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend1b_c1[4] = { "PIXA", "FOGA", "SHADEA", "ZERO" };
						static const char *s_blend2a_c1[4] = { "BPIXC", "MEMC", "BLENDC", "FOGC" };
						static const char *s_blend2b_c1[4] = { "INVPIXA", "MEMA", "ONE", "ZERO" };
						static const char *s_suba_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "NOISE", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_subb_rgb[16] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYC", "K4", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_mul_rgb[32] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "KEYS", "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "LODF", "PLODF", "K5",
							"ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO", "ZERO" };
						static const char *s_add_rgb[8] = { "Combined", "TEX0C", "TEX1C", "PRIMC", "SHADEC", "ENVC", "ONE", "ZERO" };
						static const char *s_sub_a[16] = { "CombinedA", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "ONE", "ZERO" };
						static const char *s_mul_a[16] = { "LODF", "TEX0A", "TEX1A", "PRIMA", "SHADEA", "ENVA", "PLODF", "ZERO" };

						printf("Write to %08x: %d, %d\n", curpixel, x, scanline);
						printf("m_fb_size: %d\n", 4 << object.m_misc_state.m_fb_size);
						printf("m_fb_format: %s\n", s_fb_format[object.m_misc_state.m_fb_format]);
						printf("blend enable: %d\n", userdata->m_blend_enable);
						printf("other modes:\n");
						printf("    cycle_type: %d\n", object.m_other_modes.cycle_type);
						printf("    persp_tex_en: %d\n", object.m_other_modes.persp_tex_en);
						printf("    detail_tex_en: %d\n", object.m_other_modes.detail_tex_en);
						printf("    sharpen_tex_en: %d\n", object.m_other_modes.sharpen_tex_en);
						printf("    tex_lod_en: %d\n", object.m_other_modes.tex_lod_en);
						printf("    en_tlut: %d\n", object.m_other_modes.en_tlut);
						printf("    tlut_type: %d\n", object.m_other_modes.tlut_type);
						printf("    sample_type: %d\n", object.m_other_modes.sample_type);
						printf("    mid_texel: %d\n", object.m_other_modes.mid_texel);
						printf("    bi_lerp0: %d\n", object.m_other_modes.bi_lerp0);
						printf("    bi_lerp1: %d\n", object.m_other_modes.bi_lerp1);
						printf("    convert_one: %d\n", object.m_other_modes.convert_one);
						printf("    key_en: %d\n", object.m_other_modes.key_en);
						printf("    rgb_dither_sel: %d\n", object.m_other_modes.rgb_dither_sel);
						printf("    alpha_dither_sel: %d\n", object.m_other_modes.alpha_dither_sel);
						printf("    blend_m1a_0 (A Cycle 0, 1): %s\n", s_blend1a_c0[object.m_other_modes.blend_m1a_0]);
						printf("    blend_m1a_1 (A Cycle 1, 1): %s\n", s_blend1a_c1[object.m_other_modes.blend_m1a_1]);
						printf("    blend_m1b_0 (B Cycle 0, 1): %s\n", s_blend1b_c0[object.m_other_modes.blend_m1b_0]);
						printf("    blend_m1b_1 (B Cycle 1, 1): %s\n", s_blend1b_c1[object.m_other_modes.blend_m1b_1]);
						printf("    blend_m2a_0 (A Cycle 0, 2): %s\n", s_blend2a_c0[object.m_other_modes.blend_m2a_0]);
						printf("    blend_m2a_1 (A Cycle 1, 2): %s\n", s_blend2a_c1[object.m_other_modes.blend_m2a_1]);
						printf("    blend_m2b_0 (B Cycle 0, 2): %s\n", s_blend2b_c0[object.m_other_modes.blend_m2b_0]);
						printf("    blend_m2b_1 (B Cycle 1, 2): %s\n", s_blend2b_c1[object.m_other_modes.blend_m2b_1]);
						printf("    tex_edge: %d\n", object.m_other_modes.tex_edge);
						printf("    force_blend: %d\n", object.m_other_modes.force_blend);
						printf("    blend_shift: %d\n", object.m_other_modes.blend_shift);
						printf("    alpha_cvg_select: %d\n", object.m_other_modes.alpha_cvg_select);
						printf("    cvg_times_alpha: %d\n", object.m_other_modes.cvg_times_alpha);
						printf("    z_mode: %d\n", object.m_other_modes.z_mode);
						printf("    cvg_dest: %d\n", object.m_other_modes.cvg_dest);
						printf("    color_on_cvg: %d\n", object.m_other_modes.color_on_cvg);
						printf("    image_read_en: %d\n", object.m_other_modes.image_read_en);
						printf("    z_update_en: %d\n", object.m_other_modes.z_update_en);
						printf("    z_compare_en: %d\n", object.m_other_modes.z_compare_en);
						printf("    antialias_en: %d\n", object.m_other_modes.antialias_en);
						printf("    z_source_sel: %d\n", object.m_other_modes.z_source_sel);
						printf("    dither_alpha_en: %d\n", object.m_other_modes.dither_alpha_en);
						printf("    alpha_compare_en: %d\n", object.m_other_modes.alpha_compare_en);
						printf("    alpha_dither_mode: %d\n", object.m_other_modes.alpha_dither_mode);
						printf("combine:\n");
						printf("    RGB sub A, cycle 0: %s\n", s_suba_rgb[m_combine.sub_a_rgb0]);
						printf("    RGB sub B, cycle 0: %s\n", s_subb_rgb[m_combine.sub_b_rgb0]);
						printf("    RGB mul, cycle 0: %s\n", s_mul_rgb[m_combine.mul_rgb0]);
						printf("    RGB add, cycle 0: %s\n", s_add_rgb[m_combine.add_rgb0]);
						printf("    Alpha sub A, cycle 0: %s\n", s_sub_a[m_combine.sub_a_a0]);
						printf("    Alpha sub B, cycle 0: %s\n", s_sub_a[m_combine.sub_b_a0]);
						printf("    Alpha mul, cycle 0: %s\n", s_mul_a[m_combine.mul_a0]);
						printf("    Alpha add, cycle 0: %s\n\n", s_add_rgb[m_combine.add_a0]);
						printf("    RGB sub A, cycle 1: %s\n", s_suba_rgb[m_combine.sub_a_rgb1]);
						printf("    RGB sub B, cycle 1: %s\n", s_subb_rgb[m_combine.sub_b_rgb1]);
						printf("    RGB mul, cycle 1: %s\n", s_mul_rgb[m_combine.mul_rgb1]);
						printf("    RGB add, cycle 1: %s\n", s_add_rgb[m_combine.add_rgb1]);
						printf("    Alpha sub A, cycle 1: %s\n", s_sub_a[m_combine.sub_a_a1]);
						printf("    Alpha sub B, cycle 1: %s\n", s_sub_a[m_combine.sub_b_a1]);
						printf("    Alpha mul, cycle 1: %s\n", s_mul_a[m_combine.mul_a1]);
						printf("    Alpha add, cycle 1: %s\n\n", s_add_rgb[m_combine.add_a1]);
						printf("Texel 0: %08x\n", (uint32_t)userdata->m_texel0_color.to_rgba());
						printf("Texel 1: %08x\n", (uint32_t)userdata->m_texel1_color.to_rgba());
						printf("Env: %08x\n", (uint32_t)userdata->m_env_color.to_rgba());
						printf("Prim: %08x\n", (uint32_t)userdata->m_prim_color.to_rgba());
						printf("Mem: %08x\n", (uint32_t)userdata->m_memory_color.to_rgba());
						printf("Shade: %08x\n", (uint32_t)userdata->m_shade_color.to_rgba());
						printf("sargb: %08x, %08x, %08x, %08x\n", (uint32_t)sa, (uint32_t)sr, (uint32_t)sg, (uint32_t)sb);

						printf("Blend index: %d\n", (userdata->m_blend_enable << 2) | blend_index);
						int32_t cdith = 0;
						int32_t adith = 0;
						get_dither_values(scanline, j, &cdith, &adith, object);
						color_t reblended_pixel;
						((&m_blender)->*(m_blender.blend2[(userdata->m_blend_enable << 2) | blend_index]))(reblended_pixel, cdith, adith, partialreject, sel0, sel1, userdata, object/*, true*/);

						//((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tilenum, 0, userdata, object/*, true*/);
					}
				}
			}
#endif

			if(z_compare(zbcur, zhbcur, sz, dzpix, userdata, object))
			{
				get_dither_values(scanline, j, &cdith, &adith, object);

				color_t blended_pixel;
				bool rendered = ((&m_blender)->*(m_blender.blend2[(userdata->m_blend_enable << 2) | blend_index]))(blended_pixel, cdith, adith, partialreject, sel0, sel1, userdata, object/*, false*/);

				if (rendered)
				{
#if DEBUG_RDP_PIXEL
					if (x == 157 && scanline == 89 && s_debug_drawing)
					{
						printf("WRITE2: %08x\n", (uint32_t)blended_pixel.to_rgba());
					}
#endif
					((this)->*(m_write_pixel[object.m_misc_state.m_fb_size]))(curpixel, blended_pixel, userdata, object);
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

void n64_rdp::span_draw_copy(int32_t scanline, const extent_t &extent, const rdp_poly_state &object, int32_t threadid)
{
	const int32_t clipx1 = object.m_scissor.m_xh;
	const int32_t clipx2 = object.m_scissor.m_xl;
	const int32_t tilenum = object.tilenum;
	const bool flip = object.flip;

	rdp_span_aux* userdata = (rdp_span_aux*)extent.userdata;
	const int32_t xstart = extent.startx;
	const int32_t xend = userdata->m_unscissored_rx;
	const int32_t xend_scissored = extent.stopx;
	const int32_t xinc = flip ? 1 : -1;
	const int32_t length = flip ? (xstart - xend) : (xend - xstart);

	span_param_t s; s.w = extent.param[SPAN_S].start;
	span_param_t t; t.w = extent.param[SPAN_T].start;

	const int32_t ds = object.m_span_base.m_span_ds / 4;
	const int32_t dt = object.m_span_base.m_span_dt / 4;
	const int32_t dsinc = flip ? (ds) : -ds;
	const int32_t dtinc = flip ? (dt) : -dt;

	const int32_t fb_index = object.m_misc_state.m_fb_width * scanline;

	int32_t x = xend;

	for (int32_t j = 0; j <= length; j++)
	{
		const bool valid_x = (flip) ? (x >= xend_scissored) : (x <= xend_scissored);

		if (x >= clipx1 && x < clipx2 && valid_x)
		{
			int32_t sss = s.h.h;
			int32_t sst = t.h.h;
			m_tex_pipe.copy(&userdata->m_texel0_color, sss, sst, tilenum, object, userdata);

			uint32_t curpixel = fb_index + x;
			if (userdata->m_texel0_color.get_a() != 0 || !object.m_other_modes.alpha_compare_en || object.m_misc_state.m_fb_size == 1)
			{
				((this)->*(m_copy_pixel[object.m_misc_state.m_fb_size]))(curpixel, userdata->m_texel0_color, object);
			}
		}

		s.w += dsinc;
		t.w += dtinc;
		x += xinc;
	}
}

void n64_rdp::span_draw_fill(int32_t scanline, const extent_t &extent, const rdp_poly_state &object, int32_t threadid)
{
	assert(object.m_misc_state.m_fb_size < 4);

	const bool flip = object.flip;

	const int32_t clipx1 = object.m_scissor.m_xh;
	const int32_t clipx2 = object.m_scissor.m_xl;

	const int32_t xinc = flip ? 1 : -1;

	const int32_t fb_index = object.m_misc_state.m_fb_width * scanline;

	const int32_t xstart = extent.startx;
	const int32_t xend_scissored = extent.stopx;

	int32_t x = xend_scissored;

	const int32_t length = flip ? (xstart - xend_scissored) : (xend_scissored - xstart);

	for (int32_t j = 0; j <= length; j++)
	{
		if (x >= clipx1 && x < clipx2)
		{
			((this)->*(m_fill_pixel[object.m_misc_state.m_fb_size]))(fb_index + x, object);
		}

		x += xinc;
	}
}
