// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor
    -------------------

    by MooglyGuy
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
#include "video/n64.h"
#include "video/rdpblend.h"
#include "video/rdptpipe.h"

#include <algorithm>

#define LOG_RDP_EXECUTION       0

static FILE* rdp_exec;

UINT32 n64_rdp::s_special_9bit_clamptable[512];

bool n64_rdp::rdp_range_check(UINT32 addr)
{
	if(m_misc_state.m_fb_size == 0) return false;

	INT32 fbcount = ((m_misc_state.m_fb_width * m_scissor.m_yl) << (m_misc_state.m_fb_size - 1)) * 3;
	INT32 fbaddr = m_misc_state.m_fb_address & 0x007fffff;
	if ((addr >= fbaddr) && (addr < (fbaddr + fbcount)))
	{
		return false;
	}

	INT32 zbcount = m_misc_state.m_fb_width * m_scissor.m_yl * 2;
	INT32 zbaddr = m_misc_state.m_zb_address & 0x007fffff;
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
#include "rdpfiltr.inc"

INT32 n64_rdp::get_alpha_cvg(INT32 comb_alpha, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	INT32 temp = comb_alpha;
	INT32 temp2 = userdata->m_current_pix_cvg;
	INT32 temp3 = 0;

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

void n64_rdp::video_update(n64_periphs* n64, bitmap_rgb32 &bitmap)
{
	switch(n64->vi_control & 0x3)
	{
		case PIXEL_SIZE_16BIT:
			video_update16(n64, bitmap);
			break;

		case PIXEL_SIZE_32BIT:
			video_update32(n64, bitmap);
			break;

		default:
			//fatalerror("Unsupported framebuffer depth: m_fb_size=%d\n", m_misc_state.m_fb_size);
			break;
	}
}

void n64_rdp::video_update16(n64_periphs* n64, bitmap_rgb32 &bitmap)
{
	//INT32 fsaa = (((n64->vi_control >> 8) & 3) < 2);
	//INT32 divot = (n64->vi_control >> 4) & 1;

	//UINT32 prev_cvg = 0;
	//UINT32 next_cvg = 0;
	//INT32 dither_filter = (n64->vi_control >> 16) & 1;
	//INT32 vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	UINT16* frame_buffer = (UINT16*)&rdram[(n64->vi_origin & 0xffffff) >> 2];
	//UINT32 hb = ((n64->vi_origin & 0xffffff) >> 2) >> 1;
	//UINT8* hidden_buffer = &m_hidden_bits[hb];

	INT32 hdiff = (n64->vi_hstart & 0x3ff) - ((n64->vi_hstart >> 16) & 0x3ff);
	float hcoeff = ((float)(n64->vi_xscale & 0xfff) / (1 << 10));
	UINT32 hres = ((float)hdiff * hcoeff);
	INT32 invisiblewidth = n64->vi_width - hres;

	INT32 vdiff = ((n64->vi_vstart & 0x3ff) - ((n64->vi_vstart >> 16) & 0x3ff)) >> 1;
	float vcoeff = ((float)(n64->vi_yscale & 0xfff) / (1 << 10));
	UINT32 vres = ((float)vdiff * vcoeff);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return;
	}

	if (hres > 640) // Needed by Top Gear Overdrive (E)
	{
		invisiblewidth += (hres - 640);
		hres = 640;
	}

	if (vres > bitmap.height()) // makes Perfect Dark boot w/o crashing
	{
		vres = bitmap.height();
	}

	UINT32 pixels = 0;

	if (frame_buffer)
	{
		for(INT32 j = 0; j < vres; j++)
		{
			UINT32* d = &bitmap.pix32(j);

			for(INT32 i = 0; i < hres; i++)
			{
				UINT16 pix = frame_buffer[pixels ^ WORD_ADDR_XOR];

				const UINT8 r = ((pix >> 8) & 0xf8) | (pix >> 13);
				const UINT8 g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				const UINT8 b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);
				d[i] = (r << 16) | (g << 8) | b;
				pixels++;
			}
			pixels +=invisiblewidth;
		}
	}
}

void n64_rdp::video_update32(n64_periphs* n64, bitmap_rgb32 &bitmap)
{
	INT32 gamma = (n64->vi_control >> 3) & 1;
	INT32 gamma_dither = (n64->vi_control >> 2) & 1;
	//INT32 vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	UINT32* frame_buffer32 = (UINT32*)&rdram[(n64->vi_origin & 0xffffff) >> 2];

	const INT32 hdiff = (n64->vi_hstart & 0x3ff) - ((n64->vi_hstart >> 16) & 0x3ff);
	const float hcoeff = ((float)(n64->vi_xscale & 0xfff) / (1 << 10));
	UINT32 hres = ((float)hdiff * hcoeff);
	INT32 invisiblewidth = n64->vi_width - hres;

	const INT32 vdiff = ((n64->vi_vstart & 0x3ff) - ((n64->vi_vstart >> 16) & 0x3ff)) >> 1;
	const float vcoeff = ((float)(n64->vi_yscale & 0xfff) / (1 << 10));
	const UINT32 vres = ((float)vdiff * vcoeff);

	if (vdiff <= 0 || hdiff <= 0)
	{
		return;
	}

	if (hres > 640) // Needed by Top Gear Overdrive (E)
	{
		invisiblewidth += (hres - 640);
		hres = 640;
	}

	if (frame_buffer32)
	{
		for (INT32 j = 0; j < vres; j++)
		{
			UINT32* d = &bitmap.pix32(j);
			for (INT32 i = 0; i < hres; i++)
			{
				UINT32 pix = *frame_buffer32++;
				if (gamma || gamma_dither)
				{
					INT32 r = (pix >> 24) & 0xff;
					INT32 g = (pix >> 16) & 0xff;
					INT32 b = (pix >> 8) & 0xff;
					INT32 dith = 0;
					if (gamma_dither)
					{
						dith = get_random() & 0x3f;
					}
					if (gamma)
					{
						if (gamma_dither)
						{
							r = m_gamma_dither_table[(r << 6)| dith];
							g = m_gamma_dither_table[(g << 6)| dith];
							b = m_gamma_dither_table[(b << 6)| dith];
						}
						else
						{
							r = m_gamma_table[r];
							g = m_gamma_table[g];
							b = m_gamma_table[b];
						}
					}
					else if (gamma_dither)
					{
						if (r < 255)
							r += (dith & 1);
						if (g < 255)
							g += (dith & 1);
						if (b < 255)
							b += (dith & 1);
					}
					pix = (r << 24) | (g << 16) | (b << 8);
				}


				d[i] = (pix >> 8);
			}
			frame_buffer32 += invisiblewidth;
		}
	}
}

/*****************************************************************************/

void n64_rdp::tc_div_no_perspective(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst)
{
	*sss = (SIGN16(ss)) & 0x1ffff;
	*sst = (SIGN16(st)) & 0x1ffff;
}

void n64_rdp::tc_div(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst)
{
	INT32 w_carry = 0;
	if ((sw & 0x8000) || !(sw & 0x7fff))
	{
		w_carry = 1;
	}

	sw &= 0x7fff;

	INT32 shift;
	for (shift = 1; shift <= 14 && !((sw << shift) & 0x8000); shift++);
	shift -= 1;

	INT32 normout = (sw << shift) & 0x3fff;
	INT32 wnorm = (normout & 0xff) << 2;
	normout >>= 8;

	INT32 temppoint = m_norm_point_rom[normout];
	INT32 tempslope = m_norm_slope_rom[normout];

	INT32 tlu_rcp = ((-(tempslope * wnorm)) >> 10) + temppoint;

	INT32 sprod = SIGN16(ss) * tlu_rcp;
	INT32 tprod = SIGN16(st) * tlu_rcp;
	INT32 tempmask = ((1 << (shift + 1)) - 1) << (29 - shift);
	INT32 shift_value = 13 - shift;

	INT32 outofbounds_s = sprod & tempmask;
	INT32 outofbounds_t = tprod & tempmask;
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
	INT32 under_s = 0;
	INT32 under_t = 0;
	INT32 over_s = 0;
	INT32 over_t = 0;

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

INT32 n64_rdp::color_combiner_equation(INT32 a, INT32 b, INT32 c, INT32 d)
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

INT32 n64_rdp::alpha_combiner_equation(INT32 a, INT32 b, INT32 c, INT32 d)
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

void n64_rdp::set_suba_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata)
{
	userdata->m_noise_used = false;
	switch (code & 0xf)
	{
		case 0:     *input = &userdata->m_combined_color; break;
		case 1:     *input = &userdata->m_texel0_color; break;
		case 2:     *input = &userdata->m_texel1_color; break;
		case 3:     *input = &userdata->m_prim_color; break;
		case 4:     *input = &userdata->m_shade_color; break;
		case 5:     *input = &userdata->m_env_color; break;
		case 6:     *input = &m_one; break;
		case 7:     *input = &userdata->m_noise_color; userdata->m_noise_used = true; break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
					*input = &m_zero; break;
		}
	}
}

void n64_rdp::set_subb_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata)
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

void n64_rdp::set_mul_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata)
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

void n64_rdp::set_add_input_rgb(color_t** input, INT32 code, rdp_span_aux* userdata)
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

void n64_rdp::set_sub_input_alpha(color_t** input, INT32 code, rdp_span_aux* userdata)
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

void n64_rdp::set_mul_input_alpha(color_t** input, INT32 code, rdp_span_aux* userdata)
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

void n64_rdp::set_blender_input(INT32 cycle, INT32 which, color_t** input_rgb, color_t** input_a, INT32 a, INT32 b, rdp_span_aux* userdata)
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

const UINT8 n64_rdp::s_bayer_matrix[16] =
{ /* Bayer matrix */
		0,  4,  1, 5,
		6,  2,  7, 3,
		1,   5,  0, 4,
		7,  3,  6, 2
};

const UINT8 n64_rdp::s_magic_matrix[16] =
{ /* Magic square matrix */
		0,  6,  1, 7,
		4,  2,  5, 3,
		3,   5,  2, 4,
		7,  1,  6, 0
};

const z_decompress_entry_t n64_rdp::m_z_dec_table[8] =
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
	UINT16 altmem = 0;
	for(INT32 z = 0; z < 0x40000; z++)
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
	const UINT8 yarray[16] = {0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const UINT8 xarray[16] = {0, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	for (INT32 i = 0; i < 0x100; i++)
	{
		UINT16 mask = decompress_cvmask_frombyte(i);
		cvarray[i].cvg = cvarray[i].cvbit = 0;
		cvarray[i].cvbit = (i >> 7) & 1;
		for (INT32 k = 0; k < 8; k++)
		{
			cvarray[i].cvg += ((i >> k) & 1);
		}

		UINT16 masky = 0;
		for (INT32 k = 0; k < 4; k++)
		{
			masky |= ((mask & (0xf000 >> (k << 2))) > 0) << k;
		}
		UINT8 offy = yarray[masky];

		UINT16 maskx = (mask & (0xf000 >> (offy << 2))) >> ((offy ^ 3) << 2);
		UINT8 offx = xarray[maskx];

		cvarray[i].xoff = offx;
		cvarray[i].yoff = offy;
	}
}

UINT16 n64_rdp::decompress_cvmask_frombyte(UINT8 x)
{
	return (x & 1) | ((x & 2) << 4) | (x & 4) | ((x & 8) << 4) | ((x & 0x10) << 4) | ((x & 0x20) << 8) | ((x & 0x40) << 4) | ((x & 0x80) << 8);
}

void n64_rdp::lookup_cvmask_derivatives(UINT32 mask, UINT8* offx, UINT8* offy, rdp_span_aux* userdata)
{
	userdata->m_current_pix_cvg = cvarray[mask].cvg;
	userdata->m_current_cvg_bit = cvarray[mask].cvbit;
	*offx = cvarray[mask].xoff;
	*offy = cvarray[mask].yoff;
}

void n64_rdp::z_store(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 z, UINT32 dzpix_enc)
{
	RWRITEIDX16(zcurpixel, m_z_com_table[z & 0x3ffff] | (dzpix_enc >> 2));
	HWRITEADDR8(dzcurpixel, dzpix_enc & 3);
}

INT32 n64_rdp::normalize_dzpix(INT32 sum)
{
	if (sum & 0xc000)
	{
		return 0x8000;
	}
	if (!(sum & 0xffff))
	{
		return 1;
	}
	if (sum == 1)
	{
		return 3;
	}
	for(INT32 count = 0x2000; count > 0; count >>= 1)
	{
		if (sum & count)
		{
			return(count << 1);
		}
	}
	return 0;
}

UINT32 n64_rdp::z_decompress(UINT32 zcurpixel)
{
	return m_z_complete_dec_table[(RREADIDX16(zcurpixel) >> 2) & 0x3fff];
}

UINT32 n64_rdp::dz_compress(UINT32 value)
{
	INT32 j = 0;
	if (value & 0xff00)
	{
		j |= 8;
	}
	if (value & 0xf0f0)
	{
		j |= 4;
	}
	if (value & 0xcccc)
	{
		j |= 2;
	}
	if (value & 0xaaaa)
	{
		j |= 1;
	}
	return j;
}

void n64_rdp::get_dither_values(INT32 x, INT32 y, INT32* cdith, INT32* adith, const rdp_poly_state& object)
{
	const INT32 dithindex = ((y & 3) << 2) | (x & 3);
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
		*adith = m_machine->rand() & 7;
		break;
	case 15:
		*adith = *cdith = 0;
		break;
	}
}

INT32 CLAMP(INT32 in, INT32 min, INT32 max)
{
	if(in < min) return min;
	if(in > max) return max;
	return in;
}

bool n64_rdp::z_compare(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 sz, UINT16 dzpix, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	bool force_coplanar = false;
	sz &= 0x3ffff;

	UINT32 oz;
	UINT32 dzmem;
	UINT32 zval;
	INT32 rawdzmem;

	if (object.m_other_modes.z_compare_en)
	{
		oz = z_decompress(zcurpixel);
		zval = RREADIDX16(zcurpixel);
		UINT32 hval = HREADADDR8(dzcurpixel);
		rawdzmem = ((zval & 3) << 2) | hval;
		dzmem = 1 << rawdzmem;

		userdata->m_shift_a = CLAMP(userdata->m_dzpix_enc - rawdzmem, 0, 4);
		userdata->m_shift_b = CLAMP(rawdzmem - userdata->m_dzpix_enc, 0, 4);

		if(object.m_other_modes.cycle_type == CYCLE_TYPE_2)
		{
			userdata->m_prev_shift_a = CLAMP(userdata->m_dzpix_enc - userdata->m_prev_raw_dzmem, 0, 4);
			userdata->m_prev_shift_b = CLAMP(userdata->m_prev_raw_dzmem - userdata->m_dzpix_enc, 0, 4);
		}
		userdata->m_prev_raw_dzmem = rawdzmem;

		INT32 precision_factor = (zval >> 13) & 0xf;
		if (precision_factor < 3)
		{
			if (dzmem != 0x8000)
			{
				UINT32 dzmemmodifier = 16 >> precision_factor;
				dzmem <<= 1;
				if (dzmem < dzmemmodifier)
				{
					dzmem = dzmemmodifier;
				}
			}
			else
			{
				force_coplanar = true;
				dzmem = 0xffff;
			}
		}

		UINT32 dznew = m_dz_compare_lut[dzpix | dzmem];
		UINT32 dznotshift = dznew;
		dznew <<= 3;

		bool farther = force_coplanar || (sz + dznew) >= oz;

		bool overflow = ((userdata->m_current_mem_cvg + userdata->m_current_pix_cvg) & 8) > 0;
		userdata->m_blend_enable = (object.m_other_modes.force_blend || (!overflow && object.m_other_modes.antialias_en && farther)) ? 1 : 0;
		userdata->m_pre_wrap = overflow;

		switch(object.m_other_modes.z_mode)
		{
			case ZMODE_OPAQUE:
			{
				bool infront = sz < oz;
				INT32 diff = (INT32)sz - (INT32)dznew;
				bool nearer = force_coplanar || (diff <= (INT32)oz);
				bool max = oz == 0x3ffff;
				return (max || (overflow ? infront : nearer));
			}
			case ZMODE_INTERPENETRATING:
			{
				bool infront = sz < oz;
				if (!infront || !farther || !overflow)
				{
					INT32 diff = (INT32)sz - (INT32)dznew;
					bool nearer = force_coplanar || (diff <= (INT32)oz);
					bool max = oz == 0x3ffff;
					return (max || (overflow ? infront : nearer));
				}
				else
				{
					UINT32 dzenc = dz_compress(dznotshift & 0xffff);
					INT32 cvgcoeff = ((oz >> dzenc) - (sz >> dzenc)) & 0xf;
					userdata->m_current_pix_cvg = ((cvgcoeff * userdata->m_current_pix_cvg) >> 3) & 0xf;
					return true;
				}
				break;
			}
			case ZMODE_TRANSLUCENT:
			{
				return (sz < oz || oz == 0x3ffff);
			}
			case ZMODE_DECAL:
			{
				INT32 diff = (INT32)sz - (INT32)dznew;
				bool nearer = force_coplanar || (diff <= (INT32)oz);
				bool max = oz == 0x3ffff;
				return farther && nearer && !max;
			}
		}
	}
	else
	{
		userdata->m_shift_a = 0;
		userdata->m_shift_b = (userdata->m_dzpix_enc < 0xb) ? 4 : (0xf - userdata->m_dzpix_enc);

		if(object.m_other_modes.cycle_type == CYCLE_TYPE_2)
		{
			userdata->m_prev_shift_a = 0;
			userdata->m_prev_shift_b = userdata->m_shift_b;
		}
		userdata->m_prev_raw_dzmem = 0xf;

		bool overflow = ((userdata->m_current_mem_cvg + userdata->m_current_pix_cvg) & 8) > 0;
		userdata->m_blend_enable = (object.m_other_modes.force_blend || (!overflow && object.m_other_modes.antialias_en)) ? 1 : 0;
		userdata->m_pre_wrap = overflow;
	}

	return true;
}

UINT32 n64_rdp::get_log2(UINT32 lod_clamp)
{
	if (lod_clamp < 2)
	{
		return 0;
	}
	else
	{
		for (INT32 i = 7; i > 0; i--)
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

UINT32 n64_rdp::read_data(UINT32 address)
{
	if (m_status & 0x1)     // XBUS_DMEM_DMA enabled
	{
		return rsp_dmem[(address & 0xfff) / 4];
	}
	else
	{
		return rdram[((address & 0xffffff) / 4)];
	}
}

const char* n64_rdp::s_image_format[] = { "RGBA", "YUV", "CI", "IA", "I", "???", "???", "???" };
const char* n64_rdp::s_image_size[] = { "4-bit", "8-bit", "16-bit", "32-bit" };

const INT32 n64_rdp::s_rdp_command_length[64] =
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

void n64_rdp::disassemble(char* buffer)
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

	UINT32 cmd[64];

	const UINT32 length = m_cmd_ptr * 4;
	if (length < 8)
	{
		sprintf(buffer, "ERROR: length = %d\n", length);
		return;
	}

	cmd[0] = m_cmd_data[m_cmd_cur+0];
	cmd[1] = m_cmd_data[m_cmd_cur+1];

	const INT32 tile = (cmd[1] >> 24) & 0x7;
	sprintf(sl, "%4.2f", (float)((cmd[0] >> 12) & 0xfff) / 4.0f);
	sprintf(tl, "%4.2f", (float)((cmd[0] >>  0) & 0xfff) / 4.0f);
	sprintf(sh, "%4.2f", (float)((cmd[1] >> 12) & 0xfff) / 4.0f);
	sprintf(th, "%4.2f", (float)((cmd[1] >>  0) & 0xfff) / 4.0f);

	const char* format = s_image_format[(cmd[0] >> 21) & 0x7];
	const char* size = s_image_size[(cmd[0] >> 19) & 0x3];

	const UINT32 r = (cmd[1] >> 24) & 0xff;
	const UINT32 g = (cmd[1] >> 16) & 0xff;
	const UINT32 b = (cmd[1] >>  8) & 0xff;
	const UINT32 a = (cmd[1] >>  0) & 0xff;

	const UINT32 command = (cmd[0] >> 24) & 0x3f;
	switch (command)
	{
		case 0x00:  sprintf(buffer, "No Op"); break;
		case 0x08:      // Tri_NoShade
		{
			const INT32 lft = (command >> 23) & 0x1;

			if (length != s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_NoShade length = %d\n", length);
				return;
			}

			cmd[2] = m_cmd_data[m_cmd_cur+2];
			cmd[3] = m_cmd_data[m_cmd_cur+3];
			cmd[4] = m_cmd_data[m_cmd_cur+4];
			cmd[5] = m_cmd_data[m_cmd_cur+5];
			cmd[6] = m_cmd_data[m_cmd_cur+6];
			cmd[7] = m_cmd_data[m_cmd_cur+7];

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)(cmd[7] / 65536.0f));

			sprintf(buffer, "Tri_NoShade            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x09:      // Tri_NoShadeZ
		{
			const INT32 lft = (command >> 23) & 0x1;

			if (length != s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_NoShadeZ length = %d\n", length);
				return;
			}

			cmd[2] = m_cmd_data[m_cmd_cur+2];
			cmd[3] = m_cmd_data[m_cmd_cur+3];
			cmd[4] = m_cmd_data[m_cmd_cur+4];
			cmd[5] = m_cmd_data[m_cmd_cur+5];
			cmd[6] = m_cmd_data[m_cmd_cur+6];
			cmd[7] = m_cmd_data[m_cmd_cur+7];

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)(cmd[7] / 65536.0f));

			sprintf(buffer, "Tri_NoShadeZ            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x0a:      // Tri_Tex
		{
			const INT32 lft = (command >> 23) & 0x1;

			if (length < s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Tex length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));

			sprintf(s,      "%4.4f", (float)(INT32)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)(INT32)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)(INT32)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


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
			const INT32 lft = (command >> 23) & 0x1;

			if (length < s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexZ length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));

			sprintf(s,      "%4.4f", (float)(INT32)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)(INT32)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)(INT32)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


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
			const INT32 lft = (command >> 23) & 0x1;

			if (length != s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Shade length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,     "%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

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
			const INT32 lft = (command >> 23) & 0x1;

			if (length != s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_ShadeZ length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,     "%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

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
			const INT32 lft = (command >> 23) & 0x1;

			if (length < s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShade length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 40; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,     "%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,      "%4.4f", (float)(INT32)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)(INT32)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)(INT32)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)(INT32)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)(INT32)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)(INT32)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)(INT32)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)(INT32)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)(INT32)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)(INT32)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)(INT32)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)(INT32)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


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
			const INT32 lft = (command >> 23) & 0x1;

			if (length < s_rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShadeZ length = %d\n", length);
				return;
			}

			for (INT32 i = 2; i < 40; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,     "%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,     "%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,     "%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,     "%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,  "%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,     "%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,  "%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,     "%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,  "%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,     "%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,     "%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,     "%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,     "%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,   "%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,   "%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,   "%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,   "%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,   "%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,   "%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,   "%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,   "%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,   "%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,   "%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,   "%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,   "%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,      "%4.4f", (float)(INT32)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,      "%4.4f", (float)(INT32)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,      "%4.4f", (float)(INT32)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,   "%4.4f", (float)(INT32)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,   "%4.4f", (float)(INT32)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,   "%4.4f", (float)(INT32)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,   "%4.4f", (float)(INT32)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,   "%4.4f", (float)(INT32)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,   "%4.4f", (float)(INT32)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,   "%4.4f", (float)(INT32)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,   "%4.4f", (float)(INT32)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,   "%4.4f", (float)(INT32)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


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
			if (length < 16)
			{
				sprintf(buffer, "ERROR: Texture_Rectangle length = %d\n", length);
				return;
			}
			cmd[2] = m_cmd_data[m_cmd_cur+2];
			cmd[3] = m_cmd_data[m_cmd_cur+3];
			sprintf(s,    "%4.4f", (float)(INT16)((cmd[2] >> 16) & 0xffff) / 32.0f);
			sprintf(t,    "%4.4f", (float)(INT16)((cmd[2] >>  0) & 0xffff) / 32.0f);
			sprintf(dsdx, "%4.4f", (float)(INT16)((cmd[3] >> 16) & 0xffff) / 1024.0f);
			sprintf(dtdy, "%4.4f", (float)(INT16)((cmd[3] >> 16) & 0xffff) / 1024.0f);

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
		case 0x2e:  sprintf(buffer, "Set_Prim_Depth         %04X, %04X", (cmd[1] >> 16) & 0xffff, cmd[1] & 0xffff); break;
		case 0x2f:  sprintf(buffer, "Set_Other_Modes        %08X %08X", cmd[0], cmd[1]); break;
		case 0x30:  sprintf(buffer, "Load_TLUT              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x32:  sprintf(buffer, "Set_Tile_Size          %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x33:  sprintf(buffer, "Load_Block             %d, %03X, %03X, %03X, %03X", tile, (cmd[0] >> 12) & 0xfff, cmd[0] & 0xfff, (cmd[1] >> 12) & 0xfff, cmd[1] & 0xfff); break;
		case 0x34:  sprintf(buffer, "Load_Tile              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x35:  sprintf(buffer, "Set_Tile               %d, %s, %s, %d, %04X", tile, format, size, ((cmd[0] >> 9) & 0x1ff) * 8, (cmd[0] & 0x1ff) * 8); break;
		case 0x36:  sprintf(buffer, "Fill_Rectangle         %s, %s, %s, %s", sh, th, sl, tl); break;
		case 0x37:  sprintf(buffer, "Set_Fill_Color         R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x38:  sprintf(buffer, "Set_Fog_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x39:  sprintf(buffer, "Set_Blend_Color        R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3a:  sprintf(buffer, "Set_Prim_Color         %d, %d, R: %d, G: %d, B: %d, A: %d", (cmd[0] >> 8) & 0x1f, cmd[0] & 0xff, r, g, b, a); break;
		case 0x3b:  sprintf(buffer, "Set_Env_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3c:  sprintf(buffer, "Set_Combine            %08X %08X", cmd[0], cmd[1]); break;
		case 0x3d:  sprintf(buffer, "Set_Texture_Image      %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		case 0x3e:  sprintf(buffer, "Set_Mask_Image         %08X", cmd[1]); break;
		case 0x3f:  sprintf(buffer, "Set_Color_Image        %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		default:    sprintf(buffer, "Unknown (%08X %08X)", cmd[0], cmd[1]); break;
	}
}

/*****************************************************************************/

static UINT32 rightcvghex(UINT32 x, UINT32 fmask)
{
	return (0xf0 >> (((x & 7) + 1) >> 1)) & fmask;
}

static UINT32 leftcvghex(UINT32 x, UINT32 fmask)
{
	return (0xf >> (((x & 7) + 1) >> 1)) & fmask;
}

void n64_rdp::compute_cvg_noflip(rdp_span_aux* data, UINT32 lx, UINT32 rx)
{
	UINT8* cvgbuf = data->m_cvg;
	INT32 purgestart = lx;
	INT32 purgeend = rx;
	INT32 length = purgeend - purgestart;

	if (length < 0)
	{
		return;
	}

	memset(&cvgbuf[purgestart], 0xff, length + 1);

	for(INT32 i = 0; i < 4; i++)
	{
		const INT32 fmask = 0xa >> (i & 1);
		const INT32 maskshift = (i - 2) & 4;
		const INT32 fmaskshifted = fmask << maskshift;

		if (!data->m_invalid_y[i])
		{
			INT32 minorcur = data->m_minorx[i];
			INT32 majorcur = data->m_majorx[i];
			INT32 minorcurint = minorcur >> 3;
			INT32 majorcurint = majorcur >> 3;

			for (INT32 k = purgestart; k <= minorcurint; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}
			for (INT32 k = majorcurint; k <= purgeend; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}

			if (majorcurint > minorcurint)
			{
				cvgbuf[minorcurint] |= leftcvghex(minorcur, fmask) << maskshift;
				cvgbuf[majorcurint] |= rightcvghex(majorcur, fmask) << maskshift;
			}
			else if (majorcurint == minorcurint)
			{
				INT32 samecvg = leftcvghex(minorcur, fmask) & rightcvghex(majorcur, fmask);
				cvgbuf[majorcurint] |= samecvg << maskshift;
			}
		}
		else
		{
			for (INT32 k = purgestart; k <= purgeend; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}
		}
	}
}

void n64_rdp::compute_cvg_flip(rdp_span_aux* data, UINT32 lx, UINT32 rx)
{
	UINT8* cvgbuf = data->m_cvg;
	INT32 purgestart = rx;
	INT32 purgeend = lx;
	INT32 length = purgeend - purgestart;

	if (length < 0)
	{
		return;
	}

	memset(&cvgbuf[purgestart], 0xff, length + 1);

	for(INT32 i = 0; i < 4; i++)
	{
		const INT32 fmask = 0xa >> (i & 1);
		const INT32 maskshift = (i - 2) & 4;
		const INT32 fmaskshifted = fmask << maskshift;

		if (!data->m_invalid_y[i])
		{
			INT32 minorcur = data->m_minorx[i];
			INT32 majorcur = data->m_majorx[i];
			INT32 minorcurint = minorcur >> 3;
			INT32 majorcurint = majorcur >> 3;

			for (INT32 k = purgestart; k <= majorcurint; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}
			for (INT32 k = minorcurint; k <= purgeend; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}

			if (minorcurint > majorcurint)
			{
				cvgbuf[minorcurint] |= rightcvghex(minorcur, fmask) << maskshift;
				cvgbuf[majorcurint] |= leftcvghex(majorcur, fmask) << maskshift;
			}
			else if (minorcurint == majorcurint)
			{
				INT32 samecvg = rightcvghex(minorcur, fmask) & leftcvghex(majorcur, fmask);
				cvgbuf[majorcurint] |= samecvg << maskshift;
			}
		}
		else
		{
			for (INT32 k = purgestart; k <= purgeend; k++)
			{
				cvgbuf[k] &= ~fmaskshifted;
			}
		}
	}
}

void n64_rdp::get_span_userdata(void** userdata)
{
	*userdata = (void*)((UINT8*)m_aux_buf + m_aux_buf_ptr);
	m_aux_buf_ptr += sizeof(rdp_span_aux);
	rdp_span_aux* data = (rdp_span_aux*)*userdata;
	data->m_valid_line = true;
}

void n64_rdp::deduce_derivatives(rdp_span_aux *userdata)
{
	m_mode_derivs.m_partial_reject_1cycle = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[0] == &userdata->m_pixel_color);
	m_mode_derivs.m_partial_reject_2cycle = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[1] == &userdata->m_pixel_color);

	m_mode_derivs.m_special_bsel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color);
	m_mode_derivs.m_special_bsel1 = (userdata->m_color_inputs.blender2b_a[1] == &userdata->m_memory_color);

	m_mode_derivs.m_rgb_alpha_dither = (m_other_modes.rgb_dither_sel << 2) | m_other_modes.alpha_dither_sel;

	const bool lod_frac_used_in_cycle1 = (userdata->m_color_inputs.combiner_rgbmul[0] == &userdata->m_lod_fraction) || (userdata->m_color_inputs.combiner_alphamul[0] == &userdata->m_lod_fraction);
	const bool lod_frac_used_in_cycle2 = (userdata->m_color_inputs.combiner_rgbmul[1] == &userdata->m_lod_fraction) || (userdata->m_color_inputs.combiner_alphamul[1] == &userdata->m_lod_fraction);
	const bool lod_frac_used = (m_other_modes.cycle_type == CYCLE_TYPE_2 && (lod_frac_used_in_cycle1 || lod_frac_used_in_cycle2)) || (m_other_modes.cycle_type == CYCLE_TYPE_1 && lod_frac_used_in_cycle2);
	m_mode_derivs.m_do_lod = m_other_modes.tex_lod_en || lod_frac_used;
}

void n64_rdp::draw_triangle(bool shade, bool texture, bool zbuffer, bool rect)
{
	const UINT32* cmd_data = rect ? m_temp_rect_data : m_cmd_data;
	const UINT32 fifo_index = rect ? 0 : m_cmd_cur;
	const UINT32 w1 = cmd_data[fifo_index + 0];
	const UINT32 w2 = cmd_data[fifo_index + 1];

	INT32 flip = (w1 & 0x00800000) ? 1 : 0;
	m_misc_state.m_max_level = ((w1 >> 19) & 7);
	INT32 tilenum = (w1 >> 16) & 0x7;

	INT32 dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	INT32 dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0;
	INT32 dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;
	INT32 dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;

	INT32 shade_base = fifo_index + 8;
	INT32 texture_base = fifo_index + 8;
	INT32 zbuffer_base = fifo_index + 8;
	if(shade)
	{
		texture_base += 16;
		zbuffer_base += 16;
	}
	if(texture)
	{
		zbuffer_base += 16;
	}

	UINT32 w3 = cmd_data[fifo_index + 2];
	UINT32 w4 = cmd_data[fifo_index + 3];
	UINT32 w5 = cmd_data[fifo_index + 4];
	UINT32 w6 = cmd_data[fifo_index + 5];
	UINT32 w7 = cmd_data[fifo_index + 6];
	UINT32 w8 = cmd_data[fifo_index + 7];

	INT32 yl = w1 & 0x3fff;
	INT32 ym = ((w2 >> 16) & 0x3fff);
	INT32 yh = w2 & 0x3fff;
	INT32 xl = w3 & 0x3fffffff;
	INT32 xh = w5 & 0x3fffffff;
	INT32 xm = w7 & 0x3fffffff;
	// Inverse slopes in 16.16 format
	INT32 dxldy = (INT32)(w4);
	INT32 dxhdy = (INT32)(w6);
	INT32 dxmdy = (INT32)(w8);

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	INT32 r    = (cmd_data[shade_base+0 ] & 0xffff0000) | ((cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	INT32 g    = ((cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (cmd_data[shade_base+4 ] & 0x0000ffff);
	INT32 b    = (cmd_data[shade_base+1 ] & 0xffff0000) | ((cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	INT32 a    = ((cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (cmd_data[shade_base+5 ] & 0x0000ffff);
	const INT32 drdx = (cmd_data[shade_base+2 ] & 0xffff0000) | ((cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	const INT32 dgdx = ((cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (cmd_data[shade_base+6 ] & 0x0000ffff);
	const INT32 dbdx = (cmd_data[shade_base+3 ] & 0xffff0000) | ((cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	const INT32 dadx = ((cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (cmd_data[shade_base+7 ] & 0x0000ffff);
	const INT32 drde = (cmd_data[shade_base+8 ] & 0xffff0000) | ((cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	const INT32 dgde = ((cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (cmd_data[shade_base+12] & 0x0000ffff);
	const INT32 dbde = (cmd_data[shade_base+9 ] & 0xffff0000) | ((cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	const INT32 dade = ((cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (cmd_data[shade_base+13] & 0x0000ffff);
	const INT32 drdy = (cmd_data[shade_base+10] & 0xffff0000) | ((cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	const INT32 dgdy = ((cmd_data[shade_base+10] << 16) & 0xffff0000) | (cmd_data[shade_base+14] & 0x0000ffff);
	const INT32 dbdy = (cmd_data[shade_base+11] & 0xffff0000) | ((cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	const INT32 dady = ((cmd_data[shade_base+11] << 16) & 0xffff0000) | (cmd_data[shade_base+15] & 0x0000ffff);
	INT32 s    = (cmd_data[texture_base+0 ] & 0xffff0000) | ((cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	INT32 t    = ((cmd_data[texture_base+0 ] << 16) & 0xffff0000) | (cmd_data[texture_base+4 ] & 0x0000ffff);
	INT32 w    = (cmd_data[texture_base+1 ] & 0xffff0000) | ((cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	const INT32 dsdx = (cmd_data[texture_base+2 ] & 0xffff0000) | ((cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	const INT32 dtdx = ((cmd_data[texture_base+2 ] << 16) & 0xffff0000) | (cmd_data[texture_base+6 ] & 0x0000ffff);
	const INT32 dwdx = (cmd_data[texture_base+3 ] & 0xffff0000) | ((cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	const INT32 dsde = (cmd_data[texture_base+8 ] & 0xffff0000) | ((cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	const INT32 dtde = ((cmd_data[texture_base+8 ] << 16) & 0xffff0000) | (cmd_data[texture_base+12] & 0x0000ffff);
	const INT32 dwde = (cmd_data[texture_base+9 ] & 0xffff0000) | ((cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	const INT32 dsdy = (cmd_data[texture_base+10] & 0xffff0000) | ((cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	const INT32 dtdy = ((cmd_data[texture_base+10] << 16) & 0xffff0000) | (cmd_data[texture_base+14] & 0x0000ffff);
	const INT32 dwdy = (cmd_data[texture_base+11] & 0xffff0000) | ((cmd_data[texture_base+15] >> 16) & 0x0000ffff);
	INT32 z    = cmd_data[zbuffer_base+0];
	const INT32 dzdx = cmd_data[zbuffer_base+1];
	const INT32 dzde = cmd_data[zbuffer_base+2];
	const INT32 dzdy = cmd_data[zbuffer_base+3];

	extent_t spans[2048];
#ifdef MAME_DEBUG
	memset(spans, 0xcc, sizeof(spans));
#endif

	m_span_base.m_span_ds = dsdx & ~0x1f;
	m_span_base.m_span_dt = dtdx & ~0x1f;
	m_span_base.m_span_dw = dwdx & ~0x1f;
	m_span_base.m_span_dr = drdx & ~0x1f;
	m_span_base.m_span_dg = dgdx & ~0x1f;
	m_span_base.m_span_db = dbdx & ~0x1f;
	m_span_base.m_span_da = dadx & ~0x1f;
	m_span_base.m_span_dz = dzdx;

	m_span_base.m_span_drdy = SIGN13(drdy >> 14);
	m_span_base.m_span_dgdy = SIGN13(dgdy >> 14);
	m_span_base.m_span_dbdy = SIGN13(dbdy >> 14);
	m_span_base.m_span_dady = SIGN13(dady >> 14);
	m_span_base.m_span_dzdy = dzdy >> 10;
	m_span_base.m_span_cdr = SIGN13(m_span_base.m_span_dr >> 14);
	m_span_base.m_span_cdg = SIGN13(m_span_base.m_span_dg >> 14);
	m_span_base.m_span_cdb = SIGN13(m_span_base.m_span_db >> 14);
	m_span_base.m_span_cda = SIGN13(m_span_base.m_span_da >> 14);
	m_span_base.m_span_cdz = SIGN22(m_span_base.m_span_dz >> 10);

	m_span_base.m_span_dsdy = dsdy & ~0x7fff;
	m_span_base.m_span_dtdy = dtdy & ~0x7fff;
	m_span_base.m_span_dwdy = dwdy & ~0x7fff;

	const INT32 dzdy_dz = (dzdy >> 16) & 0xffff;
	const INT32 dzdx_dz = (dzdx >> 16) & 0xffff;

	const UINT32 temp_dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	m_span_base.m_span_dzpix = m_dzpix_normalize[temp_dzpix & 0xffff];

	INT32 xleft_inc = (dxmdy >> 2) & ~1;
	INT32 xright_inc = (dxhdy >> 2) & ~1;

	INT32 xright = xh & ~1;
	INT32 xleft = xm & ~1;

	const INT32 sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;
	const INT32 do_offset = !(sign_dxhdy ^ flip);

	if (do_offset)
	{
		dsdeh = dsde & ~0x1ff;
		dtdeh = dtde & ~0x1ff;
		dwdeh = dwde & ~0x1ff;
		drdeh = drde & ~0x1ff;
		dgdeh = dgde & ~0x1ff;
		dbdeh = dbde & ~0x1ff;
		dadeh = dade & ~0x1ff;
		dzdeh = dzde & ~0x1ff;

		dsdyh = dsdy & ~0x1ff;
		dtdyh = dtdy & ~0x1ff;
		dwdyh = dwdy & ~0x1ff;
		drdyh = drdy & ~0x1ff;
		dgdyh = dgdy & ~0x1ff;
		dbdyh = dbdy & ~0x1ff;
		dadyh = dady & ~0x1ff;
		dzdyh = dzdy & ~0x1ff;

		dsdiff = (dsdeh - dsdyh) - (dsdiff >> 2);
		dtdiff = (dtdeh - dtdyh) - (dtdiff >> 2);
		dwdiff = (dwdeh - dwdyh) - (dwdiff >> 2);
		drdiff = (drdeh - drdyh) - (drdiff >> 2);
		dgdiff = (dgdeh - dgdyh) - (dgdiff >> 2);
		dbdiff = (dbdeh - dbdyh) - (dbdiff >> 2);
		dadiff = (dadeh - dadyh) - (dadiff >> 2);
		dzdiff = (dzdeh - dzdyh) - (dzdiff >> 2);
	}
	else
	{
		dsdiff = dtdiff = dwdiff = drdiff = dgdiff = dbdiff = dadiff = dzdiff = 0;
	}

	if (m_other_modes.cycle_type != CYCLE_TYPE_COPY)
	{
		dsdxh = (dsdx >> 8) & ~1;
		dtdxh = (dtdx >> 8) & ~1;
		dwdxh = (dwdx >> 8) & ~1;
		drdxh = (drdx >> 8) & ~1;
		dgdxh = (dgdx >> 8) & ~1;
		dbdxh = (dbdx >> 8) & ~1;
		dadxh = (dadx >> 8) & ~1;
		dzdxh = (dzdx >> 8) & ~1;
	}
	else
	{
		dsdxh = dtdxh = dwdxh = drdxh = dgdxh = dbdxh = dadxh = dzdxh = 0;
	}

	INT32 maxxmx = 0;
	INT32 minxmx = 0;
	INT32 maxxhx = 0;
	INT32 minxhx = 0;

	rdp_poly_state* object = &object_data_alloc();
	memcpy(object->m_tmem, m_tmem, 0x1000);

	const INT32 ycur = yh & ~3;
	const INT32 ldflag = (sign_dxhdy ^ flip) ? 0 : 3;

	bool invalid_y = true;
	bool limited_yl = false;
	INT32 yllimit = 0;
	if (yl & 0x2000)
	{
		limited_yl = true;
	}
	else if (yl & 0x1000)
	{
		limited_yl = false;
	}
	else
	{
		limited_yl = (yl & 0xfff) < m_scissor.m_yl;
	}
	yllimit = limited_yl ? yl : m_scissor.m_yl;

	INT32 invalid_index = -1;
	INT32 ylfar = yllimit | 3;
	if ((yl >> 2) > (ylfar >> 2))
	{
		ylfar += 4;
	}
	else if((yllimit >> 2) >= 0 && (yllimit >> 2) < 0x3ff)
	{
		INT32 idx = (yllimit >> 2) + 1;
		get_span_userdata(&spans[idx].userdata);
		((rdp_span_aux*)spans[idx].userdata)->m_valid_line = false;
		invalid_index = idx;
	}

	bool limited_yh = false;
	INT32 yhlimit = 0;
	if (yh & 0x2000)
	{
		limited_yh = false;
	}
	else if (yh & 0x1000)
	{
		limited_yh = true;
	}
	else
	{
		limited_yh = (yh >= m_scissor.m_yh);
	}
	yhlimit = limited_yh ? yh : m_scissor.m_yh;

	INT32 yhclose = yhlimit & ~3;

	INT32 clipxlshift = m_scissor.m_xl << 1;
	INT32 clipxhshift = m_scissor.m_xh << 1;

	bool allover = true;
	bool allunder = true;
	bool bothinvalid = true;
	bool anyvalid = false;
	bool first_line = true;

	INT32 xfrac = ((xright >> 8) & 0xff);

	if (flip)
	{
		for (INT32 k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl & ~1;
				xleft_inc = (dxldy >> 2) & ~1;
			}

			INT32 spix = k & 3;

			if (k >= yhclose)
			{
				invalid_y = k < yhlimit || k >= yllimit;

				INT32 j = k >> 2;
				const INT32 spanidx = j - (ycur >> 2);

				if (spix == 0)
				{
					maxxmx = 0;
					minxhx = 0xfff;
					allover = allunder = bothinvalid = true;
				}

				if (spix == 0 || k == ycur)
				{
					get_span_userdata(&spans[spanidx].userdata);

					rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
					if (j == invalid_index)
					{
						userdata->m_valid_line = false;
					}
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

					deduce_derivatives(userdata);
				}

				rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
				INT32 stickybit = ((xright >> 1) & 0x1fff) > 0 ? 1 : 0;
				UINT32 xsrc = ((xright >> 13) & 0x1ffe) | stickybit;
				bool curunder = (xright & 0x8000000) || xsrc < clipxhshift;
				xsrc = curunder ? clipxhshift : (((xright >> 13) & 0x3ffe) | stickybit);
				bool curover = (xsrc & 0x2000) != 0 || (xsrc & 0x1fff) > clipxlshift;
				xsrc = curover ? clipxlshift : xsrc;
				userdata->m_majorx[spix] = xsrc & 0x1fff;
				allover = allover && curover;
				allunder = allunder && curunder;

				stickybit = ((xleft >> 1) & 0x1fff) > 0 ? 1 : 0;
				UINT32 xlsc = ((xleft >> 13) & 0x1ffe) | stickybit;
				curunder = (xleft & 0x8000000) || xlsc < clipxhshift;
				xlsc = curunder ? clipxhshift : (((xleft >> 13) & 0x3ffe) | stickybit);
				curover = (xlsc & 0x2000) != 0 || (xlsc & 0x1fff) > clipxlshift;
				xlsc = curover ? clipxlshift : xlsc;
				userdata->m_minorx[spix] = xlsc & 0x1fff;
				allover = allover && curover;
				allunder = allunder && curunder;

				bool curcross = ((xleft ^ (1 << 27)) & (0x3fff << 14)) < ((xright ^ (1 << 27)) & (0x3fff << 14));

				invalid_y = invalid_y || curcross;
				userdata->m_invalid_y[spix] = invalid_y;
				bothinvalid = bothinvalid && invalid_y;

				if (!invalid_y)
				{
					maxxmx = (((xlsc >> 3) & 0xfff) > maxxmx) ? (xlsc >> 3) & 0xfff : maxxmx;
					minxhx = (((xsrc >> 3) & 0xfff) < minxhx) ? (xsrc >> 3) & 0xfff : minxhx;
				}

				if (spix == ldflag)
				{
					userdata->m_unscissored_rx = xright >> 16;
					xfrac = (xright >> 8) & 0xff;
					spans[spanidx].param[SPAN_R].start = ((r & ~0x1ff) + drdiff - (xfrac * drdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_G].start = ((g & ~0x1ff) + dgdiff - (xfrac * dgdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_B].start = ((b & ~0x1ff) + dbdiff - (xfrac * dbdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_A].start = ((a & ~0x1ff) + dadiff - (xfrac * dadxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_S].start = ((s & ~0x1ff) + dsdiff - (xfrac * dsdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_T].start = ((t & ~0x1ff) + dtdiff - (xfrac * dtdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_W].start = ((w & ~0x1ff) + dwdiff - (xfrac * dwdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_Z].start = ((z & ~0x1ff) + dzdiff - (xfrac * dzdxh)) & ~0x3ff;
				}

				if (spix == 3)
				{
					spans[spanidx].startx = maxxmx;
					spans[spanidx].stopx = minxhx;
					bool even_line = j & 1;
					userdata->m_valid_line = !bothinvalid && !allover && !allunder && (!m_scissor.m_field || (m_scissor.m_field && !(m_scissor.m_keep_odd ^ even_line)));
					if (!first_line && spans[spanidx-1].userdata != NULL)
					{
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_valid = true;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_s = spans[spanidx].param[SPAN_S].start;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_t = spans[spanidx].param[SPAN_T].start;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_w = spans[spanidx].param[SPAN_W].start;
					}
					else
					{
						first_line = false;
					}
					anyvalid = anyvalid || !bothinvalid;
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
	}
	else
	{
		for (INT32 k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl & ~1;
				xleft_inc = (dxldy >> 2) & ~1;
			}

			INT32 spix = k & 3;

			if (k >= yhclose)
			{
				bool invalid_y = k < yhlimit || k >= yllimit;

				INT32 j = k >> 2;
				const INT32 spanidx = j - (ycur >> 2);

				if (spix == 0)
				{
					maxxhx = 0;
					minxmx = 0xfff;
					allover = allunder = bothinvalid = true;
				}

				if (spix == 0 || k == ycur)
				{
					get_span_userdata(&spans[spanidx].userdata);

					rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
					if (j == invalid_index)
					{
						userdata->m_valid_line = false;
					}
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

					deduce_derivatives(userdata);
				}

				rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
				INT32 stickybit = ((xright >> 1) & 0x1fff) > 0 ? 1 : 0;
				UINT32 xsrc = ((xright >> 13) & 0x1ffe) | stickybit;
				bool curunder = (xright & 0x8000000) || xsrc < clipxhshift;
				xsrc = curunder ? clipxhshift : (((xright >> 13) & 0x3ffe) | stickybit);
				bool curover = (xsrc & 0x2000) != 0 || (xsrc & 0x1fff) > clipxlshift;
				xsrc = curover ? clipxlshift : xsrc;
				userdata->m_majorx[spix] = xsrc & 0x1fff;
				allover = allover && curover;
				allunder = allunder && curunder;

				stickybit = ((xleft >> 1) & 0x1fff) > 0 ? 1 : 0;
				UINT32 xlsc = ((xleft >> 13) & 0x1ffe) | stickybit;
				curunder = (xleft & 0x8000000) || xlsc < clipxhshift;
				xlsc = curunder ? clipxhshift : (((xleft >> 13) & 0x3ffe) | stickybit);
				curover = (xlsc & 0x2000) != 0 || (xlsc & 0x1fff) > clipxlshift;
				xlsc = curover ? clipxlshift : xlsc;
				userdata->m_minorx[spix] = xlsc & 0x1fff;
				allover = allover && curover;
				allunder = allunder && curunder;

				bool curcross = ((xright ^ (1 << 27)) & (0x3fff << 14)) < ((xleft ^ (1 << 27)) & (0x3fff << 14));

				invalid_y = invalid_y || curcross;
				userdata->m_invalid_y[spix] = invalid_y;
				bothinvalid = bothinvalid && invalid_y;

				if (!invalid_y)
				{
					minxmx = (((xlsc >> 3) & 0xfff) > minxmx) ? (xlsc >> 3) & 0xfff : minxmx;
					maxxhx = (((xsrc >> 3) & 0xfff) < maxxhx) ? (xsrc >> 3) & 0xfff : maxxhx;
				}

				if (spix == ldflag)
				{
					userdata->m_unscissored_rx = xright >> 16;
					xfrac = (xright >> 8) & 0xff;
					spans[spanidx].param[SPAN_R].start = ((r & ~0x1ff) + drdiff - (xfrac * drdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_G].start = ((g & ~0x1ff) + dgdiff - (xfrac * dgdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_B].start = ((b & ~0x1ff) + dbdiff - (xfrac * dbdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_A].start = ((a & ~0x1ff) + dadiff - (xfrac * dadxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_S].start = ((s & ~0x1ff) + dsdiff - (xfrac * dsdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_T].start = ((t & ~0x1ff) + dtdiff - (xfrac * dtdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_W].start = ((w & ~0x1ff) + dwdiff - (xfrac * dwdxh)) & ~0x3ff;
					spans[spanidx].param[SPAN_Z].start = ((z & ~0x1ff) + dzdiff - (xfrac * dzdxh)) & ~0x3ff;
				}

				if (spix == 3)
				{
					spans[spanidx].startx = minxmx;
					spans[spanidx].stopx = maxxhx;
					bool even_line = j & 1;
					userdata->m_valid_line = !bothinvalid && !allover && !allunder && (!m_scissor.m_field || (m_scissor.m_field && !(m_scissor.m_keep_odd ^ even_line)));
					if (!first_line && spans[spanidx-1].userdata != NULL)
					{
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_valid = true;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_s = spans[spanidx].param[SPAN_S].start;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_t = spans[spanidx].param[SPAN_T].start;
						((rdp_span_aux*)spans[spanidx-1].userdata)->m_next_w = spans[spanidx].param[SPAN_W].start;
					}
					else
					{
						first_line = false;
					}
					anyvalid = anyvalid || !bothinvalid;
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
	}
#if 0
	for (INT32 k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl & ~1;
			xleft_inc = (dxldy >> 2) & ~1;
		}

		const INT32 xstart = xleft >> 16;
		const INT32 xend = xright >> 16;
		const INT32 j = k >> 2;
		const INT32 spanidx = (k - ycur) >> 2;
		const INT32  spix = k & 3;
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
				valid = true;

				if(m_aux_buf_ptr >= EXTENT_AUX_COUNT)
				{
					fatalerror("n64_rdp::draw_triangle: span aux buffer overflow\n");
				}

				rdp_span_aux* userdata = (rdp_span_aux*)spans[spanidx].userdata;
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
				bool even_line = j & 1;
				userdata->m_valid_line = !allinvalid && !allover && !allunder && (!m_scissor.m_field || (m_scissor.m_field && !(m_scissor.m_keep_odd ^ even_line)));
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
#endif

	if(anyvalid)
	{
		render_spans(yhlimit >> 2, yllimit >> 2, tilenum, flip ? true : false, spans, rect, object);
	}

	//wait("draw_triangle");
}

/*****************************************************************************/

////////////////////////
// RDP COMMANDS
////////////////////////

void n64_rdp::triangle(bool shade, bool texture, bool zbuffer)
{
	draw_triangle(shade, texture, zbuffer, false);
	m_pipe_clean = false;
}

void n64_rdp::cmd_triangle(UINT32 w1, UINT32 w2)
{
	triangle(false, false, false);
}

void n64_rdp::cmd_triangle_z(UINT32 w1, UINT32 w2)
{
	triangle(false, false, true);
}

void n64_rdp::cmd_triangle_t(UINT32 w1, UINT32 w2)
{
	triangle(false, true, false);
}

void n64_rdp::cmd_triangle_tz(UINT32 w1, UINT32 w2)
{
	triangle(false, true, true);
}

void n64_rdp::cmd_triangle_s(UINT32 w1, UINT32 w2)
{
	triangle(true, false, false);
}

void n64_rdp::cmd_triangle_sz(UINT32 w1, UINT32 w2)
{
	triangle(true, false, true);
}

void n64_rdp::cmd_triangle_st(UINT32 w1, UINT32 w2)
{
	triangle(true, true, false);
}

void n64_rdp::cmd_triangle_stz(UINT32 w1, UINT32 w2)
{
	triangle(true, true, true);
}

void n64_rdp::cmd_tex_rect(UINT32 w1, UINT32 w2)
{
	const UINT32* data = m_cmd_data + m_cmd_cur;

	const UINT32 w3 = data[2];
	const UINT32 w4 = data[3];

	const INT32 tilenum  = (w2 >> 24) & 0x7;
	const INT32 xh   = (w2 >> 12) & 0xfff;
	const INT32 xl = (w1 >> 12) & 0xfff;
	const INT32 yh   = (w2 >>  0) & 0xfff;
	INT32 yl   = (w1 >>  0) & 0xfff;

	const INT32 s = (w3 >> 16) & 0xffff;
	const INT32 t = (w3 >>  0) & 0xffff;
	const INT32 dsdx = SIGN16((w4 >> 16) & 0xffff);
	const INT32 dtdy = SIGN16((w4 >>  0) & 0xffff);

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const INT32 xlint = (xl >> 2) & 0x3ff;
	const INT32 xhint = (xh >> 2) & 0x3ff;

	UINT32* ewdata = m_temp_rect_data;
	ewdata[0] = (0x24 << 24) | ((0x80 | tilenum) << 16) | yl;   // command, flipped, tile, yl
	ewdata[1] = (yl << 16) | yh;                                // ym, yh
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);               // xl, xl frac
	ewdata[3] = 0;                                              // dxldy, dxldy frac
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);               // xh, xh frac
	ewdata[5] = 0;                                              // dxhdy, dxhdy frac
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);               // xm, xm frac
	ewdata[7] = 0;                                              // dxmdy, dxmdy frac
	memset(&ewdata[8], 0, 16 * sizeof(UINT32));                 // shade
	ewdata[24] = (s << 16) | t;                                 // s, t
	ewdata[25] = 0;                                             // w
	ewdata[26] = ((dsdx >> 5) << 16);                           // dsdx, dtdx
	ewdata[27] = 0;                                             // dwdx
	ewdata[28] = 0;                                             // s frac, t frac
	ewdata[29] = 0;                                             // w frac
	ewdata[30] = ((dsdx & 0x1f) << 11) << 16;                   // dsdx frac, dtdx frac
	ewdata[31] = 0;                                             // dwdx frac
	ewdata[32] = (dtdy >> 5) & 0xffff;//dsde, dtde
	ewdata[33] = 0;//dwde
	ewdata[34] = (dtdy >> 5) & 0xffff;//dsdy, dtdy
	ewdata[35] = 0;//dwdy
	ewdata[36] = (dtdy & 0x1f) << 11;//dsde frac, dtde frac
	ewdata[37] = 0;//dwde frac
	ewdata[38] = (dtdy & 0x1f) << 11;//dsdy frac, dtdy frac
	ewdata[39] = 0;//dwdy frac
	memset(&ewdata[40], 0, 4 * sizeof(UINT32));//depth

	draw_triangle(true, true, false, true);
}

void n64_rdp::cmd_tex_rect_flip(UINT32 w1, UINT32 w2)
{
	const UINT32* data = m_cmd_data + m_cmd_cur;

	const UINT32 w3 = data[2];
	const UINT32 w4 = data[3];

	const INT32 tilenum  = (w2 >> 24) & 0x7;
	const INT32 xh   = (w2 >> 12) & 0xfff;
	const INT32 xl = (w1 >> 12) & 0xfff;
	const INT32 yh   = (w2 >>  0) & 0xfff;
	INT32 yl   = (w1 >>  0) & 0xfff;

	const INT32 s = (w3 >> 16) & 0xffff;
	const INT32 t = (w3 >>  0) & 0xffff;
	const INT32 dsdx = SIGN16((w4 >> 16) & 0xffff);
	const INT32 dtdy = SIGN16((w4 >>  0) & 0xffff);

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const INT32 xlint = (xl >> 2) & 0x3ff;
	const INT32 xhint = (xh >> 2) & 0x3ff;

	UINT32* ewdata = m_temp_rect_data;
	ewdata[0] = (0x25 << 24) | ((0x80 | tilenum) << 16) | yl;//command, flipped, tile, yl
	ewdata[1] = (yl << 16) | yh;//ym, yh
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);//xl, xl frac
	ewdata[3] = 0;//dxldy, dxldy frac
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);//xh, xh frac
	ewdata[5] = 0;//dxhdy, dxhdy frac
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);//xm, xm frac
	ewdata[7] = 0;//dxmdy, dxmdy frac
	memset(&ewdata[8], 0, 16 * sizeof(UINT32));//shade
	ewdata[24] = (s << 16) | t;//s, t
	ewdata[25] = 0;//w
	ewdata[26] = (dtdy >> 5) & 0xffff;//dsdx, dtdx
	ewdata[27] = 0;//dwdx
	ewdata[28] = 0;//s frac, t frac
	ewdata[29] = 0;//w frac
	ewdata[30] = ((dtdy & 0x1f) << 11);//dsdx frac, dtdx frac
	ewdata[31] = 0;//dwdx frac
	ewdata[32] = (dsdx >> 5) << 16;//dsde, dtde
	ewdata[33] = 0;//dwde
	ewdata[34] = (dsdx >> 5) << 16;//dsdy, dtdy
	ewdata[35] = 0;//dwdy
	ewdata[36] = (dsdx & 0x1f) << 27;//dsde frac, dtde frac
	ewdata[37] = 0;//dwde frac
	ewdata[38] = (dsdx & 0x1f) << 27;//dsdy frac, dtdy frac
	ewdata[39] = 0;//dwdy frac
	memset(&ewdata[40], 0, 4 * sizeof(UINT32));//depth

	draw_triangle(true, true, false, true);
}

void n64_rdp::cmd_sync_load(UINT32 w1, UINT32 w2)
{
	//wait("SyncLoad");
}

void n64_rdp::cmd_sync_pipe(UINT32 w1, UINT32 w2)
{
	//wait("SyncPipe");
}

void n64_rdp::cmd_sync_tile(UINT32 w1, UINT32 w2)
{
	//wait("SyncTile");
}

void n64_rdp::cmd_sync_full(UINT32 w1, UINT32 w2)
{
	//wait("SyncFull");
	dp_full_sync(*m_machine);
}

void n64_rdp::cmd_set_key_gb(UINT32 w1, UINT32 w2)
{
	m_key_scale.set_b(w2 & 0xff);
	m_key_scale.set_g((w2 >> 16) & 0xff);
}

void n64_rdp::cmd_set_key_r(UINT32 w1, UINT32 w2)
{
	m_key_scale.set_r(w2 & 0xff);
}

void n64_rdp::cmd_set_fill_color32(UINT32 w1, UINT32 w2)
{
	//wait("SetFillColor");
	m_fill_color = w2;
}

void n64_rdp::cmd_set_convert(UINT32 w1, UINT32 w2)
{
	if(!m_pipe_clean) { m_pipe_clean = true; wait("SetConvert"); }
	INT32 k0 = (w1 >> 13) & 0x1ff;
	INT32 k1 = (w1 >> 4) & 0x1ff;
	INT32 k2 = ((w1 & 0xf) << 5) | ((w2 >> 27) & 0x1f);
	INT32 k3 = (w2 >> 18) & 0x1ff;
	INT32 k4 = (w2 >> 9) & 0x1ff;
	INT32 k5 = w2 & 0x1ff;

	k0 = (SIGN9(k0) << 1) + 1;
	k1 = (SIGN9(k1) << 1) + 1;
	k2 = (SIGN9(k2) << 1) + 1;
	k3 = (SIGN9(k3) << 1) + 1;

	set_yuv_factors(rgbaint_t(0, k0, k2, 0), rgbaint_t(0, 0, k1, k3), rgbaint_t(k4, k4, k4, k4), rgbaint_t(k5, k5, k5, k5));
}

void n64_rdp::cmd_set_scissor(UINT32 w1, UINT32 w2)
{
	m_scissor.m_xh = (w1 >> 12) & 0xfff;
	m_scissor.m_yh = (w1 >>  0) & 0xfff;
	m_scissor.m_xl = (w2 >> 12) & 0xfff;
	m_scissor.m_yl = (w2 >>  0) & 0xfff;

	m_scissor.m_field = (w2 >> 25) & 1;
	m_scissor.m_keep_odd = (w2 >> 24) & 1;
}

void n64_rdp::cmd_set_prim_depth(UINT32 w1, UINT32 w2)
{
	m_misc_state.m_primitive_z = w2 & 0x7fff0000;
	m_misc_state.m_primitive_dz = (UINT16)(w1);
}

void n64_rdp::cmd_set_other_modes(UINT32 w1, UINT32 w2)
{
	//wait("SetOtherModes");
	m_other_modes.cycle_type       = (w1 >> 20) & 0x3; // 01
	m_other_modes.persp_tex_en     = (w1 & 0x80000) ? 1 : 0; // 1
	m_other_modes.detail_tex_en        = (w1 & 0x40000) ? 1 : 0; // 0
	m_other_modes.sharpen_tex_en   = (w1 & 0x20000) ? 1 : 0; // 0
	m_other_modes.tex_lod_en       = (w1 & 0x10000) ? 1 : 0; // 0
	m_other_modes.en_tlut          = (w1 & 0x08000) ? 1 : 0; // 0
	m_other_modes.tlut_type            = (w1 & 0x04000) ? 1 : 0; // 0
	m_other_modes.sample_type      = (w1 & 0x02000) ? 1 : 0; // 1
	m_other_modes.mid_texel            = (w1 & 0x01000) ? 1 : 0; // 0
	m_other_modes.bi_lerp0         = (w1 & 0x00800) ? 1 : 0; // 1
	m_other_modes.bi_lerp1         = (w1 & 0x00400) ? 1 : 0; // 1
	m_other_modes.convert_one      = (w1 & 0x00200) ? 1 : 0; // 0
	m_other_modes.key_en           = (w1 & 0x00100) ? 1 : 0; // 0
	m_other_modes.rgb_dither_sel   = (w1 >> 6) & 0x3; // 00
	m_other_modes.alpha_dither_sel = (w1 >> 4) & 0x3; // 01
	m_other_modes.blend_m1a_0      = (w2 >> 30) & 0x3; // 11
	m_other_modes.blend_m1a_1      = (w2 >> 28) & 0x3; // 00
	m_other_modes.blend_m1b_0      = (w2 >> 26) & 0x3; // 10
	m_other_modes.blend_m1b_1      = (w2 >> 24) & 0x3; // 00
	m_other_modes.blend_m2a_0      = (w2 >> 22) & 0x3; // 00
	m_other_modes.blend_m2a_1      = (w2 >> 20) & 0x3; // 01
	m_other_modes.blend_m2b_0      = (w2 >> 18) & 0x3; // 00
	m_other_modes.blend_m2b_1      = (w2 >> 16) & 0x3; // 01
	m_other_modes.force_blend      = (w2 >> 14) & 1; // 0
	m_other_modes.blend_shift       = m_other_modes.force_blend ? 5 : 2;
	m_other_modes.alpha_cvg_select = (w2 >> 13) & 1; // 1
	m_other_modes.cvg_times_alpha  = (w2 >> 12) & 1; // 0
	m_other_modes.z_mode           = (w2 >> 10) & 0x3; // 00
	m_other_modes.cvg_dest         = (w2 >> 8) & 0x3; // 00
	m_other_modes.color_on_cvg     = (w2 >> 7) & 1; // 0
	m_other_modes.image_read_en    = (w2 >> 6) & 1; // 1
	m_other_modes.z_update_en      = (w2 >> 5) & 1; // 1
	m_other_modes.z_compare_en     = (w2 >> 4) & 1; // 1
	m_other_modes.antialias_en     = (w2 >> 3) & 1; // 1
	m_other_modes.z_source_sel     = (w2 >> 2) & 1; // 0
	m_other_modes.dither_alpha_en  = (w2 >> 1) & 1; // 0
	m_other_modes.alpha_compare_en = (w2) & 1; // 0
	m_other_modes.alpha_dither_mode = (m_other_modes.alpha_compare_en << 1) | m_other_modes.dither_alpha_en;
}

void n64_rdp::cmd_load_tlut(UINT32 w1, UINT32 w2)
{
	//wait("LoadTLUT");
	n64_tile_t* tile = m_tiles;

	const INT32 tilenum = (w2 >> 24) & 0x7;
	const INT32 sl = tile[tilenum].sl = ((w1 >> 12) & 0xfff);
	const INT32 tl = tile[tilenum].tl =  w1 & 0xfff;
	const INT32 sh = tile[tilenum].sh = ((w2 >> 12) & 0xfff);
	const INT32 th = tile[tilenum].th = w2 & 0xfff;

	if (tl != th)
	{
		fatalerror("Load tlut: tl=%d, th=%d\n",tl,th);
	}

	const INT32 count = ((sh >> 2) - (sl >> 2) + 1) << 2;

	switch (m_misc_state.m_ti_size)
	{
		case PIXEL_SIZE_16BIT:
		{
			if (tile[tilenum].tmem < 256)
			{
				fatalerror("rdp_load_tlut: loading tlut into low half at %d qwords\n",tile[tilenum].tmem);
			}
			INT32 srcstart = (m_misc_state.m_ti_address + (tl >> 2) * (m_misc_state.m_ti_width << 1) + (sl >> 1)) >> 1;
			INT32 dststart = tile[tilenum].tmem << 2;
			UINT16* dst = get_tmem16();

			for (INT32 i = 0; i < count; i += 4)
			{
				if (dststart < 2048)
				{
					dst[dststart] = U_RREADIDX16(srcstart);
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

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_set_tile_size(UINT32 w1, UINT32 w2)
{
	//wait("SetTileSize");

	const INT32 tilenum = (w2 >> 24) & 0x7;

	m_tiles[tilenum].sl = (w1 >> 12) & 0xfff;
	m_tiles[tilenum].tl = (w1 >>  0) & 0xfff;
	m_tiles[tilenum].sh = (w2 >> 12) & 0xfff;
	m_tiles[tilenum].th = (w2 >>  0) & 0xfff;

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_load_block(UINT32 w1, UINT32 w2)
{
	//wait("LoadBlock");
	n64_tile_t* tile = m_tiles;

	const INT32 tilenum = (w2 >> 24) & 0x7;
	UINT16* tc = get_tmem16();

	INT32 sl = tile[tilenum].sl = ((w1 >> 12) & 0xfff);
	INT32 tl = tile[tilenum].tl = ((w1 >>  0) & 0xfff);
	INT32 sh = tile[tilenum].sh = ((w2 >> 12) & 0xfff);
	const INT32 dxt = ((w2 >>  0) & 0xfff);

	if (sh < sl)
	{
		fatalerror("load_block: sh < sl\n");
	}

	INT32 width = (sh - sl) + 1;

	width = (width << m_misc_state.m_ti_size) >> 1;
	if (width & 7)
	{
		width = (width & ~7) + 8;
	}
	width >>= 3;

	const INT32 tb = tile[tilenum].tmem << 2;

	const INT32 tiwinwords = (m_misc_state.m_ti_width << m_misc_state.m_ti_size) >> 2;
	const INT32 slinwords = (sl << m_misc_state.m_ti_size) >> 2;

	const UINT32 src = (m_misc_state.m_ti_address >> 1) + (tl * tiwinwords) + slinwords;

	if (dxt != 0)
	{
		INT32 j = 0;
		INT32 t = 0;
		INT32 oldt = 0;

		if (tile[tilenum].size != PIXEL_SIZE_32BIT && tile[tilenum].format != FORMAT_YUV)
		{
			for (INT32 i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				INT32 ptr = tb + (i << 2);
				INT32 srcptr = src + (i << 2);

				tc[(ptr ^ t) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 3);
				j += dxt;
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (INT32 i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				INT32 ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				INT32 srcptr = src + (i << 2);

				INT32 first = U_RREADIDX16(srcptr);
				INT32 sec = U_RREADIDX16(srcptr + 1);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				ptr = ((tb + (i << 1) + 1) ^ t) & 0x3ff;
				first = U_RREADIDX16(srcptr + 2);
				sec = U_RREADIDX16(srcptr + 3);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				j += dxt;
			}
		}
		else
		{
			for (INT32 i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
					i += tile[tilenum].line;

				INT32 ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				INT32 srcptr = src + (i << 2);
				tc[ptr] = U_RREADIDX16(srcptr);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 1);

				ptr = ((tb + (i << 1) + 1) ^ t) & 0x3ff;
				tc[ptr] = U_RREADIDX16(srcptr + 2);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 3);

				j += dxt;
			}
		}
		tile[tilenum].th = tl + (j >> 11);
	}
	else
	{
		if (tile[tilenum].size != PIXEL_SIZE_32BIT && tile[tilenum].format != FORMAT_YUV)
		{
			for (INT32 i = 0; i < width; i ++)
			{
				INT32 ptr = tb + (i << 2);
				INT32 srcptr = src + (i << 2);
				tc[(ptr ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 3);
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (INT32 i = 0; i < width; i ++)
			{
				INT32 ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				INT32 srcptr = src + (i << 2);
				INT32 first = U_RREADIDX16(srcptr);
				INT32 sec = U_RREADIDX16(srcptr + 1);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);//UV pair
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);

				ptr = ((tb + (i << 1) + 1) ^ WORD_ADDR_XOR) & 0x3ff;
				first = U_RREADIDX16(srcptr + 2);
				sec = U_RREADIDX16(srcptr + 3);
				tc[ptr] = ((first >> 8) << 8) | (sec >> 8);
				tc[ptr | 0x400] = ((first & 0xff) << 8) | (sec & 0xff);
			}
		}
		else
		{
			for (INT32 i = 0; i < width; i ++)
			{
				INT32 ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				INT32 srcptr = src + (i << 2);
				tc[ptr] = U_RREADIDX16(srcptr);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 1);

				ptr = ((tb + (i << 1) + 1) ^ WORD_ADDR_XOR) & 0x3ff;
				tc[ptr] = U_RREADIDX16(srcptr + 2);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 3);
			}
		}
		tile[tilenum].th = tl;
	}

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_load_tile(UINT32 w1, UINT32 w2)
{
	//wait("LoadTile");
	n64_tile_t* tile = m_tiles;
	const INT32 tilenum = (w2 >> 24) & 0x7;

	tile[tilenum].sl    = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl    = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh    = ((w2 >> 12) & 0xfff);
	tile[tilenum].th    = ((w2 >>  0) & 0xfff);

	const INT32 sl = tile[tilenum].sl >> 2;
	const INT32 tl = tile[tilenum].tl >> 2;
	const INT32 sh = tile[tilenum].sh >> 2;
	const INT32 th = tile[tilenum].th >> 2;

	const INT32 width = (sh - sl) + 1;
	const INT32 height = (th - tl) + 1;
/*
    INT32 topad;
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

	switch (m_misc_state.m_ti_size)
	{
		case PIXEL_SIZE_8BIT:
		{
			const UINT32 src = m_misc_state.m_ti_address;
			const INT32 tb = tile[tilenum].tmem << 3;
			UINT8* tc = get_tmem8();

			for (INT32 j = 0; j < height; j++)
			{
				const INT32 tline = tb + ((tile[tilenum].line << 3) * j);
				const INT32 s = ((j + tl) * m_misc_state.m_ti_width) + sl;
				const INT32 xorval8 = ((j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);

				for (INT32 i = 0; i < width; i++)
				{
					tc[((tline + i) ^ xorval8) & 0xfff] = U_RREADADDR8(src + s + i);
				}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			const UINT32 src = m_misc_state.m_ti_address >> 1;
			UINT16* tc = get_tmem16();

			if (tile[tilenum].format != FORMAT_YUV)
			{
				for (INT32 j = 0; j < height; j++)
				{
					const INT32 tb = tile[tilenum].tmem << 2;
					const INT32 tline = tb + ((tile[tilenum].line << 2) * j);
					const INT32 s = ((j + tl) * m_misc_state.m_ti_width) + sl;
					const INT32 xorval16 = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;

					for (INT32 i = 0; i < width; i++)
					{
						UINT32 taddr = (tline + i) ^ xorval16;
						tc[taddr & 0x7ff] = U_RREADIDX16(src + s + i);
					}
				}
			}
			else
			{
				for (INT32 j = 0; j < height; j++)
				{
					const INT32 tb = tile[tilenum].tmem << 3;
					const INT32 tline = tb + ((tile[tilenum].line << 3) * j);
					const INT32 s = ((j + tl) * m_misc_state.m_ti_width) + sl;
					const INT32 xorval8 = (j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;

					for (INT32 i = 0; i < width; i++)
					{
						UINT32 taddr = ((tline + i) ^ xorval8) & 0x7ff;
						UINT16 yuvword = U_RREADIDX16(src + s + i);
						get_tmem8()[taddr] = yuvword >> 8;
						get_tmem8()[taddr | 0x800] = yuvword & 0xff;
					}
				}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			const UINT32 src = m_misc_state.m_ti_address >> 2;
			const INT32 tb = (tile[tilenum].tmem << 2);
			UINT16* tc16 = get_tmem16();

			for (INT32 j = 0; j < height; j++)
			{
				const INT32 tline = tb + ((tile[tilenum].line << 2) * j);

				const INT32 s = ((j + tl) * m_misc_state.m_ti_width) + sl;
				const INT32 xorval32cur = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				for (INT32 i = 0; i < width; i++)
				{
					UINT32 c = U_RREADIDX32(src + s + i);
					UINT32 ptr = ((tline + i) ^ xorval32cur) & 0x3ff;
					tc16[ptr] = c >> 16;
					tc16[ptr | 0x400] = c & 0xffff;
				}
			}
			break;
		}

		default:    fatalerror("RDP: load_tile: size = %d\n", m_misc_state.m_ti_size);
	}

	m_tiles[tilenum].sth = rgbaint_t(m_tiles[tilenum].sh, m_tiles[tilenum].sh, m_tiles[tilenum].th, m_tiles[tilenum].th);
	m_tiles[tilenum].stl = rgbaint_t(m_tiles[tilenum].sl, m_tiles[tilenum].sl, m_tiles[tilenum].tl, m_tiles[tilenum].tl);
}

void n64_rdp::cmd_set_tile(UINT32 w1, UINT32 w2)
{
	//wait("SetTile");
	const INT32 tilenum = (w2 >> 24) & 0x7;
	n64_tile_t* tex_tile = &m_tiles[tilenum];

	tex_tile->format    = (w1 >> 21) & 0x7;
	tex_tile->size      = (w1 >> 19) & 0x3;
	tex_tile->line      = (w1 >>  9) & 0x1ff;
	tex_tile->tmem      = (w1 >>  0) & 0x1ff;
	tex_tile->palette   = (w2 >> 20) & 0xf;
	tex_tile->ct        = (w2 >> 19) & 0x1;
	tex_tile->mt        = (w2 >> 18) & 0x1;
	tex_tile->mask_t    = (w2 >> 14) & 0xf;
	tex_tile->shift_t   = (w2 >> 10) & 0xf;
	tex_tile->cs        = (w2 >>  9) & 0x1;
	tex_tile->ms        = (w2 >>  8) & 0x1;
	tex_tile->mask_s    = (w2 >>  4) & 0xf;
	tex_tile->shift_s   = (w2 >>  0) & 0xf;

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

void n64_rdp::cmd_fill_rect(UINT32 w1, UINT32 w2)
{
	//if(m_pending_mode_block) { wait("Block on pending mode-change"); m_pending_mode_block = false; }
	const UINT32 xh = (w2 >> 12) & 0xfff;
	const UINT32 xl = (w1 >> 12) & 0xfff;
	const UINT32 yh = (w2 >>  0) & 0xfff;
	UINT32 yl = (w1 >>  0) & 0xfff;

	if (m_other_modes.cycle_type == CYCLE_TYPE_FILL || m_other_modes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	const UINT32 xlint = (xl >> 2) & 0x3ff;
	const UINT32 xhint = (xh >> 2) & 0x3ff;

	UINT32* ewdata = m_temp_rect_data;
	ewdata[0] = (0x3680 << 16) | yl;//command, flipped, tile, yl
	ewdata[1] = (yl << 16) | yh;//ym, yh
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);//xl, xl frac
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);//xh, xh frac
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);//xm, xm frac
	memset(&ewdata[8], 0, 36 * sizeof(UINT32));//shade, texture, depth

	draw_triangle(false, false, false, true);
}

void n64_rdp::cmd_set_fog_color(UINT32 w1, UINT32 w2)
{
	m_fog_color.set(w2 & 0xff, (w2 >> 24) & 0xff, (w2 >> 16) & 0xff, (w2 >> 8) & 0xff);
}

void n64_rdp::cmd_set_blend_color(UINT32 w1, UINT32 w2)
{
	m_blend_color.set(w2 & 0xff, (w2 >> 24) & 0xff, (w2 >> 16) & 0xff, (w2 >> 8) & 0xff);
}

void n64_rdp::cmd_set_prim_color(UINT32 w1, UINT32 w2)
{
	m_misc_state.m_min_level = (w1 >> 8) & 0x1f;
	const UINT8 prim_lod_fraction = w1 & 0xff;
	m_prim_lod_fraction.set(prim_lod_fraction, prim_lod_fraction, prim_lod_fraction, prim_lod_fraction);

	m_prim_color.set(w2 & 0xff, (w2 >> 24) & 0xff, (w2 >> 16) & 0xff, (w2 >> 8) & 0xff);
	m_prim_alpha.set(w2 & 0xff, w2 & 0xff, w2 & 0xff, w2 & 0xff);
}

void n64_rdp::cmd_set_env_color(UINT32 w1, UINT32 w2)
{
	m_env_color.set(w2 & 0xff, (w2 >> 24) & 0xff, (w2 >> 16) & 0xff, (w2 >> 8) & 0xff);
	m_env_alpha.set(w2 & 0xff, w2 & 0xff, w2 & 0xff, w2 & 0xff);
}

void n64_rdp::cmd_set_combine(UINT32 w1, UINT32 w2)
{
	m_combine.sub_a_rgb0    = (w1 >> 20) & 0xf;
	m_combine.mul_rgb0      = (w1 >> 15) & 0x1f;
	m_combine.sub_a_a0      = (w1 >> 12) & 0x7;
	m_combine.mul_a0        = (w1 >>  9) & 0x7;
	m_combine.sub_a_rgb1    = (w1 >>  5) & 0xf;
	m_combine.mul_rgb1      = (w1 >>  0) & 0x1f;

	m_combine.sub_b_rgb0    = (w2 >> 28) & 0xf;
	m_combine.sub_b_rgb1    = (w2 >> 24) & 0xf;
	m_combine.sub_a_a1      = (w2 >> 21) & 0x7;
	m_combine.mul_a1        = (w2 >> 18) & 0x7;
	m_combine.add_rgb0      = (w2 >> 15) & 0x7;
	m_combine.sub_b_a0      = (w2 >> 12) & 0x7;
	m_combine.add_a0        = (w2 >>  9) & 0x7;
	m_combine.add_rgb1      = (w2 >>  6) & 0x7;
	m_combine.sub_b_a1      = (w2 >>  3) & 0x7;
	m_combine.add_a1        = (w2 >>  0) & 0x7;
}

void n64_rdp::cmd_set_texture_image(UINT32 w1, UINT32 w2)
{
	m_misc_state.m_ti_format  = (w1 >> 21) & 0x7;
	m_misc_state.m_ti_size    = (w1 >> 19) & 0x3;
	m_misc_state.m_ti_width   = (w1 & 0x3ff) + 1;
	m_misc_state.m_ti_address = w2 & 0x01ffffff;
}

void n64_rdp::cmd_set_mask_image(UINT32 w1, UINT32 w2)
{
	//wait("SetMaskImage");

	m_misc_state.m_zb_address = w2 & 0x01ffffff;
}

void n64_rdp::cmd_set_color_image(UINT32 w1, UINT32 w2)
{
	//wait("SetColorImage");

	m_misc_state.m_fb_format  = (w1 >> 21) & 0x7;
	m_misc_state.m_fb_size    = (w1 >> 19) & 0x3;
	m_misc_state.m_fb_width   = (w1 & 0x3ff) + 1;
	m_misc_state.m_fb_address = w2 & 0x01ffffff;

	if (m_misc_state.m_fb_format < 2 || m_misc_state.m_fb_format > 32) // Jet Force Gemini sets the format to 4, Intensity.  Protection?
	{
		m_misc_state.m_fb_format = 2;
	}
}

/*****************************************************************************/

void n64_rdp::cmd_invalid(UINT32 w1, UINT32 w2)
{
	fatalerror("n64_rdp::Invalid: %d, %08x %08x\n", (w1 >> 24) & 0x3f, w1, w2);
}

void n64_rdp::cmd_noop(UINT32 w1, UINT32 w2)
{
	// Do nothing
}


void n64_rdp::process_command_list()
{
	INT32 length = m_end - m_current;

	if(length < 0)
	{
		m_current = m_end;
		return;
	}

	// load command data
	for(INT32 i = 0; i < length; i += 4)
	{
		m_cmd_data[m_cmd_ptr++] = read_data((m_current & 0x1fffffff) + i);
	}

	m_current = m_end;

	UINT32 cmd = (m_cmd_data[0] >> 24) & 0x3f;
	UINT32 cmd_length = (m_cmd_ptr + 1) * 4;

	set_status(get_status() &~ DP_STATUS_FREEZE);

	// check if more data is needed
	if (cmd_length < s_rdp_command_length[cmd])
	{
		return;
	}

	while (m_cmd_cur < m_cmd_ptr)
	{
		cmd = (m_cmd_data[m_cmd_cur] >> 24) & 0x3f;

		if (((m_cmd_ptr - m_cmd_cur) * 4) < s_rdp_command_length[cmd])
		{
			return;
			//fatalerror("rdp_process_list: not enough rdp command data: cur = %d, ptr = %d, expected = %d\n", m_cmd_cur, m_cmd_ptr, s_rdp_command_length[cmd]);
		}

		if (LOG_RDP_EXECUTION)
		{
			char string[4000];
			disassemble(string);

			fprintf(rdp_exec, "%08X: %08X %08X   %s\n", m_start+(m_cmd_cur * 4), m_cmd_data[m_cmd_cur+0], m_cmd_data[m_cmd_cur+1], string);
			fflush(rdp_exec);
		}

		// execute the command
		UINT32 w1 = m_cmd_data[m_cmd_cur+0];
		UINT32 w2 = m_cmd_data[m_cmd_cur+1];

		switch(cmd)
		{
			case 0x00:  cmd_noop(w1, w2);           break;

			case 0x08:  cmd_triangle(w1, w2);       break;
			case 0x09:  cmd_triangle_z(w1, w2);     break;
			case 0x0a:  cmd_triangle_t(w1, w2);     break;
			case 0x0b:  cmd_triangle_tz(w1, w2);    break;
			case 0x0c:  cmd_triangle_s(w1, w2);     break;
			case 0x0d:  cmd_triangle_sz(w1, w2);    break;
			case 0x0e:  cmd_triangle_st(w1, w2);    break;
			case 0x0f:  cmd_triangle_stz(w1, w2);   break;

			case 0x24:  cmd_tex_rect(w1, w2);       break;
			case 0x25:  cmd_tex_rect_flip(w1, w2);  break;

			case 0x26:  cmd_sync_load(w1, w2);      break;
			case 0x27:  cmd_sync_pipe(w1, w2);      break;
			case 0x28:  cmd_sync_tile(w1, w2);      break;
			case 0x29:  cmd_sync_full(w1, w2);      break;

			case 0x2a:  cmd_set_key_gb(w1, w2);     break;
			case 0x2b:  cmd_set_key_r(w1, w2);      break;

			case 0x2c:  cmd_set_convert(w1, w2);    break;
			case 0x3c:  cmd_set_combine(w1, w2);    break;
			case 0x2d:  cmd_set_scissor(w1, w2);    break;
			case 0x2e:  cmd_set_prim_depth(w1, w2); break;
			case 0x2f:  cmd_set_other_modes(w1, w2); break;

			case 0x30:  cmd_load_tlut(w1, w2);      break;
			case 0x33:  cmd_load_block(w1, w2);     break;
			case 0x34:  cmd_load_tile(w1, w2);      break;

			case 0x32:  cmd_set_tile_size(w1, w2);  break;
			case 0x35:  cmd_set_tile(w1, w2);       break;

			case 0x36:  cmd_fill_rect(w1, w2);      break;

			case 0x37:  cmd_set_fill_color32(w1, w2); break;
			case 0x38:  cmd_set_fog_color(w1, w2);  break;
			case 0x39:  cmd_set_blend_color(w1, w2); break;
			case 0x3a:  cmd_set_prim_color(w1, w2); break;
			case 0x3b:  cmd_set_env_color(w1, w2);  break;

			case 0x3d:  cmd_set_texture_image(w1, w2); break;
			case 0x3e:  cmd_set_mask_image(w1, w2);  break;
			case 0x3f:  cmd_set_color_image(w1, w2); break;
		}

		m_cmd_cur += s_rdp_command_length[cmd] / 4;
	};
	m_cmd_ptr = 0;
	m_cmd_cur = 0;

	m_start = m_current = m_end;
}

/*****************************************************************************/

n64_rdp::n64_rdp(n64_state &state) : poly_manager<UINT32, rdp_poly_state, 8, 32000>(state.machine())
{
	ignore = false;
	dolog = false;
	m_aux_buf_ptr = 0;
	m_aux_buf = NULL;
	m_pipe_clean = true;

	m_pending_mode_block = false;

	m_cmd_ptr = 0;
	m_cmd_cur = 0;

	m_start = 0;
	m_end = 0;
	m_current = 0;
	m_status = 0x88;

	m_one.set(0xff, 0xff, 0xff, 0xff);
	m_zero.set(0, 0, 0, 0);

	m_tmem = NULL;

	m_machine = NULL;

	//memset(m_hidden_bits, 3, 8388608);

	m_prim_lod_fraction.set(0, 0, 0, 0);

	for (INT32 i = 0; i < 256; i++)
	{
		m_gamma_table[i] = sqrt((float)(i << 6));
		m_gamma_table[i] <<= 1;
	}

	for (INT32 i = 0; i < 0x4000; i++)
	{
		m_gamma_dither_table[i] = sqrt((float)i);
		m_gamma_dither_table[i] <<= 1;
	}

	z_build_com_table();

	for (INT32 i = 0; i < 0x4000; i++)
	{
		UINT32 exponent = (i >> 11) & 7;
		UINT32 mantissa = i & 0x7ff;
		m_z_complete_dec_table[i] = ((mantissa << m_z_dec_table[exponent].shift) + m_z_dec_table[exponent].add) & 0x3fffff;
	}

	precalc_cvmask_derivatives();

	for(INT32 i = 0; i < 0x200; i++)
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

	for(INT32 i = 0; i < 32; i++)
	{
		m_replicated_rgba[i] = (i << 3) | ((i >> 2) & 7);
	}

	for(INT32 i = 0; i < 0x10000; i++)
	{
		m_dzpix_normalize[i] = (UINT16)normalize_dzpix(i & 0xffff);
	}

	m_compute_cvg[0] = &n64_rdp::compute_cvg_noflip;
	m_compute_cvg[1] = &n64_rdp::compute_cvg_flip;
}

void n64_state::video_start()
{
	m_rdp = auto_alloc(machine(), n64_rdp(*this));

	m_rdp->set_machine(machine());
	m_rdp->init_internal_state();

	m_rdp->m_blender.set_machine(machine());
	m_rdp->m_blender.set_processor(m_rdp);

	m_rdp->m_tex_pipe.set_machine(machine());

	m_rdp->m_aux_buf = auto_alloc_array_clear(machine(), UINT8, EXTENT_AUX_COUNT);

	if (LOG_RDP_EXECUTION)
	{
		rdp_exec = fopen("rdp_execute.txt", "wt");
	}
}

void screen_eof_n64(screen_device &screen, bool state)
{
}

UINT32 n64_state::screen_update_n64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	n64_periphs* n64 = machine().device<n64_periphs>("rcp");
	m_rdp->m_visarea = screen.visible_area();

	//UINT16* frame_buffer = (UINT16*)&rdram[(n64->vi_origin & 0xffffff) >> 2];
	//UINT8* cvg_buffer = &m_rdp.m_hidden_bits[((n64->vi_origin & 0xffffff) >> 2) >> 1];
	//INT32 vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	//vibuffering = 0; // Disabled for now

	/*
	if (vibuffering && ((n64->vi_control & 3) == 2))
	{
	    if (frame_buffer)
	    {
	        for (j=0; j < vres; j++)
	        {
	            for (i=0; i < hres; i++)
	            {
	                UINT16 pix;
	                pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
	                curpixel_cvg = ((pix & 1) << 2) | (cvg_buffer[pixels ^ BYTE_ADDR_XOR] & 3); // Reuse of this variable
	                if (curpixel_cvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2) && fsaa)
	                {
	                    newc = video_filter16(&frame_buffer[pixels ^ WORD_ADDR_XOR], &cvg_buffer[pixels ^ BYTE_ADDR_XOR], n64->vi_width);
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

	m_rdp->wait();
	m_rdp->m_aux_buf_ptr = 0;

	if (n64->vi_blank)
	{
		bitmap.fill(0, m_rdp->m_visarea);
		return 0;
	}

	m_rdp->video_update(n64, bitmap);

	return 0;
}

void n64_state::screen_eof_n64(screen_device &screen, bool state)
{
	m_rdp->wait();
	m_rdp->m_aux_buf_ptr = 0;
}

void n64_rdp::render_spans(INT32 start, INT32 end, INT32 tilenum, bool flip, extent_t* spans, bool rect, rdp_poly_state* object)
{
	const INT32 clipy1 = m_scissor.m_yh >> 2;
	const INT32 clipy2 = m_scissor.m_yl >> 2;
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
	memcpy(&object->m_mode_derivs, &m_mode_derivs, sizeof(mode_derivs_t));
	memcpy(&object->m_tiles, &m_tiles, 8 * sizeof(n64_tile_t));
	object->tilenum = tilenum;
	object->flip = flip;
	object->m_fill_color = m_fill_color;
	object->rect = rect;

	//printf("%d", m_other_modes.cycle_type);
	switch(m_other_modes.cycle_type)
	{
		case CYCLE_TYPE_1:
			//printf("%d, %d\n", start, end);
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

void n64_rdp::rgbaz_correct_clip(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z, rdp_span_aux* userdata, const rdp_poly_state &object)
{
	if (userdata->m_current_pix_cvg == 8)
	{
		*r >>= 2;
		*g >>= 2;
		*b >>= 2;
		*a >>= 2;
		*z >>= 3;
	}
	else
	{
		const INT32 summand_r = offx * object.m_span_base.m_span_cdr + offy * object.m_span_base.m_span_drdy;
		const INT32 summand_g = offx * object.m_span_base.m_span_cdg + offy * object.m_span_base.m_span_dgdy;
		const INT32 summand_b = offx * object.m_span_base.m_span_cdb + offy * object.m_span_base.m_span_dbdy;
		const INT32 summand_a = offx * object.m_span_base.m_span_cda + offy * object.m_span_base.m_span_dady;
		const INT32 summand_z = offx * object.m_span_base.m_span_cdz + offy * object.m_span_base.m_span_dzdy;

		*r = ((*r << 2) + summand_r) >> 4;
		*g = ((*g << 2) + summand_g) >> 4;
		*b = ((*b << 2) + summand_b) >> 4;
		*a = ((*a << 2) + summand_a) >> 4;
		*z = ((*z << 2) + summand_z) >> 5;
	}

	userdata->m_shade_color.set(*r, *g, *b, *a);
	userdata->m_shade_color.clamp_and_clear(0xfffffe00);
	userdata->m_shade_alpha.set(*a, *a, *a, *a);

	const INT32 zanded = ((*z) & 0x60000) >> 17;

	switch(zanded)
	{
		case 0: *z &= 0x3ffff;	break;
		case 1: *z &= 0x3ffff;	break;
		case 2: *z = 0x3ffff;	break;
		case 3: *z = 0;			break;
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

	if (!userdata->m_valid_line)
	{
		return;
	}

	const INT32 tilenum = object.tilenum;
	const INT32 prim_tile = tilenum;
	INT32 tile1 = tilenum;
	INT32 newtile = tilenum;

	const bool flip = object.flip;

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
	userdata->m_dzpix_enc = dz_compress(dzpix);

	m_tex_pipe.calculate_clamp_diffs(tilenum, userdata, object);

	const bool partialreject = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_inv_pixel_color && userdata->m_color_inputs.blender1b_a[0] == &userdata->m_pixel_color);
	const INT32 sel0 = (userdata->m_color_inputs.blender2b_a[0] == &userdata->m_memory_color) ? 1 : 0;

	const INT32 fb_index = object.m_misc_state.m_fb_width * scanline;

	const INT32 xstart = extent.startx;
	const INT32 xend = userdata->m_unscissored_rx;
	const INT32 xend_scissored = extent.stopx;

	INT32 x = xend_scissored;

	INT32 length, scdiff;
	if (flip)
	{
		compute_cvg_flip(userdata, extent.startx, extent.stopx);
		length = xstart - xend_scissored;
		scdiff = xend_scissored - xend;
	}
	else
	{
		compute_cvg_noflip(userdata, extent.startx, extent.stopx);
		length = xend_scissored - xstart;
		scdiff = xend - xend_scissored;
	}

	if (scdiff)
	{
		r.w += (drinc * scdiff);
		g.w += (dginc * scdiff);
		b.w += (dbinc * scdiff);
		a.w += (dainc * scdiff);
		z.w += (dzinc * scdiff);
		s.w += (dsinc * scdiff);
		t.w += (dtinc * scdiff);
		w.w += (dwinc * scdiff);
	}

	if (object.m_misc_state.m_fb_size < 2 || object.m_misc_state.m_fb_size > 4)
		fatalerror("unsupported m_fb_size %d\n", object.m_misc_state.m_fb_size);

	const INT32 blend_index = (object.m_other_modes.alpha_cvg_select ? 2 : 0) | ((object.m_other_modes.rgb_dither_sel < 3) ? 1 : 0);
	const INT32 cycle0 = ((object.m_other_modes.sample_type & 1) << 1) | (object.m_other_modes.bi_lerp0 & 1);

	INT32 sss = 0;
	INT32 sst = 0;

	INT32 news, newt;
	INT32 prelodfrac;

	userdata->m_start_span = true;
	userdata->m_long_span = (length > 7);
	userdata->m_mid_span = (length == 7);
	userdata->m_almost_mid_span = (length == 6);

	INT32 nexts, nextt, nextw;

	for (INT32 j = 0; j <= length; j++)
	{
		INT32 sr = r.w >> 14;
		INT32 sg = g.w >> 14;
		INT32 sb = b.w >> 14;
		INT32 sa = a.w >> 14;
		INT32 ss = s.w >> 16;
		INT32 st = t.w >> 16;
		INT32 sw = w.w >> 16;
		INT32 sz = (z.w >> 10) & 0x3fffff;

		userdata->m_end_span = (j == length);
		userdata->m_pre_end_span = (j == (length - 1));

		UINT8 offx, offy;
		lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

		if (!userdata->m_end_span || !userdata->m_long_span || !userdata->m_next_valid)
		{
			nexts = (s.w + dsinc) >> 16;
			nextt = (t.w + dtinc) >> 16;
			nextw = (w.w + dwinc) >> 16;
		}
		else
		{
			nexts = userdata->m_next_s >> 16;
			nextt = userdata->m_next_t >> 16;
			nextw = userdata->m_next_w >> 16;
		}

		if (object.m_other_modes.persp_tex_en)
		{
			tc_div(nexts, nextt, nextw, &news, &newt);
		}
		else
		{
			tc_div_no_perspective(nexts, nextt, nextw, &news, &newt);
		}

		if (!userdata->m_start_span)
		{
			userdata->m_texel0_color.set(userdata->m_texel1_color);
			userdata->m_texel1_alpha.set(userdata->m_texel1_alpha);
			userdata->m_lod_fraction.set(prelodfrac, prelodfrac, prelodfrac, prelodfrac);
		}
		else
		{
			if (object.m_other_modes.persp_tex_en)
			{
				tc_div(ss, st, sw, &sss, &sst);
			}
			else
			{
				tc_div_no_perspective(ss, st, sw, &sss, &sst);
			}

			m_tex_pipe.tclod_1cycle_current(&sss, &sst, news, newt, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, userdata, object);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tilenum, 0, userdata, object);

			userdata->m_start_span = false;
		}

		userdata->m_next_span = userdata->m_end_span;
		userdata->m_end_span = userdata->m_pre_end_span;
		userdata->m_pre_end_span = (j == (length - 2));

		s.w += dsinc;
		t.w += dtinc;
		w.w += dwinc;

		m_tex_pipe.tclod_1cycle_next(&news, &newt, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &newtile, &prelodfrac, userdata, object);

		((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel1_color, &userdata->m_texel1_color, news, newt, tilenum, 0, userdata, object);

		rgbaz_correct_clip(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);

		UINT32 t0a = userdata->m_texel0_color.get_a();
		userdata->m_texel0_alpha.set(t0a, t0a, t0a, t0a);

		if (userdata->m_noise_used)
		{
			const UINT8 noise = rand() << 3; // Not accurate
			userdata->m_noise_color.set(0, noise, noise, noise);
		}

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
		userdata->m_pixel_color.set_a(get_alpha_cvg(userdata->m_pixel_color.get_a(), userdata, object));

		const UINT32 curpixel = fb_index + x;
		const UINT32 zbcur = zb + curpixel;
		const UINT32 zhbcur = zhb + curpixel;

		//printf("%d, %d\n", x, scanline);
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
					z_store(zbcur, zhbcur, sz, userdata->m_dzpix_enc);
				}
			}
		}

		sss = userdata->m_precomp_s;
		sst = userdata->m_precomp_t;

		r.w += drinc;
		g.w += dginc;
		b.w += dbinc;
		a.w += dainc;
		z.w += dzinc;

		x += xinc;
	}
}

void n64_rdp::span_draw_2cycle(INT32 scanline, const extent_t &extent, const rdp_poly_state &object, INT32 threadid)
{
	assert(object.m_misc_state.m_fb_size >= 2 && object.m_misc_state.m_fb_size < 4);

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
	INT32 newtile2 = tile2;

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

	INT32 length, scdiff;
	if (flip)
	{
		compute_cvg_flip(userdata, extent.startx, extent.stopx);
		length = xstart - xend_scissored;
		scdiff = xend_scissored - xend;
	}
	else
	{
		compute_cvg_noflip(userdata, extent.startx, extent.stopx);
		length = xend_scissored - xstart;
		scdiff = xend - xend_scissored;
	}

	if (scdiff)
	{
		r.w += (drinc * scdiff);
		g.w += (dginc * scdiff);
		b.w += (dbinc * scdiff);
		a.w += (dainc * scdiff);
		z.w += (dzinc * scdiff);
		s.w += (dsinc * scdiff);
		t.w += (dtinc * scdiff);
		w.w += (dwinc * scdiff);
	}

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

	INT32 news, newt;
	INT32 prelodfrac;

	userdata->m_start_span = true;
	userdata->m_long_span = (length > 7);
	userdata->m_mid_span = (length == 7);
	userdata->m_almost_mid_span = (length == 6);

	INT32 nexts, nextt, nextw;

	userdata->m_start_span = true;

	for (INT32 j = 0; j <= length; j++)
	{
		INT32 sr = r.w >> 14;
		INT32 sg = g.w >> 14;
		INT32 sb = b.w >> 14;
		INT32 sa = a.w >> 14;
		INT32 ss = s.w >> 16;
		INT32 st = t.w >> 16;
		INT32 sw = w.w >> 16;
		INT32 sz = (z.w >> 10) & 0x3fffff;

		userdata->m_end_span = (j == length);
		userdata->m_pre_end_span = (j == (length - 1));

		UINT8 offx, offy;
		lookup_cvmask_derivatives(userdata->m_cvg[x], &offx, &offy, userdata);

		nexts = (s.w + dsinc) >> 16;
		nextt = (t.w + dtinc) >> 16;
		nextw = (w.w + dwinc) >> 16;

		if (object.m_other_modes.persp_tex_en)
		{
			tc_div(nexts, nextt, nextw, &news, &newt);
		}
		else
		{
			tc_div_no_perspective(nexts, nextt, nextw, &news, &newt);
		}

		if (!userdata->m_start_span)
		{
			userdata->m_texel0_color.set(userdata->m_next_texel0_color);
			userdata->m_texel1_color.set(userdata->m_next_texel1_color);
			userdata->m_texel0_alpha.set(userdata->m_next_texel0_alpha);
			userdata->m_texel1_alpha.set(userdata->m_next_texel1_alpha);
			userdata->m_lod_fraction.set(prelodfrac, prelodfrac, prelodfrac, prelodfrac);
		}
		else
		{
			if (object.m_other_modes.persp_tex_en)
			{
				tc_div(ss, st, sw, &sss, &sst);
			}
			else
			{
				tc_div_no_perspective(ss, st, sw, &sss, &sst);
			}

			m_tex_pipe.tclod_2cycle_current(&sss, &sst, news, newt, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2, userdata, object);

			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_texel0_color, &userdata->m_texel0_color, sss, sst, tile1, 0, userdata, object);
			((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle1]))(&userdata->m_texel1_color, &userdata->m_texel0_color, sss, sst, tile2, 1, userdata, object);

			userdata->m_start_span = false;
		}

		s.w += dsinc;
		t.w += dtinc;
		w.w += dwinc;

		m_tex_pipe.tclod_2cycle_next(&news, &newt, s.w, t.w, w.w, dsinc, dtinc, dwinc, prim_tile, &newtile1, &newtile2, &prelodfrac, userdata, object);

		((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle0]))(&userdata->m_next_texel0_color, &userdata->m_texel0_color, news, newt, newtile1, 0, userdata, object);
		((m_tex_pipe).*(m_tex_pipe.m_cycle[cycle1]))(&userdata->m_next_texel1_color, &userdata->m_texel1_color, news, newt, newtile2, 1, userdata, object);

		rgbaz_correct_clip(offx, offy, &sr, &sg, &sb, &sa, &sz, userdata, object);

		if (userdata->m_noise_used)
		{
			const UINT8 noise = rand() << 3; // Not accurate
			userdata->m_noise_color.set(0, noise, noise, noise);
		}

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
		userdata->m_texel1_color.set(userdata->m_next_texel0_color);

		UINT32 ca = userdata->m_combined_color.get_a();
		userdata->m_combined_alpha.set(ca, ca, ca, ca);
		userdata->m_texel0_alpha.set(userdata->m_texel1_alpha);
		userdata->m_texel1_alpha.set(userdata->m_next_texel0_alpha);

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
					z_store(zbcur, zhbcur, sz, userdata->m_dzpix_enc);
				}
			}
		}
		sss = userdata->m_precomp_s;
		sst = userdata->m_precomp_t;

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
	const INT32 clipx1 = object.m_scissor.m_xh >> 2;
	const INT32 clipx2 = object.m_scissor.m_xl >> 2;
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

	const INT32 clipx1 = object.m_scissor.m_xh >> 2;
	const INT32 clipx2 = object.m_scissor.m_xl >> 2;

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
