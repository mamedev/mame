/******************************************************************************


    SGI/Nintendo Reality Display Processor
    -------------------

    Initial revision by Ville Linde
    Many improvements by Harmony, angrylion, Ziggy, Gonetz and Orkin

    Class re-write by Harmony


*******************************************************************************

STATUS:

Much behavior needs verification against real hardware.  Many literal edge
cases must be verified on real hardware as well.

TODO:

- Further re-work class structure to avoid dependencies

*******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

#define LOG_RDP_EXECUTION		0

static FILE *rdp_exec;

bool n64_rdp::rdp_range_check(UINT32 addr)
{
	if(MiscState.FBSize == 0) return false;

	int fbcount = ((MiscState.FBWidth * Scissor.m_yl) << (MiscState.FBSize - 1)) * 3;
	int zbcount = MiscState.FBWidth * Scissor.m_yl * 2;
	int fbaddr = MiscState.FBAddress & 0x007fffff;
	int zbaddr = MiscState.ZBAddress & 0x007fffff;
	if ((addr >= fbaddr) && (addr < (fbaddr + fbcount)))
	{
		return false;
	}
	if ((addr >= zbaddr) && (addr < (zbaddr + zbcount)))
	{
		return false;
	}

	printf("Check failed: %08x vs. %08x-%08x, %08x-%08x (%d, %d)\n", addr, fbaddr, fbaddr + fbcount, zbaddr, zbaddr + zbcount, MiscState.FBWidth, Scissor.m_yl);
	fflush(stdout);
	return true;
}

/*****************************************************************************/

// The functions in this file should be moved into the parent Processor class.
#include "rdpfiltr.c"

void n64_rdp::GetAlphaCvg(UINT8 *comb_alpha, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	INT32 temp = *comb_alpha;
	INT32 temp2 = userdata->CurrentPixCvg;
	INT32 temp3 = 0;

	if (object.OtherModes.cvg_times_alpha)
	{
		temp3 = (temp * temp2) + 4;
		userdata->CurrentPixCvg = (temp3 >> 8) & 0xf;
	}
	if (object.OtherModes.alpha_cvg_select)
	{
		temp = (OtherModes.cvg_times_alpha) ? (temp3 >> 3) : (temp2 << 5);
	}
	if (temp > 0xff)
	{
		temp = 0xff;
	}
	*comb_alpha = temp;
}

/*****************************************************************************/

void n64_rdp::VideoUpdate(n64_periphs *n64, bitmap_rgb32 &bitmap)
{
	switch(n64->vi_control & 0x3)
	{
		case PIXEL_SIZE_16BIT:
			VideoUpdate16(n64, bitmap);
			break;

		case PIXEL_SIZE_32BIT:
			VideoUpdate32(n64, bitmap);
			break;

		default:
			//fatalerror("Unsupported framebuffer depth: m_fb_size=%d\n", MiscState.FBSize);
			break;
	}
}

void n64_rdp::VideoUpdate16(n64_periphs *n64, bitmap_rgb32 &bitmap)
{
    //int fsaa = (((n64->vi_control >> 8) & 3) < 2);
    //int divot = (n64->vi_control >> 4) & 1;

	//UINT32 prev_cvg = 0;
	//UINT32 next_cvg = 0;
    //int dither_filter = (n64->vi_control >> 16) & 1;
    //int vibuffering = ((n64->vi_control & 2) && fsaa && divot);

	UINT16 *frame_buffer = (UINT16*)&rdram[(n64->vi_origin & 0xffffff) >> 2];
	//UINT32 hb = ((n64->vi_origin & 0xffffff) >> 2) >> 1;
	//UINT8* hidden_buffer = &HiddenBits[hb];

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
		for(int j = 0; j < vres; j++)
		{
			UINT32 *d = &bitmap.pix32(j);

			for(int i = 0; i < hres; i++)
			{
				Color c;
				//int r, g, b;

				UINT16 pix = frame_buffer[pixels ^ WORD_ADDR_XOR];
				//MiscState.CurrentPixCvg = ((pix & 1) << 2) | (hidden_buffer[pixels ^ BYTE_ADDR_XOR] & 3);

				//if(divot)
				//{
				//  if(i > 0 && i < (hres - 1))
				//  {
				//      prev_cvg = ((frame_buffer[(pixels - 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels - 1)^BYTE_ADDR_XOR] & 3);
				//      next_cvg = ((frame_buffer[(pixels + 1)^WORD_ADDR_XOR] & 1) << 2) | (hidden_buffer[(pixels + 1)^BYTE_ADDR_XOR] & 3);
				//  }
				//}
				c.i.r = ((pix >> 8) & 0xf8) | (pix >> 13);
				c.i.g = ((pix >> 3) & 0xf8) | ((pix >>  8) & 0x07);
				c.i.b = ((pix << 2) & 0xf8) | ((pix >>  3) & 0x07);

				//if(fsaa)
				//{
					//if (/*!vibuffering &&*/ state->m_rdp.MiscState.CurrentPixCvg < 7 && i > 1 && j > 1 && i < (hres - 2) && j < (vres - 2))
					//{
						//video_filter16(&c.i.r, &c.i.g, &c.i.b, &frame_buffer[pixels ^ WORD_ADDR_XOR],&hidden_buffer[pixels ^ BYTE_ADDR_XOR], n64->vi_width);
					//}
				//}
				//else if (dither_filter && state->m_rdp.MiscState.CurrentPixCvg == 7 && i > 0 && j > 0 && i < (hres - 1) && j < (vres - 1))
				//{
					//if (vibuffering)
					//{
					//  restore_filter16_buffer(&r, &g, &b, &ViBuffer[i][j], n64->vi_width);
					//}
					//else
					//{
						//restore_filter16(&c.i.r, &c.i.g, &c.i.b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR, n64->vi_width);
					//}
				//}
				//if(divot)
				//{
					//if (i > 0 && i < (hres - 1) && (MiscState.CurrentPixCvg != 7 || prev_cvg != 7 || next_cvg != 7))
					//{
						//if (vibuffering)
						//{
						//  divot_filter16_buffer(&r, &g, &b, &ViBuffer[i][j]);
						//}
						//else
						//{
							//divot_filter16(&c.i.r, &c.i.g, &c.i.b, &frame_buffer[pixels ^ WORD_ADDR_XOR], pixels ^ WORD_ADDR_XOR);
						//}
					//}
				//}

				/*
                if (gamma_dither)
                {
                    dith = screen.machine().rand() & 0x3f;
                }
                if (gamma)
                {
                    if (gamma_dither)
                    {
                        r = m_gamma_dither_table[(r << 6)|dith];
                        g = m_gamma_dither_table[(g << 6)|dith];
                        b = m_gamma_dither_table[(b << 6)|dith];
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
                */
				pixels++;

				d[i] = c.c >> 8;//(r << 16) | (g << 8) | b; // Fix me for endianness
			}
			pixels +=invisiblewidth;
		}
	}
}

void n64_rdp::VideoUpdate32(n64_periphs *n64, bitmap_rgb32 &bitmap)
{
    int gamma = (n64->vi_control >> 3) & 1;
    int gamma_dither = (n64->vi_control >> 2) & 1;
    //int vibuffering = ((n64->vi_control & 2) && fsaa && divot);

    UINT32 *frame_buffer32 = (UINT32*)&rdram[(n64->vi_origin & 0xffffff) >> 2];

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
		for (int j = 0; j < vres; j++)
		{
			UINT32 *d = &bitmap.pix32(j);
			for (int i = 0; i < hres; i++)
			{
				UINT32 pix = *frame_buffer32++;
				if (gamma || gamma_dither)
				{
					int r = (pix >> 24) & 0xff;
					int g = (pix >> 16) & 0xff;
					int b = (pix >> 8) & 0xff;
					int dith = 0;
					if (gamma_dither)
					{
						dith = GetRandom() & 0x3f;
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

void n64_rdp::TCDivNoPersp(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst)
{
	*sss = (SIGN16(ss)) & 0x1ffff;
	*sst = (SIGN16(st)) & 0x1ffff;
}

void n64_rdp::TCDiv(INT32 ss, INT32 st, INT32 sw, INT32* sss, INT32* sst)
{
	int w_carry = 0;
	if ((sw & 0x8000) || !(sw & 0x7fff))
	{
		w_carry = 1;
	}

	sw &= 0x7fff;

	int shift;
	for (shift = 1; shift <= 14 && !((sw << shift) & 0x8000); shift++);
	shift -= 1;

	int normout = (sw << shift) & 0x3fff;
	int wnorm = (normout & 0xff) << 2;
	normout >>= 8;

	int temppoint = m_norm_point_rom[normout];
	int tempslope = m_norm_slope_rom[normout];

	int tlu_rcp = ((-(tempslope * wnorm)) >> 10) + temppoint;

	int sprod = SIGN16(ss) * tlu_rcp;
	int tprod = SIGN16(st) * tlu_rcp;
	int tempmask = ((1 << (shift + 1)) - 1) << (29 - shift);
	int shift_value = 13 - shift;

	int outofbounds_s = sprod & tempmask;
	int outofbounds_t = tprod & tempmask;
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
	int under_s = 0;
	int under_t = 0;
	int over_s = 0;
	int over_t = 0;

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

INT32 n64_rdp::ColorCombinerEquation(INT32 a, INT32 b, INT32 c, INT32 d)
{
	a = KURT_AKELEY_SIGN9(a);
	b = KURT_AKELEY_SIGN9(b);
	c = SIGN9(c);
	d = KURT_AKELEY_SIGN9(d);
	a = (((a - b) * c) + (d << 8) + 0x80);
	a = SIGN17(a) >> 8;
	a = m_special_9bit_clamptable[a & 0x1ff];
	return a;
}

INT32 n64_rdp::AlphaCombinerEquation(INT32 a, INT32 b, INT32 c, INT32 d)
{
	a = KURT_AKELEY_SIGN9(a);
	b = KURT_AKELEY_SIGN9(b);
	c = SIGN9(c);
	d = KURT_AKELEY_SIGN9(d);
	a = (((a - b) * c) + (d << 8) + 0x80) >> 8;
	a = SIGN9(a);
	a = m_special_9bit_clamptable[a & 0x1ff];
	return a;
}

void n64_rdp::ColorCombiner1Cycle(rdp_span_aux *userdata)
{
	userdata->NoiseColor.i.r = userdata->NoiseColor.i.g = userdata->NoiseColor.i.b = GetRandom() << 3; // Not accurate

	userdata->PixelColor.i.r = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_r[1],*userdata->ColorInputs.combiner_rgbsub_b_r[1],*userdata->ColorInputs.combiner_rgbmul_r[1],*userdata->ColorInputs.combiner_rgbadd_r[1]);
	userdata->PixelColor.i.g = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_g[1],*userdata->ColorInputs.combiner_rgbsub_b_g[1],*userdata->ColorInputs.combiner_rgbmul_g[1],*userdata->ColorInputs.combiner_rgbadd_g[1]);
	userdata->PixelColor.i.b = ColorCombinerEquation(*userdata->ColorInputs.combiner_rgbsub_a_b[1],*userdata->ColorInputs.combiner_rgbsub_b_b[1],*userdata->ColorInputs.combiner_rgbmul_b[1],*userdata->ColorInputs.combiner_rgbadd_b[1]);
	userdata->PixelColor.i.a = AlphaCombinerEquation(*userdata->ColorInputs.combiner_alphasub_a[1],*userdata->ColorInputs.combiner_alphasub_b[1],*userdata->ColorInputs.combiner_alphamul[1],*userdata->ColorInputs.combiner_alphaadd[1]);
}

void n64_rdp::ColorCombiner2Cycle(rdp_span_aux *userdata)
{
	userdata->NoiseColor.i.r = userdata->NoiseColor.i.g = userdata->NoiseColor.i.b = GetRandom() << 3; // Not accurate
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
}

void n64_rdp::SetSubAInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code, rdp_span_aux *userdata)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &userdata->CombinedColor.i.r;	*input_g = &userdata->CombinedColor.i.g;	*input_b = &userdata->CombinedColor.i.b;	break;
		case 1:		*input_r = &userdata->Texel0Color.i.r;		*input_g = &userdata->Texel0Color.i.g;		*input_b = &userdata->Texel0Color.i.b;		break;
		case 2:		*input_r = &userdata->Texel1Color.i.r;		*input_g = &userdata->Texel1Color.i.g;		*input_b = &userdata->Texel1Color.i.b;		break;
		case 3:		*input_r = &userdata->PrimColor.i.r;		*input_g = &userdata->PrimColor.i.g;		*input_b = &userdata->PrimColor.i.b;		break;
		case 4:		*input_r = &userdata->ShadeColor.i.r;		*input_g = &userdata->ShadeColor.i.g;		*input_b = &userdata->ShadeColor.i.b;		break;
		case 5:		*input_r = &userdata->EnvColor.i.r;			*input_g = &userdata->EnvColor.i.g;			*input_b = &userdata->EnvColor.i.b;			break;
		case 6:		*input_r = &OneColor.i.r;					*input_g = &OneColor.i.g;					*input_b = &OneColor.i.b;					break;
		case 7:		*input_r = &userdata->NoiseColor.i.r;		*input_g = &userdata->NoiseColor.i.g;		*input_b = &userdata->NoiseColor.i.b;		break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
					*input_r = &ZeroColor.i.r;		*input_g = &ZeroColor.i.g;		*input_b = &ZeroColor.i.b;		break;
		}
	}
}

void n64_rdp::SetSubBInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code, rdp_span_aux *userdata)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &userdata->CombinedColor.i.r;	*input_g = &userdata->CombinedColor.i.g;	*input_b = &userdata->CombinedColor.i.b;	break;
		case 1:		*input_r = &userdata->Texel0Color.i.r;		*input_g = &userdata->Texel0Color.i.g;		*input_b = &userdata->Texel0Color.i.b;		break;
		case 2:		*input_r = &userdata->Texel1Color.i.r;		*input_g = &userdata->Texel1Color.i.g;		*input_b = &userdata->Texel1Color.i.b;		break;
		case 3:		*input_r = &userdata->PrimColor.i.r;		*input_g = &userdata->PrimColor.i.g;		*input_b = &userdata->PrimColor.i.b;		break;
		case 4:		*input_r = &userdata->ShadeColor.i.r;		*input_g = &userdata->ShadeColor.i.g;		*input_b = &userdata->ShadeColor.i.b;		break;
		case 5:		*input_r = &userdata->EnvColor.i.r;			*input_g = &userdata->EnvColor.i.g;			*input_b = &userdata->EnvColor.i.b;			break;
		case 6:		fatalerror("SET_SUBB_RGB_INPUT: key_center\n");	break;
		case 7:		*input_r = (UINT8*)&m_k4;		*input_g = (UINT8*)&m_k4;		*input_b = (UINT8*)&m_k4;		break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
					*input_r = &ZeroColor.i.r;		*input_g = &ZeroColor.i.g;		*input_b = &ZeroColor.i.b;		break;
		}
	}
}

void n64_rdp::SetMulInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code, rdp_span_aux *userdata)
{
	switch (code & 0x1f)
	{
		case 0:		*input_r = &userdata->CombinedColor.i.r;	*input_g = &userdata->CombinedColor.i.g;	*input_b = &userdata->CombinedColor.i.b;	break;
		case 1:		*input_r = &userdata->Texel0Color.i.r;		*input_g = &userdata->Texel0Color.i.g;		*input_b = &userdata->Texel0Color.i.b;		break;
		case 2:		*input_r = &userdata->Texel1Color.i.r;		*input_g = &userdata->Texel1Color.i.g;		*input_b = &userdata->Texel1Color.i.b;		break;
		case 3:		*input_r = &userdata->PrimColor.i.r;		*input_g = &userdata->PrimColor.i.g;		*input_b = &userdata->PrimColor.i.b;		break;
		case 4:		*input_r = &userdata->ShadeColor.i.r;		*input_g = &userdata->ShadeColor.i.g;		*input_b = &userdata->ShadeColor.i.b;		break;
		case 5:		*input_r = &userdata->EnvColor.i.r;			*input_g = &userdata->EnvColor.i.g;			*input_b = &userdata->EnvColor.i.b;			break;
		case 6:		*input_r = &userdata->KeyScale.i.r;			*input_g = &userdata->KeyScale.i.g;			*input_b = &userdata->KeyScale.i.b;			break;
		case 7:		*input_r = &userdata->CombinedColor.i.a;	*input_g = &userdata->CombinedColor.i.a;	*input_b = &userdata->CombinedColor.i.a;	break;
		case 8:		*input_r = &userdata->Texel0Color.i.a;		*input_g = &userdata->Texel0Color.i.a;		*input_b = &userdata->Texel0Color.i.a;		break;
		case 9:		*input_r = &userdata->Texel1Color.i.a;		*input_g = &userdata->Texel1Color.i.a;		*input_b = &userdata->Texel1Color.i.a;		break;
		case 10:	*input_r = &userdata->PrimColor.i.a;		*input_g = &userdata->PrimColor.i.a;		*input_b = &userdata->PrimColor.i.a;		break;
		case 11:	*input_r = &userdata->ShadeColor.i.a;		*input_g = &userdata->ShadeColor.i.a;		*input_b = &userdata->ShadeColor.i.a;		break;
		case 12:	*input_r = &userdata->EnvColor.i.a;			*input_g = &userdata->EnvColor.i.a;			*input_b = &userdata->EnvColor.i.a;			break;
		case 13:	*input_r = &userdata->LODFraction;			*input_g = &userdata->LODFraction;			*input_b = &userdata->LODFraction;			break;
		case 14:	*input_r = &userdata->PrimLODFraction;		*input_g = &userdata->PrimLODFraction;		*input_b = &userdata->PrimLODFraction;		break;
		case 15:	*input_r = (UINT8*)&m_k5;					*input_g = (UINT8*)&m_k5;					*input_b = (UINT8*)&m_k5;					break;
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
		{
					*input_r = &ZeroColor.i.r;		*input_g = &ZeroColor.i.g;		*input_b = &ZeroColor.i.b;		break;
		}
	}
}

void n64_rdp::SetAddInputRGB(UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, int code, rdp_span_aux *userdata)
{
	switch (code & 0x7)
	{
		case 0:		*input_r = &userdata->CombinedColor.i.r;	*input_g = &userdata->CombinedColor.i.g;	*input_b = &userdata->CombinedColor.i.b;	break;
		case 1:		*input_r = &userdata->Texel0Color.i.r;		*input_g = &userdata->Texel0Color.i.g;		*input_b = &userdata->Texel0Color.i.b;		break;
		case 2:		*input_r = &userdata->Texel1Color.i.r;		*input_g = &userdata->Texel1Color.i.g;		*input_b = &userdata->Texel1Color.i.b;		break;
		case 3:		*input_r = &userdata->PrimColor.i.r;		*input_g = &userdata->PrimColor.i.g;		*input_b = &userdata->PrimColor.i.b;		break;
		case 4:		*input_r = &userdata->ShadeColor.i.r;		*input_g = &userdata->ShadeColor.i.g;		*input_b = &userdata->ShadeColor.i.b;		break;
		case 5:		*input_r = &userdata->EnvColor.i.r;			*input_g = &userdata->EnvColor.i.g;			*input_b = &userdata->EnvColor.i.b;			break;
		case 6:		*input_r = &OneColor.i.r;					*input_g = &OneColor.i.g;					*input_b = &OneColor.i.b;					break;
		case 7:		*input_r = &ZeroColor.i.r;					*input_g = &ZeroColor.i.g;					*input_b = &ZeroColor.i.b;					break;
	}
}

void n64_rdp::SetSubInputAlpha(UINT8 **input, int code, rdp_span_aux *userdata)
{
	switch (code & 0x7)
	{
		case 0:		*input = &userdata->CombinedColor.i.a; break;
		case 1:		*input = &userdata->Texel0Color.i.a; break;
		case 2:		*input = &userdata->Texel1Color.i.a; break;
		case 3:		*input = &userdata->PrimColor.i.a; break;
		case 4:		*input = &userdata->ShadeColor.i.a; break;
		case 5:		*input = &userdata->EnvColor.i.a; break;
		case 6:		*input = &OneColor.i.a; break;
		case 7:		*input = &ZeroColor.i.a; break;
	}
}

void n64_rdp::SetMulInputAlpha(UINT8 **input, int code, rdp_span_aux *userdata)
{
	switch (code & 0x7)
	{
		case 0:		*input = &userdata->LODFraction; break;
		case 1:		*input = &userdata->Texel0Color.i.a; break;
		case 2:		*input = &userdata->Texel1Color.i.a; break;
		case 3:		*input = &userdata->PrimColor.i.a; break;
		case 4:		*input = &userdata->ShadeColor.i.a; break;
		case 5:		*input = &userdata->EnvColor.i.a; break;
		case 6:		*input = &userdata->PrimLODFraction; break;
		case 7:		*input = &ZeroColor.i.a; break;
	}
}

void n64_rdp::SetBlenderInput(int cycle, int which, UINT8 **input_r, UINT8 **input_g, UINT8 **input_b, UINT8 **input_a, int a, int b, rdp_span_aux *userdata)
{
	switch (a & 0x3)
	{
		case 0:
		{
			if (cycle == 0)
			{
				*input_r = &userdata->PixelColor.i.r;
				*input_g = &userdata->PixelColor.i.g;
				*input_b = &userdata->PixelColor.i.b;
			}
			else
			{
				*input_r = &userdata->BlendedPixelColor.i.r;
				*input_g = &userdata->BlendedPixelColor.i.g;
				*input_b = &userdata->BlendedPixelColor.i.b;
			}
			break;
		}

		case 1:
		{
			*input_r = &userdata->MemoryColor.i.r;
			*input_g = &userdata->MemoryColor.i.g;
			*input_b = &userdata->MemoryColor.i.b;
			break;
		}

		case 2:
		{
			*input_r = &userdata->BlendColor.i.r;
			*input_g = &userdata->BlendColor.i.g;
			*input_b = &userdata->BlendColor.i.b;
			break;
		}

		case 3:
		{
			*input_r = &userdata->FogColor.i.r;
			*input_g = &userdata->FogColor.i.g;
			*input_b = &userdata->FogColor.i.b;
			break;
		}
	}

	if (which == 0)
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &userdata->PixelColor.i.a; break;
			case 1:		*input_a = &userdata->FogColor.i.a; break;
			case 2:		*input_a = &userdata->ShadeColor.i.a; break;
			case 3:		*input_a = &ZeroColor.i.a; break;
		}
	}
	else
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &userdata->InvPixelColor.i.a; break;
			case 1:		*input_a = &userdata->MemoryColor.i.a; break;
			case 2:		*input_a = &OneColor.i.a; break;
			case 3:		*input_a = &ZeroColor.i.a; break;
		}
	}
}

const UINT8 n64_rdp::s_bayer_matrix[16] =
{ /* Bayer matrix */
	 0,  4,  1, 5,
	 6,  2,  7, 3,
	 1,	 5,  0, 4,
	 7,  3,  6, 2
};

const UINT8 n64_rdp::s_magic_matrix[16] =
{ /* Magic square matrix */
	 0,  6,  1, 7,
	 4,  2,  5, 3,
	 3,	 5,  2, 4,
	 7,  1,  6, 0
};

const n64_rdp::ZDecompressEntry n64_rdp::z_dec_table[8] =
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
	for(int z = 0; z < 0x40000; z++)
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

    z_com_table[z] = altmem;

    }
}

void n64_rdp::precalc_cvmask_derivatives(void)
{
	const UINT8 yarray[16] = {0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const UINT8 xarray[16] = {0, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	for (int i = 0; i < 0x10000; i++)
	{
		compressed_cvmasks[i] = (i & 1) | ((i & 4) >> 1) | ((i & 0x20) >> 3) | ((i & 0x80) >> 4) |
		((i & 0x100) >> 4) | ((i & 0x400) >> 5) | ((i & 0x2000) >> 7) | ((i & 0x8000) >> 8);
	}

	for (int i = 0; i < 0x100; i++)
	{
		UINT16 mask = decompress_cvmask_frombyte(i);
		cvarray[i].cvg = cvarray[i].cvbit = 0;
		cvarray[i].cvbit = (i >> 7) & 1;
		for (int k = 0; k < 8; k++)
		{
			cvarray[i].cvg += ((i >> k) & 1);
		}

		UINT16 masky = 0;
		for (int k = 0; k < 4; k++)
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
	UINT16 y = (x & 1) | ((x & 2) << 1) | ((x & 4) << 3) | ((x & 8) << 4) |
		((x & 0x10) << 4) | ((x & 0x20) << 5) | ((x & 0x40) << 7) | ((x & 0x80) << 8);
	return y;
}

void n64_rdp::lookup_cvmask_derivatives(UINT32 mask, UINT8* offx, UINT8* offy, rdp_span_aux *userdata)
{
	UINT32 index = compressed_cvmasks[mask];
	userdata->CurrentPixCvg = cvarray[index].cvg;
	userdata->CurrentCvgBit = cvarray[index].cvbit;
	*offx = cvarray[index].xoff;
	*offy = cvarray[index].yoff;
}

void n64_rdp::ZStore(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 z, UINT32 enc)
{
	UINT16 zval = z_com_table[z & 0x3ffff]|(enc >> 2);
	if(zcurpixel <= MEM16_LIMIT)
	{
		((UINT16*)rdram)[zcurpixel ^ WORD_ADDR_XOR] = zval;
	}
	if(dzcurpixel <= MEM8_LIMIT)
	{
		HiddenBits[dzcurpixel ^ BYTE_ADDR_XOR] = enc & 3;
	}
}

INT32 n64_rdp::NormalizeDZPix(INT32 sum)
{
	if (sum & 0xc000)
	{
		return 0x8000;
	}
	if (!(sum & 0xffff))
	{
		return 1;
	}
	for(int count = 0x2000; count > 0; count >>= 1)
    {
		if (sum & count)
		{
        	return(count << 1);
		}
    }
    return 0;
}

UINT32 n64_rdp::ZDecompress(UINT32 zcurpixel)
{
	UINT32 zb = RREADIDX16(zcurpixel);
	CHECK16(zcurpixel);
	return z_complete_dec_table[(zb >> 2) & 0x3fff];
}

UINT32 n64_rdp::DZDecompress(UINT32 zcurpixel, UINT32 dzcurpixel)
{
	UINT16 zval = RREADIDX16(zcurpixel);
	CHECK16(zcurpixel);
	UINT8 dzval = (((dzcurpixel) <= 0x7fffff) ? (HiddenBits[(dzcurpixel) ^ BYTE_ADDR_XOR]) : 0);
	UINT32 dz_compressed = ((zval & 3) << 2) | (dzval & 3);
	return (1 << dz_compressed);
}

UINT32 n64_rdp::DZCompress(UINT32 value)
{
	INT32 j = 0;
	for (; value > 1; j++, value >>= 1);
	return j;
}

void n64_rdp::GetDitherValues(int x, int y, int* cdith, int* adith, const rdp_poly_state& object)
{
	int dithindex = ((y & 3) << 2) | (x & 3);
	switch((object.OtherModes.rgb_dither_sel << 2) | object.OtherModes.alpha_dither_sel)
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

bool n64_rdp::ZCompare(UINT32 zcurpixel, UINT32 dzcurpixel, UINT32 sz, UINT16 dzpix, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	bool force_coplanar = false;
	sz &= 0x3ffff;

	UINT32 oz;
	UINT32 dzmem;
	UINT32 zval;
	INT32 rawdzmem;

	if (object.OtherModes.z_compare_en)
	{
		oz = ZDecompress(zcurpixel);
		dzmem = DZDecompress(zcurpixel, dzcurpixel);
		zval = RREADIDX16(zcurpixel);
		CHECK16(zcurpixel);
		rawdzmem = ((zval & 3) << 2) | ((((dzcurpixel) <= 0x3fffff) ? (HiddenBits[(dzcurpixel) ^ BYTE_ADDR_XOR]) : 0) & 3);
	}
	else
	{
		oz = 0;
		dzmem = 1 << 0xf;
		zval = 0x3;
		rawdzmem = 0xf;
	}

	userdata->m_dzpix_enc = DZCompress(dzpix & 0xffff);
	userdata->ShiftA = CLAMP(userdata->m_dzpix_enc - rawdzmem, 0, 4);
	userdata->ShiftB = CLAMP(rawdzmem - userdata->m_dzpix_enc, 0, 4);

	int precision_factor = (zval >> 13) & 0xf;
	if (precision_factor < 3)
	{
		int dzmemmodifier = 16 >> precision_factor;
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

	UINT32 dznew = (dzmem > dzpix) ? dzmem : (UINT32)dzpix;
	UINT32 dznotshift = dznew;
	dznew <<= 3;

	bool farther = (sz + dznew) >= oz;
	bool infront = sz < oz;

	if (force_coplanar)
	{
		farther = true;
	}

	bool overflow = ((userdata->CurrentMemCvg + userdata->CurrentPixCvg) & 8) > 0;
	userdata->BlendEnable = (object.OtherModes.force_blend || (!overflow && object.OtherModes.antialias_en && farther)) ? 1 : 0;
	userdata->PreWrap = overflow;

	int cvgcoeff = 0;
	UINT32 dzenc = 0;

	if (object.OtherModes.z_mode == 1 && infront && farther && overflow)
	{
		dzenc = DZCompress(dznotshift & 0xffff);
		cvgcoeff = ((oz >> dzenc) - (sz >> dzenc)) & 0xf;
		userdata->CurrentPixCvg = ((cvgcoeff * userdata->CurrentPixCvg) >> 3) & 0xf;
	}

	if (!object.OtherModes.z_compare_en)
	{
		return true;
	}

	INT32 diff = (INT32)sz - (INT32)dznew;
	bool nearer = diff <= (INT32)oz;
	bool max = (oz == 0x3ffff);
	if (force_coplanar)
	{
		nearer = true;
	}

	switch(object.OtherModes.z_mode)
	{
	case 0:
		return (max || (overflow ? infront : nearer));
		break;
	case 1:
		return (max || (overflow ? infront : nearer));
		break;
	case 2:
		return (infront || max);
		break;
	case 3:
		return (farther && nearer && !max);
		break;
	}

	return false;
}

UINT32 n64_rdp::GetLog2(UINT32 lod_clamp)
{
	if (lod_clamp < 2)
	{
		return 0;
	}
	else
	{
		for (int i = 7; i > 0; i--)
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

UINT32 n64_rdp::ReadData(UINT32 address)
{
	if (m_status & 0x1)		// XBUS_DMEM_DMA enabled
	{
		return rsp_dmem[(address & 0xfff) / 4];
	}
	else
	{
		return rdram[((address & 0xffffff) / 4)];
	}
}

static const char *const image_format[] = { "RGBA", "YUV", "CI", "IA", "I", "???", "???", "???" };
static const char *const image_size[] = { "4-bit", "8-bit", "16-bit", "32-bit" };

static const int rdp_command_length[64] =
{
	8,			// 0x00, No Op
	8,			// 0x01, ???
	8,			// 0x02, ???
	8,			// 0x03, ???
	8,			// 0x04, ???
	8,			// 0x05, ???
	8,			// 0x06, ???
	8,			// 0x07, ???
	32,			// 0x08, Non-Shaded Triangle
	32+16,		// 0x09, Non-Shaded, Z-Buffered Triangle
	32+64,		// 0x0a, Textured Triangle
	32+64+16,	// 0x0b, Textured, Z-Buffered Triangle
	32+64,		// 0x0c, Shaded Triangle
	32+64+16,	// 0x0d, Shaded, Z-Buffered Triangle
	32+64+64,	// 0x0e, Shaded+Textured Triangle
	32+64+64+16,// 0x0f, Shaded+Textured, Z-Buffered Triangle
	8,			// 0x10, ???
	8,			// 0x11, ???
	8,			// 0x12, ???
	8,			// 0x13, ???
	8,			// 0x14, ???
	8,			// 0x15, ???
	8,			// 0x16, ???
	8,			// 0x17, ???
	8,			// 0x18, ???
	8,			// 0x19, ???
	8,			// 0x1a, ???
	8,			// 0x1b, ???
	8,			// 0x1c, ???
	8,			// 0x1d, ???
	8,			// 0x1e, ???
	8,			// 0x1f, ???
	8,			// 0x20, ???
	8,			// 0x21, ???
	8,			// 0x22, ???
	8,			// 0x23, ???
	16,			// 0x24, Texture_Rectangle
	16,			// 0x25, Texture_Rectangle_Flip
	8,			// 0x26, Sync_Load
	8,			// 0x27, Sync_Pipe
	8,			// 0x28, Sync_Tile
	8,			// 0x29, Sync_Full
	8,			// 0x2a, Set_Key_GB
	8,			// 0x2b, Set_Key_R
	8,			// 0x2c, Set_Convert
	8,			// 0x2d, Set_Scissor
	8,			// 0x2e, Set_Prim_Depth
	8,			// 0x2f, Set_Other_Modes
	8,			// 0x30, Load_TLUT
	8,			// 0x31, ???
	8,			// 0x32, Set_Tile_Size
	8,			// 0x33, Load_Block
	8,			// 0x34, Load_Tile
	8,			// 0x35, Set_Tile
	8,			// 0x36, Fill_Rectangle
	8,			// 0x37, Set_Fill_Color
	8,			// 0x38, Set_Fog_Color
	8,			// 0x39, Set_Blend_Color
	8,			// 0x3a, Set_Prim_Color
	8,			// 0x3b, Set_Env_Color
	8,			// 0x3c, Set_Combine
	8,			// 0x3d, Set_Texture_Image
	8,			// 0x3e, Set_Mask_Image
	8			// 0x3f, Set_Color_Image
};

void n64_rdp::Dasm(char *buffer)
{
	int i;
	int tile;
	const char *format, *size;
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
	UINT32 r,g,b,a;

	UINT32 cmd[64];
	UINT32 length;
	UINT32 command;

	length = m_cmd_ptr * 4;
	if (length < 8)
	{
		sprintf(buffer, "ERROR: length = %d\n", length);
		return;
	}

	cmd[0] = m_cmd_data[m_cmd_cur+0];
	cmd[1] = m_cmd_data[m_cmd_cur+1];

	tile = (cmd[1] >> 24) & 0x7;
	sprintf(sl, "%4.2f", (float)((cmd[0] >> 12) & 0xfff) / 4.0f);
	sprintf(tl, "%4.2f", (float)((cmd[0] >>  0) & 0xfff) / 4.0f);
	sprintf(sh, "%4.2f", (float)((cmd[1] >> 12) & 0xfff) / 4.0f);
	sprintf(th, "%4.2f", (float)((cmd[1] >>  0) & 0xfff) / 4.0f);

	format = image_format[(cmd[0] >> 21) & 0x7];
	size = image_size[(cmd[0] >> 19) & 0x3];

	r = (cmd[1] >> 24) & 0xff;
	g = (cmd[1] >> 16) & 0xff;
	b = (cmd[1] >>  8) & 0xff;
	a = (cmd[1] >>  0) & 0xff;

	command = (cmd[0] >> 24) & 0x3f;
	switch (command)
	{
		case 0x00:	sprintf(buffer, "No Op"); break;
		case 0x08:		// Tri_NoShade
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
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

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)(cmd[7] / 65536.0f));

					sprintf(buffer, "Tri_NoShade            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x09:		// Tri_NoShadeZ
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
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

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)(cmd[7] / 65536.0f));

					sprintf(buffer, "Tri_NoShadeZ            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x0a:		// Tri_Tex
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Tex length = %d\n", length);
				return;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


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
		case 0x0b:		// Tri_TexZ
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexZ length = %d\n", length);
				return;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


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
		case 0x0c:		// Tri_Shade
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Shade length = %d\n", length);
				return;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

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
		case 0x0d:		// Tri_ShadeZ
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_ShadeZ length = %d\n", length);
				return;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = m_cmd_data[i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

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
		case 0x0e:		// Tri_TexShade
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShade length = %d\n", length);
				return;
			}

			for (i=2; i < 40; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


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
		case 0x0f:		// Tri_TexShadeZ
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShadeZ length = %d\n", length);
				return;
			}

			for (i=2; i < 40; i++)
			{
				cmd[i] = m_cmd_data[m_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((INT32)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((INT32)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((INT32)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((INT32)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((INT32)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((INT32)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(INT32)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(INT32)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(INT32)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(INT32)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(INT32)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(INT32)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(INT32)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(INT32)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(INT32)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(INT32)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(INT32)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(INT32)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(INT32)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(INT32)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(INT32)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(INT32)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,		"%4.4f", (float)(INT32)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(INT32)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(INT32)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(INT32)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(INT32)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(INT32)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(INT32)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(INT32)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(INT32)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(INT32)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(INT32)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(INT32)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


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
		case 0x26:	sprintf(buffer, "Sync_Load"); break;
		case 0x27:	sprintf(buffer, "Sync_Pipe"); break;
		case 0x28:	sprintf(buffer, "Sync_Tile"); break;
		case 0x29:	sprintf(buffer, "Sync_Full"); break;
		case 0x2d:	sprintf(buffer, "Set_Scissor            %s, %s, %s, %s", sl, tl, sh, th); break;
		case 0x2e:	sprintf(buffer, "Set_Prim_Depth         %04X, %04X", (cmd[1] >> 16) & 0xffff, cmd[1] & 0xffff); break;
		case 0x2f:	sprintf(buffer, "Set_Other_Modes        %08X %08X", cmd[0], cmd[1]); break;
		case 0x30:	sprintf(buffer, "Load_TLUT              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x32:	sprintf(buffer, "Set_Tile_Size          %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x33:	sprintf(buffer, "Load_Block             %d, %03X, %03X, %03X, %03X", tile, (cmd[0] >> 12) & 0xfff, cmd[0] & 0xfff, (cmd[1] >> 12) & 0xfff, cmd[1] & 0xfff); break;
		case 0x34:	sprintf(buffer, "Load_Tile              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x35:	sprintf(buffer, "Set_Tile               %d, %s, %s, %d, %04X", tile, format, size, ((cmd[0] >> 9) & 0x1ff) * 8, (cmd[0] & 0x1ff) * 8); break;
		case 0x36:	sprintf(buffer, "Fill_Rectangle         %s, %s, %s, %s", sh, th, sl, tl); break;
		case 0x37:	sprintf(buffer, "Set_Fill_Color         R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x38:	sprintf(buffer, "Set_Fog_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x39:	sprintf(buffer, "Set_Blend_Color        R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3a:	sprintf(buffer, "Set_Prim_Color         %d, %d, R: %d, G: %d, B: %d, A: %d", (cmd[0] >> 8) & 0x1f, cmd[0] & 0xff, r, g, b, a); break;
		case 0x3b:	sprintf(buffer, "Set_Env_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3c:	sprintf(buffer, "Set_Combine            %08X %08X", cmd[0], cmd[1]); break;
		case 0x3d:	sprintf(buffer, "Set_Texture_Image      %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		case 0x3e:	sprintf(buffer, "Set_Mask_Image         %08X", cmd[1]); break;
		case 0x3f:	sprintf(buffer, "Set_Color_Image        %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		default:	sprintf(buffer, "??? (%08X %08X)", cmd[0], cmd[1]); break;
	}
}

/*****************************************************************************/

static UINT32 rightcvghex(UINT32 x, UINT32 fmask)
{
	UINT32 stickybit = ((x >> 1) & 0x1fff) > 0;
	UINT32 covered = ((x >> 14) & 3) + stickybit;
	covered = (0xf0 >> covered) & 0xf;
	return (covered & fmask);
}

static UINT32 leftcvghex(UINT32 x, UINT32 fmask)
{
	UINT32 stickybit = ((x >> 1) & 0x1fff) > 0;
	UINT32 covered = ((x >> 14) & 3) + stickybit;
	covered = 0xf >> covered;
	return (covered & fmask);
}

static INT32 CLIP(INT32 value,INT32 min,INT32 max)
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

void n64_rdp::compute_cvg_noflip(extent_t *Spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base)
{
	INT32 purgestart = 0xfff;
	INT32 purgeend = 0;
	bool writablescanline = !(scanline & ~0x3ff);
	INT32 scanlinespx = scanline << 2;

	if (!writablescanline) return;

	for(int i = 0; i < 4; i++)
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
	INT32 length = purgeend - purgestart;

	if (length < 0) return;

	rdp_span_aux *userdata = (rdp_span_aux*)Spans[scanline - base].userdata;
	memset(&userdata->m_cvg[purgestart], 0, (length + 1) << 1);

	for(int i = 0; i < 4; i++)
	{
		INT32 minorcur = minorx[i];
		INT32 majorcur = majorx[i];
		INT32 minorcurint = minorxint[i];
		INT32 majorcurint = majorxint[i];
		length = majorcurint - minorcurint;

		INT32 fmask = (i & 1) ? 5 : 0xa;
		INT32 maskshift = (i ^ 3) << 2;
		INT32 fmaskshifted = fmask << maskshift;
		INT32 fleft = CLIP(minorcurint + 1, 0, 647);
		INT32 fright = CLIP(majorcurint - 1, 0, 647);
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
					INT32 samecvg = leftcvghex(minorcur, fmask) & rightcvghex(majorcur, fmask);
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

void n64_rdp::compute_cvg_flip(extent_t *Spans, INT32* majorx, INT32* minorx, INT32* majorxint, INT32* minorxint, INT32 scanline, INT32 yh, INT32 yl, INT32 base)
{
	INT32 purgestart = 0xfff;
	INT32 purgeend = 0;
	bool writablescanline = !(scanline & ~0x3ff);
	INT32 scanlinespx = scanline << 2;

	if(!writablescanline) return;

	for(int i = 0; i < 4; i++)
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

	int length = purgeend - purgestart;

	if (length < 0) return;

	rdp_span_aux *userdata = (rdp_span_aux*)Spans[scanline - base].userdata;
	memset(&userdata->m_cvg[purgestart], 0, (length + 1) << 1);

	for(int i = 0; i < 4; i++)
	{
		INT32 minorcur = minorx[i];
		INT32 majorcur = majorx[i];
		INT32 minorcurint = minorxint[i];
		INT32 majorcurint = majorxint[i];
		length = minorcurint - majorcurint;

		INT32 fmask = (i & 1) ? 5 : 0xa;
		INT32 maskshift = (i ^ 3) << 2;
		INT32 fmaskshifted = fmask << maskshift;
		INT32 fleft = CLIP(majorcurint + 1, 0, 647);
		INT32 fright = CLIP(minorcurint - 1, 0, 647);
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
					INT32 samecvg = rightcvghex(minorcur, fmask) & leftcvghex(majorcur, fmask);
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

void n64_rdp::DrawTriangle(bool shade, bool texture, bool zbuffer, bool rect)
{
	UINT32 *cmd_data = rect ? m_temp_rect_data : m_cmd_data;
	UINT32 fifo_index = rect ? 0 : m_cmd_cur;
	UINT32 w1 = cmd_data[fifo_index + 0];
	UINT32 w2 = cmd_data[fifo_index + 1];

	int flip = (w1 & 0x00800000) ? 1 : 0;
	MiscState.MaxLevel = ((w1 >> 19) & 7);
	int tilenum = (w1 >> 16) & 0x7;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	int dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0;
	int dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;
	int dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;

	INT32 maxxmx = 0; // maxxmx / minxhx very opaque names, consider re-naming
	INT32 minxmx = 0;
	INT32 maxxhx = 0;
	INT32 minxhx = 0;

	int shade_base = fifo_index + 8;
	int texture_base = fifo_index + 8;
	int zbuffer_base = fifo_index + 8;
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

	INT32 yl = (w1 & 0x3fff);
	INT32 ym = ((w2 >> 16) & 0x3fff);
	INT32 yh = ((w2 >>  0) & 0x3fff);
	INT32 xl = (INT32)(w3 & 0x3fffffff);
	INT32 xh = (INT32)(w5 & 0x3fffffff);
	INT32 xm = (INT32)(w7 & 0x3fffffff);
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

	int r    = (cmd_data[shade_base+0 ] & 0xffff0000) | ((cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	int g    = ((cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (cmd_data[shade_base+4 ] & 0x0000ffff);
	int b    = (cmd_data[shade_base+1 ] & 0xffff0000) | ((cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	int a    = ((cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (cmd_data[shade_base+5 ] & 0x0000ffff);
	int drdx = (cmd_data[shade_base+2 ] & 0xffff0000) | ((cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	int dgdx = ((cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (cmd_data[shade_base+6 ] & 0x0000ffff);
	int dbdx = (cmd_data[shade_base+3 ] & 0xffff0000) | ((cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	int dadx = ((cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (cmd_data[shade_base+7 ] & 0x0000ffff);
	int drde = (cmd_data[shade_base+8 ] & 0xffff0000) | ((cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	int dgde = ((cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (cmd_data[shade_base+12] & 0x0000ffff);
	int dbde = (cmd_data[shade_base+9 ] & 0xffff0000) | ((cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	int dade = ((cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (cmd_data[shade_base+13] & 0x0000ffff);
	int drdy = (cmd_data[shade_base+10] & 0xffff0000) | ((cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	int dgdy = ((cmd_data[shade_base+10] << 16) & 0xffff0000) | (cmd_data[shade_base+14] & 0x0000ffff);
	int dbdy = (cmd_data[shade_base+11] & 0xffff0000) | ((cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	int dady = ((cmd_data[shade_base+11] << 16) & 0xffff0000) | (cmd_data[shade_base+15] & 0x0000ffff);
	int s    = (cmd_data[texture_base+0 ] & 0xffff0000) | ((cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	int t    = ((cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (cmd_data[texture_base+4 ] & 0x0000ffff);
	int w    = (cmd_data[texture_base+1 ] & 0xffff0000) | ((cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	int dsdx = (cmd_data[texture_base+2 ] & 0xffff0000) | ((cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	int dtdx = ((cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (cmd_data[texture_base+6 ] & 0x0000ffff);
	int dwdx = (cmd_data[texture_base+3 ] & 0xffff0000) | ((cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	int dsde = (cmd_data[texture_base+8 ] & 0xffff0000) | ((cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	int dtde = ((cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (cmd_data[texture_base+12] & 0x0000ffff);
	int dwde = (cmd_data[texture_base+9 ] & 0xffff0000) | ((cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	int dsdy = (cmd_data[texture_base+10] & 0xffff0000) | ((cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	int dtdy = ((cmd_data[texture_base+10] << 16) & 0xffff0000)	| (cmd_data[texture_base+14] & 0x0000ffff);
	int dwdy = (cmd_data[texture_base+11] & 0xffff0000) | ((cmd_data[texture_base+15] >> 16) & 0x0000ffff);
	int z    = cmd_data[zbuffer_base+0];
	int dzdx = cmd_data[zbuffer_base+1];
	int dzde = cmd_data[zbuffer_base+2];
	int dzdy = cmd_data[zbuffer_base+3];

	int dzdy_dz = (dzdy >> 16) & 0xffff;
	int dzdx_dz = (dzdx >> 16) & 0xffff;

	extent_t Spans[1024];

	SpanBase.m_span_drdy = drdy;
	SpanBase.m_span_dgdy = dgdy;
	SpanBase.m_span_dbdy = dbdy;
	SpanBase.m_span_dady = dady;
	SpanBase.m_span_dzdy = OtherModes.z_source_sel ? 0 : dzdy;

	UINT32 temp_dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	SpanBase.m_span_dr = drdx & ~0x1f;
	SpanBase.m_span_dg = dgdx & ~0x1f;
	SpanBase.m_span_db = dbdx & ~0x1f;
	SpanBase.m_span_da = dadx & ~0x1f;
	SpanBase.m_span_ds = dsdx;
	SpanBase.m_span_dt = dtdx;
	SpanBase.m_span_dw = dwdx;
	SpanBase.m_span_dz = OtherModes.z_source_sel ? 0 : dzdx;
	SpanBase.m_span_dymax = 0;
	SpanBase.m_span_dzpix = NormalizeDZPix(temp_dzpix & 0xffff) & 0xffff;

	int xleft_inc = (dxmdy >> 2) & ~1;
	int xright_inc = (dxhdy >> 2) & ~1;

	int xright = xh & ~1;
	int xleft = xm & ~1;

	int sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;
	int do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

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

	INT32 ycur = yh & ~3;
	INT32 ylfar = yl | 3;
	INT32 ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	INT32 majorx[4];
	INT32 minorx[4];
	INT32 majorxint[4];
	INT32 minorxint[4];
	bool valid_y = true;

	int xfrac = ((xright >> 8) & 0xff);

	int clipy1 = Scissor.m_yh;
	int clipy2 = Scissor.m_yl;

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
	rdp_poly_state *object = NULL;

	if(flip)
	{
		for (int k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl & ~1;
				xleft_inc = (dxldy >> 2) & ~1;
			}

			int xstart = xleft >> 16; // 319
			int xend = xright >> 16; // 0
			int j = k >> 2;
			int spix = k & 3;
			valid_y = !(k < yh || k >= yl);

			if (k >= 0 && k < 0x1000)
			{
				majorxint[spix] = xend; // 0
				minorxint[spix] = xstart; // 319
				majorx[spix] = xright; // 0x00000000
				minorx[spix] = xleft; // 0x013f0000

				if (spix == 0)
				{
					maxxmx = 0;
					minxhx = 0xfff;
				}

				if (valid_y)
				{
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
				}

				if (spix == 0)
				{
					if(new_object)
					{
						object = &object_data_alloc();
						memcpy(object->m_tmem, m_tmem, 0x1000);
						new_object = false;
					}

					Spans[j - (ycur >> 2)].userdata = (void*)((UINT8*)AuxBuf + AuxBufPtr);
					AuxBufPtr += sizeof(rdp_span_aux);

					if(AuxBufPtr >= EXTENT_AUX_COUNT)
					{
						fatalerror("n64_rdp::DrawTriangle: span aux buffer overflow\n");
					}

					rdp_span_aux *userdata = (rdp_span_aux*)Spans[j - (ycur >> 2)].userdata;

					userdata->m_tmem = object->m_tmem;

					userdata->BlendColor = BlendColor;
					userdata->PrimColor = PrimColor;
					userdata->EnvColor = EnvColor;
					userdata->FogColor = FogColor;
					userdata->KeyScale = KeyScale;
					userdata->LODFraction = LODFraction;
					userdata->PrimLODFraction = PrimLODFraction;

					userdata->ColorInputs.combiner_rgbsub_a_r[0] = userdata->ColorInputs.combiner_rgbsub_a_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbsub_a_g[0] = userdata->ColorInputs.combiner_rgbsub_a_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbsub_a_b[0] = userdata->ColorInputs.combiner_rgbsub_a_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbsub_b_r[0] = userdata->ColorInputs.combiner_rgbsub_b_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbsub_b_g[0] = userdata->ColorInputs.combiner_rgbsub_b_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbsub_b_b[0] = userdata->ColorInputs.combiner_rgbsub_b_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbmul_r[0] = userdata->ColorInputs.combiner_rgbmul_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbmul_g[0] = userdata->ColorInputs.combiner_rgbmul_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbmul_b[0] = userdata->ColorInputs.combiner_rgbmul_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbadd_r[0] = userdata->ColorInputs.combiner_rgbadd_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbadd_g[0] = userdata->ColorInputs.combiner_rgbadd_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbadd_b[0] = userdata->ColorInputs.combiner_rgbadd_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_alphasub_a[0] = userdata->ColorInputs.combiner_alphasub_a[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphasub_b[0] = userdata->ColorInputs.combiner_alphasub_b[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphamul[0] = userdata->ColorInputs.combiner_alphamul[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphaadd[0] = userdata->ColorInputs.combiner_alphaadd[1] = &OneColor.i.a;

					userdata->ColorInputs.blender1a_r[0] = userdata->ColorInputs.blender1a_r[1] = &userdata->PixelColor.i.r;
					userdata->ColorInputs.blender1a_g[0] = userdata->ColorInputs.blender1a_g[1] = &userdata->PixelColor.i.g;
					userdata->ColorInputs.blender1a_b[0] = userdata->ColorInputs.blender1a_b[1] = &userdata->PixelColor.i.b;
					userdata->ColorInputs.blender1b_a[0] = userdata->ColorInputs.blender1b_a[1] = &userdata->PixelColor.i.a;
					userdata->ColorInputs.blender2a_r[0] = userdata->ColorInputs.blender2a_r[1] = &userdata->PixelColor.i.r;
					userdata->ColorInputs.blender2a_g[0] = userdata->ColorInputs.blender2a_g[1] = &userdata->PixelColor.i.g;
					userdata->ColorInputs.blender2a_b[0] = userdata->ColorInputs.blender2a_b[1] = &userdata->PixelColor.i.b;
					userdata->ColorInputs.blender2b_a[0] = userdata->ColorInputs.blender2b_a[1] = &userdata->PixelColor.i.a;

					// Setup blender data for this scanline
					SetBlenderInput(0, 0, &userdata->ColorInputs.blender1a_r[0],
										  &userdata->ColorInputs.blender1a_g[0],
										  &userdata->ColorInputs.blender1a_b[0],
										  &userdata->ColorInputs.blender1b_a[0], OtherModes.blend_m1a_0, OtherModes.blend_m1b_0, userdata);
					SetBlenderInput(0, 1, &userdata->ColorInputs.blender2a_r[0],
										  &userdata->ColorInputs.blender2a_g[0],
										  &userdata->ColorInputs.blender2a_b[0],
										  &userdata->ColorInputs.blender2b_a[0], OtherModes.blend_m2a_0, OtherModes.blend_m2b_0, userdata);
					SetBlenderInput(1, 0, &userdata->ColorInputs.blender1a_r[1],
										  &userdata->ColorInputs.blender1a_g[1],
										  &userdata->ColorInputs.blender1a_b[1],
										  &userdata->ColorInputs.blender1b_a[1], OtherModes.blend_m1a_1, OtherModes.blend_m1b_1, userdata);
					SetBlenderInput(1, 1, &userdata->ColorInputs.blender2a_r[1],
										  &userdata->ColorInputs.blender2a_g[1],
										  &userdata->ColorInputs.blender2a_b[1],
										  &userdata->ColorInputs.blender2b_a[1], OtherModes.blend_m2a_1, OtherModes.blend_m2b_1, userdata);

					// Setup color combiner data for this scanline
					SetSubAInputRGB(&userdata->ColorInputs.combiner_rgbsub_a_r[0], &userdata->ColorInputs.combiner_rgbsub_a_g[0], &userdata->ColorInputs.combiner_rgbsub_a_b[0], m_combine.sub_a_rgb0, userdata);
					SetSubBInputRGB(&userdata->ColorInputs.combiner_rgbsub_b_r[0], &userdata->ColorInputs.combiner_rgbsub_b_g[0], &userdata->ColorInputs.combiner_rgbsub_b_b[0], m_combine.sub_b_rgb0, userdata);
					SetMulInputRGB(&userdata->ColorInputs.combiner_rgbmul_r[0], &userdata->ColorInputs.combiner_rgbmul_g[0], &userdata->ColorInputs.combiner_rgbmul_b[0], m_combine.mul_rgb0, userdata);
					SetAddInputRGB(&userdata->ColorInputs.combiner_rgbadd_r[0], &userdata->ColorInputs.combiner_rgbadd_g[0], &userdata->ColorInputs.combiner_rgbadd_b[0], m_combine.add_rgb0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_a[0], m_combine.sub_a_a0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_b[0], m_combine.sub_b_a0, userdata);
					SetMulInputAlpha(&userdata->ColorInputs.combiner_alphamul[0], m_combine.mul_a0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphaadd[0], m_combine.add_a0, userdata);

					SetSubAInputRGB(&userdata->ColorInputs.combiner_rgbsub_a_r[1], &userdata->ColorInputs.combiner_rgbsub_a_g[1], &userdata->ColorInputs.combiner_rgbsub_a_b[1], m_combine.sub_a_rgb1, userdata);
					SetSubBInputRGB(&userdata->ColorInputs.combiner_rgbsub_b_r[1], &userdata->ColorInputs.combiner_rgbsub_b_g[1], &userdata->ColorInputs.combiner_rgbsub_b_b[1], m_combine.sub_b_rgb1, userdata);
					SetMulInputRGB(&userdata->ColorInputs.combiner_rgbmul_r[1], &userdata->ColorInputs.combiner_rgbmul_g[1], &userdata->ColorInputs.combiner_rgbmul_b[1], m_combine.mul_rgb1, userdata);
					SetAddInputRGB(&userdata->ColorInputs.combiner_rgbadd_r[1], &userdata->ColorInputs.combiner_rgbadd_g[1], &userdata->ColorInputs.combiner_rgbadd_b[1], m_combine.add_rgb1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_a[1], m_combine.sub_a_a1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_b[1], m_combine.sub_b_a1, userdata);
					SetMulInputAlpha(&userdata->ColorInputs.combiner_alphamul[1], m_combine.mul_a1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphaadd[1], m_combine.add_a1, userdata);
				}

				if (spix == 3)
				{
					Spans[j - (ycur >> 2)].startx = maxxmx;
					Spans[j - (ycur >> 2)].stopx = minxhx;
					compute_cvg_flip(Spans, majorx, minorx, majorxint, minorxint, j, yh, yl, ycur >> 2);
				}

				if (spix == ldflag)
				{
					((rdp_span_aux*)Spans[j - (ycur >> 2)].userdata)->m_unscissored_rx = xend;
					xfrac = ((xright >> 8) & 0xff);
					Spans[j - (ycur >> 2)].param[SPAN_R].start = ((r >> 9) << 9) + drdiff - (xfrac * drdxh);
					Spans[j - (ycur >> 2)].param[SPAN_G].start = ((g >> 9) << 9) + dgdiff - (xfrac * dgdxh);
					Spans[j - (ycur >> 2)].param[SPAN_B].start = ((b >> 9) << 9) + dbdiff - (xfrac * dbdxh);
					Spans[j - (ycur >> 2)].param[SPAN_A].start = ((a >> 9) << 9) + dadiff - (xfrac * dadxh);
					Spans[j - (ycur >> 2)].param[SPAN_S].start = (((s >> 9) << 9)  + dsdiff - (xfrac * dsdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_T].start = (((t >> 9) << 9)  + dtdiff - (xfrac * dtdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_W].start = (((w >> 9) << 9)  + dwdiff - (xfrac * dwdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_Z].start = ((z >> 9) << 9)  + dzdiff - (xfrac * dzdxh);
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
		for (int k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl & ~1;
				xleft_inc = (dxldy >> 2) & ~1;
			}

			int xstart = xleft >> 16;
			int xend = xright >> 16;
			int j = k >> 2;
			int spix = k & 3;
			valid_y = !(k < yh || k >= yl);

			if (k >= 0 && k < 0x1000)
			{
				majorxint[spix] = xend;
				minorxint[spix] = xstart;
				majorx[spix] = xright;
				minorx[spix] = xleft;

				if (spix == 0)
				{
					maxxhx = 0;
					minxmx = 0xfff;
				}

				if (valid_y)
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}

				if (spix == 0)
				{
					if(new_object)
					{
						object = &object_data_alloc();
						memcpy(object->m_tmem, m_tmem, 0x1000);
						new_object = false;
					}

					Spans[j - (ycur >> 2)].userdata = (void*)((UINT8*)AuxBuf + AuxBufPtr);
					AuxBufPtr += sizeof(rdp_span_aux);

					if(AuxBufPtr >= EXTENT_AUX_COUNT)
					{
						fatalerror("n64_rdp::DrawTriangle: span aux buffer overflow\n");
					}

					rdp_span_aux *userdata = (rdp_span_aux*)Spans[j - (ycur >> 2)].userdata;
					userdata->m_tmem = object->m_tmem;

					userdata->BlendColor = BlendColor;
					userdata->PrimColor = PrimColor;
					userdata->EnvColor = EnvColor;
					userdata->FogColor = FogColor;
					userdata->KeyScale = KeyScale;
					userdata->LODFraction = LODFraction;
					userdata->PrimLODFraction = PrimLODFraction;

					userdata->ColorInputs.combiner_rgbsub_a_r[0] = userdata->ColorInputs.combiner_rgbsub_a_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbsub_a_g[0] = userdata->ColorInputs.combiner_rgbsub_a_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbsub_a_b[0] = userdata->ColorInputs.combiner_rgbsub_a_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbsub_b_r[0] = userdata->ColorInputs.combiner_rgbsub_b_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbsub_b_g[0] = userdata->ColorInputs.combiner_rgbsub_b_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbsub_b_b[0] = userdata->ColorInputs.combiner_rgbsub_b_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbmul_r[0] = userdata->ColorInputs.combiner_rgbmul_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbmul_g[0] = userdata->ColorInputs.combiner_rgbmul_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbmul_b[0] = userdata->ColorInputs.combiner_rgbmul_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_rgbadd_r[0] = userdata->ColorInputs.combiner_rgbadd_r[1] = &OneColor.i.r;
					userdata->ColorInputs.combiner_rgbadd_g[0] = userdata->ColorInputs.combiner_rgbadd_g[1] = &OneColor.i.g;
					userdata->ColorInputs.combiner_rgbadd_b[0] = userdata->ColorInputs.combiner_rgbadd_b[1] = &OneColor.i.b;
					userdata->ColorInputs.combiner_alphasub_a[0] = userdata->ColorInputs.combiner_alphasub_a[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphasub_b[0] = userdata->ColorInputs.combiner_alphasub_b[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphamul[0] = userdata->ColorInputs.combiner_alphamul[1] = &OneColor.i.a;
					userdata->ColorInputs.combiner_alphaadd[0] = userdata->ColorInputs.combiner_alphaadd[1] = &OneColor.i.a;

					userdata->ColorInputs.blender1a_r[0] = userdata->ColorInputs.blender1a_r[1] = &userdata->PixelColor.i.r;
					userdata->ColorInputs.blender1a_g[0] = userdata->ColorInputs.blender1a_g[1] = &userdata->PixelColor.i.g;
					userdata->ColorInputs.blender1a_b[0] = userdata->ColorInputs.blender1a_b[1] = &userdata->PixelColor.i.b;
					userdata->ColorInputs.blender1b_a[0] = userdata->ColorInputs.blender1b_a[1] = &userdata->PixelColor.i.a;
					userdata->ColorInputs.blender2a_r[0] = userdata->ColorInputs.blender2a_r[1] = &userdata->PixelColor.i.r;
					userdata->ColorInputs.blender2a_g[0] = userdata->ColorInputs.blender2a_g[1] = &userdata->PixelColor.i.g;
					userdata->ColorInputs.blender2a_b[0] = userdata->ColorInputs.blender2a_b[1] = &userdata->PixelColor.i.b;
					userdata->ColorInputs.blender2b_a[0] = userdata->ColorInputs.blender2b_a[1] = &userdata->PixelColor.i.a;

					// Setup blender data for this scanline
					SetBlenderInput(0, 0, &userdata->ColorInputs.blender1a_r[0],
										  &userdata->ColorInputs.blender1a_g[0],
										  &userdata->ColorInputs.blender1a_b[0],
										  &userdata->ColorInputs.blender1b_a[0], OtherModes.blend_m1a_0, OtherModes.blend_m1b_0, userdata);
					SetBlenderInput(0, 1, &userdata->ColorInputs.blender2a_r[0],
										  &userdata->ColorInputs.blender2a_g[0],
										  &userdata->ColorInputs.blender2a_b[0],
										  &userdata->ColorInputs.blender2b_a[0], OtherModes.blend_m2a_0, OtherModes.blend_m2b_0, userdata);
					SetBlenderInput(1, 0, &userdata->ColorInputs.blender1a_r[1],
										  &userdata->ColorInputs.blender1a_g[1],
										  &userdata->ColorInputs.blender1a_b[1],
										  &userdata->ColorInputs.blender1b_a[1], OtherModes.blend_m1a_1, OtherModes.blend_m1b_1, userdata);
					SetBlenderInput(1, 1, &userdata->ColorInputs.blender2a_r[1],
										  &userdata->ColorInputs.blender2a_g[1],
										  &userdata->ColorInputs.blender2a_b[1],
										  &userdata->ColorInputs.blender2b_a[1], OtherModes.blend_m2a_1, OtherModes.blend_m2b_1, userdata);

					// Setup color combiner data for this scanline
					SetSubAInputRGB(&userdata->ColorInputs.combiner_rgbsub_a_r[0], &userdata->ColorInputs.combiner_rgbsub_a_g[0], &userdata->ColorInputs.combiner_rgbsub_a_b[0], m_combine.sub_a_rgb0, userdata);
					SetSubBInputRGB(&userdata->ColorInputs.combiner_rgbsub_b_r[0], &userdata->ColorInputs.combiner_rgbsub_b_g[0], &userdata->ColorInputs.combiner_rgbsub_b_b[0], m_combine.sub_b_rgb0, userdata);
					SetMulInputRGB(&userdata->ColorInputs.combiner_rgbmul_r[0], &userdata->ColorInputs.combiner_rgbmul_g[0], &userdata->ColorInputs.combiner_rgbmul_b[0], m_combine.mul_rgb0, userdata);
					SetAddInputRGB(&userdata->ColorInputs.combiner_rgbadd_r[0], &userdata->ColorInputs.combiner_rgbadd_g[0], &userdata->ColorInputs.combiner_rgbadd_b[0], m_combine.add_rgb0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_a[0], m_combine.sub_a_a0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_b[0], m_combine.sub_b_a0, userdata);
					SetMulInputAlpha(&userdata->ColorInputs.combiner_alphamul[0], m_combine.mul_a0, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphaadd[0], m_combine.add_a0, userdata);

					SetSubAInputRGB(&userdata->ColorInputs.combiner_rgbsub_a_r[1], &userdata->ColorInputs.combiner_rgbsub_a_g[1], &userdata->ColorInputs.combiner_rgbsub_a_b[1], m_combine.sub_a_rgb1, userdata);
					SetSubBInputRGB(&userdata->ColorInputs.combiner_rgbsub_b_r[1], &userdata->ColorInputs.combiner_rgbsub_b_g[1], &userdata->ColorInputs.combiner_rgbsub_b_b[1], m_combine.sub_b_rgb1, userdata);
					SetMulInputRGB(&userdata->ColorInputs.combiner_rgbmul_r[1], &userdata->ColorInputs.combiner_rgbmul_g[1], &userdata->ColorInputs.combiner_rgbmul_b[1], m_combine.mul_rgb1, userdata);
					SetAddInputRGB(&userdata->ColorInputs.combiner_rgbadd_r[1], &userdata->ColorInputs.combiner_rgbadd_g[1], &userdata->ColorInputs.combiner_rgbadd_b[1], m_combine.add_rgb1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_a[1], m_combine.sub_a_a1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphasub_b[1], m_combine.sub_b_a1, userdata);
					SetMulInputAlpha(&userdata->ColorInputs.combiner_alphamul[1], m_combine.mul_a1, userdata);
					SetSubInputAlpha(&userdata->ColorInputs.combiner_alphaadd[1], m_combine.add_a1, userdata);
				}

				if (spix == 3)
				{
					Spans[j - (ycur >> 2)].startx = minxmx;
					Spans[j - (ycur >> 2)].stopx = maxxhx;
					compute_cvg_noflip(Spans, majorx, minorx, majorxint, minorxint, j, yh, yl, ycur >> 2);
				}

				if (spix == ldflag)
				{
					((rdp_span_aux*)Spans[j - (ycur >> 2)].userdata)->m_unscissored_rx = xend;
					xfrac = ((xright >> 8) & 0xff);
					Spans[j - (ycur >> 2)].param[SPAN_R].start = ((r >> 9) << 9) + drdiff - (xfrac * drdxh);
					Spans[j - (ycur >> 2)].param[SPAN_G].start = ((g >> 9) << 9) + dgdiff - (xfrac * dgdxh);
					Spans[j - (ycur >> 2)].param[SPAN_B].start = ((b >> 9) << 9) + dbdiff - (xfrac * dbdxh);
					Spans[j - (ycur >> 2)].param[SPAN_A].start = ((a >> 9) << 9) + dadiff - (xfrac * dadxh);
					Spans[j - (ycur >> 2)].param[SPAN_S].start = (((s >> 9) << 9)  + dsdiff - (xfrac * dsdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_T].start = (((t >> 9) << 9)  + dtdiff - (xfrac * dtdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_W].start = (((w >> 9) << 9)  + dwdiff - (xfrac * dwdxh)) & ~0x1f;
					Spans[j - (ycur >> 2)].param[SPAN_Z].start = ((z >> 9) << 9)  + dzdiff - (xfrac * dzdxh);
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

	if(!new_object)
	{
		RenderSpans(yh >> 2, yl >> 2, tilenum, flip ? true : false, Spans, rect, object);
	}

	//wait("DrawTriangle");
}

/*****************************************************************************/

////////////////////////
// FB ACCESSORS
////////////////////////
void n64_rdp::_Write16Bit_Cvg0_Blend(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
		CHECK16(fb);
	}

	finalcvg = userdata->CurrentPixCvg + userdata->CurrentMemCvg;
	if (finalcvg & 8)
	{
		finalcvg = 7;
	}
	RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
	HWRITEADDR8(hb, finalcvg & 3);
}

void n64_rdp::_Write16Bit_Cvg0_NoBlend(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
		CHECK16(fb);
	}

	finalcvg = (userdata->CurrentPixCvg - 1) & 7;
	RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
	HWRITEADDR8(hb, finalcvg & 3);
}

void n64_rdp::_Write16Bit_Cvg1(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
		CHECK16(fb);
	}

	finalcvg = (userdata->CurrentPixCvg + userdata->CurrentMemCvg) & 7;
	RWRITEIDX16(fb, finalcolor | ((finalcvg >> 2) & 1));
	HWRITEADDR8(hb, finalcvg & 3);
}

void n64_rdp::_Write16Bit_Cvg2(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
		CHECK16(fb);
	}

	RWRITEIDX16(fb, finalcolor | 1);
	HWRITEADDR8(hb, 3);
}

void n64_rdp::_Write16Bit_Cvg3(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 1) + curpixel;
	UINT32 hb = fb;

	UINT16 finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX16(fb) & 0xfffe;
		CHECK16(fb);
	}

	RWRITEIDX16(fb, finalcolor | ((userdata->CurrentMemCvg >> 2) & 1));
	HWRITEADDR8(hb, userdata->CurrentMemCvg & 3);
}

void n64_rdp::_Write32Bit_Cvg0_Blend(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
		CHECK32(fb);
	}

	finalcvg = userdata->CurrentPixCvg + userdata->CurrentMemCvg;
	if (finalcvg & 8)
	{
		finalcvg = 7;
	}
	RWRITEIDX32(fb, finalcolor | (finalcvg << 5));
}

void n64_rdp::_Write32Bit_Cvg0_NoBlend(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
		CHECK32(fb);
	}

	finalcvg = (userdata->CurrentPixCvg - 1) & 7;
	RWRITEIDX32(fb, finalcolor | (finalcvg << 5));
}

void n64_rdp::_Write32Bit_Cvg1(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;
	UINT32 finalcvg = 0;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
		CHECK32(fb);
	}

	finalcvg = (userdata->CurrentPixCvg + userdata->CurrentMemCvg) & 7;
	finalcolor |= (finalcvg << 5);
	RWRITEIDX32(fb, finalcolor);
}

void n64_rdp::_Write32Bit_Cvg2(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
		CHECK32(fb);
	}

	RWRITEIDX32(fb, finalcolor | 0xE0);
}

void n64_rdp::_Write32Bit_Cvg3(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 fb = (object.MiscState.FBAddress >> 2) + curpixel;
	UINT32 finalcolor = (r << 24) | (g << 16) | (b << 8);//cvg as 3 MSBs of alpha channel;

	if (object.OtherModes.color_on_cvg && !userdata->PreWrap)
	{
		finalcolor = RREADIDX32(fb) & 0xffffff00;
		CHECK32(fb);
	}

	RWRITEIDX32(fb, finalcolor | (userdata->CurrentMemCvg << 5));
}


void n64_rdp::_Read16Bit_ImgRead0(UINT32 curpixel, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT16 fword = RREADIDX16((object.MiscState.FBAddress >> 1) + curpixel);
	CHECK16((object.MiscState.FBAddress >> 1) + curpixel);
	userdata->MemoryColor.i.r = GETHICOL(fword);
	userdata->MemoryColor.i.g = GETMEDCOL(fword);
	userdata->MemoryColor.i.b = GETLOWCOL(fword);
	userdata->MemoryColor.i.a = 0xff;
	userdata->CurrentMemCvg = 7;
}

void n64_rdp::_Read16Bit_ImgRead1(UINT32 curpixel, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT16 fword = RREADIDX16((object.MiscState.FBAddress >> 1) + curpixel);
	CHECK16((object.MiscState.FBAddress >> 1) + curpixel);
	UINT8 hbyte = HREADADDR8((object.MiscState.FBAddress >> 1) + curpixel);
	userdata->MemoryColor.i.r = GETHICOL(fword);
	userdata->MemoryColor.i.g = GETMEDCOL(fword);
	userdata->MemoryColor.i.b = GETLOWCOL(fword);
	userdata->MemoryColor.i.a = userdata->CurrentMemCvg << 5;
	userdata->CurrentMemCvg = ((fword & 1) << 2) | (hbyte & 3);
}

void n64_rdp::_Read32Bit_ImgRead0(UINT32 curpixel, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 mem = RREADIDX32((object.MiscState.FBAddress >> 2) + curpixel);
	CHECK32((object.MiscState.FBAddress >> 2) + curpixel);
	userdata->MemoryColor.i.r = (mem >> 24) & 0xff;
	userdata->MemoryColor.i.g = (mem >> 16) & 0xff;
	userdata->MemoryColor.i.b = (mem >> 8) & 0xff;
	userdata->MemoryColor.i.a = 0xff;
	userdata->CurrentMemCvg = 7;
}

void n64_rdp::_Read32Bit_ImgRead1(UINT32 curpixel, rdp_span_aux *userdata, const rdp_poly_state &object)
{
	UINT32 mem = RREADIDX32((object.MiscState.FBAddress >> 2) + curpixel);
	CHECK32((object.MiscState.FBAddress >> 2) + curpixel);
	userdata->MemoryColor.i.r = (mem >> 24) & 0xff;
	userdata->MemoryColor.i.g = (mem >> 16) & 0xff;
	userdata->MemoryColor.i.b = (mem >> 8) & 0xff;
	userdata->MemoryColor.i.a = (mem) & 0xff;
	userdata->CurrentMemCvg = (mem >> 5) & 7;
}

void n64_rdp::_Copy16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, int CurrentPixCvg, const rdp_poly_state &object)
{
	UINT16 val = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((CurrentPixCvg >> 2) & 1);
	RWRITEIDX16((object.MiscState.FBAddress >> 1) + curpixel, val);
	HWRITEADDR8((object.MiscState.FBAddress >> 1) + curpixel, CurrentPixCvg & 3);
	CHECK16((object.MiscState.FBAddress >> 1) + curpixel);
}

void n64_rdp::_Copy32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, int CurrentPixCvg, const rdp_poly_state &object)
{
	UINT32 val = (r << 24) | (g << 16) | (b << 8) | (CurrentPixCvg << 5);
	RWRITEIDX32((object.MiscState.FBAddress >> 2) + curpixel, val);
	CHECK32((object.MiscState.FBAddress >> 2) + curpixel);
}


void n64_rdp::_Fill16Bit(UINT32 curpixel, const rdp_poly_state &object)
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
	CHECK16((object.MiscState.FBAddress >> 1) + curpixel);
	HWRITEADDR8((object.MiscState.FBAddress >> 1) + curpixel, ((val & 1) << 1) | (val & 1));
}

void n64_rdp::_Fill32Bit(UINT32 curpixel, const rdp_poly_state &object)
{
	UINT32 FillColor = object.FillColor;
	RWRITEIDX32((object.MiscState.FBAddress >> 2) + curpixel, FillColor);
	CHECK32((object.MiscState.FBAddress >> 2) + curpixel);
	HWRITEADDR8((object.MiscState.FBAddress >> 1) + (curpixel << 1), (FillColor & 0x10000) ? 3 : 0);
	HWRITEADDR8((object.MiscState.FBAddress >> 1) + (curpixel << 1) + 1, (FillColor & 0x1) ? 3 : 0);
}

////////////////////////
// RDP COMMANDS
////////////////////////
void n64_rdp::Triangle(bool shade, bool texture, bool zbuffer)
{
	DrawTriangle(shade, texture, zbuffer, false);
	m_pipe_clean = false;
	//wait();
}

void n64_rdp::CmdTriangle(UINT32 w1, UINT32 w2)
{
	Triangle(false, false, false);
}

void n64_rdp::CmdTriangleZ(UINT32 w1, UINT32 w2)
{
	Triangle(false, false, true);
}

void n64_rdp::CmdTriangleT(UINT32 w1, UINT32 w2)
{
	Triangle(false, true, false);
}

void n64_rdp::CmdTriangleTZ(UINT32 w1, UINT32 w2)
{
	Triangle(false, true, true);
}

void n64_rdp::CmdTriangleS(UINT32 w1, UINT32 w2)
{
	Triangle(true, false, false);
}

void n64_rdp::CmdTriangleSZ(UINT32 w1, UINT32 w2)
{
	Triangle(true, false, true);
}

void n64_rdp::CmdTriangleST(UINT32 w1, UINT32 w2)
{
	Triangle(true, true, false);
}

void n64_rdp::CmdTriangleSTZ(UINT32 w1, UINT32 w2)
{
	Triangle(true, true, true);
}

void n64_rdp::CmdTexRect(UINT32 w1, UINT32 w2)
{
	//if(m_pending_mode_block) { wait("Block on pending mode-change"); m_pending_mode_block = false; }

	UINT32 *data = m_cmd_data + m_cmd_cur;

	UINT32 w3 = data[2];
	UINT32 w4 = data[3];

	UINT32 tilenum	= (w2 >> 24) & 0x7;
	UINT32 xl = (w1 >> 12) & 0xfff;
	UINT32 yl	= (w1 >>  0) & 0xfff;
	UINT32 xh	= (w2 >> 12) & 0xfff;
	UINT32 yh	= (w2 >>  0) & 0xfff;
	INT32 s = (w3 >> 16) & 0xffff;
	INT32 t = (w3 >>  0) & 0xffff;
	INT32 dsdx = (w4 >> 16) & 0xffff;
	INT32 dtdy = (w4 >>  0) & 0xffff;

	dsdx = SIGN16(dsdx);
	dtdy = SIGN16(dtdy);

	if (OtherModes.cycle_type == CYCLE_TYPE_FILL || OtherModes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	UINT32 xlint = (xl >> 2) & 0x3ff;
	UINT32 xhint = (xh >> 2) & 0x3ff;

	UINT32* ewdata = m_temp_rect_data;
	ewdata[0] = (0x24 << 24) | ((0x80 | tilenum) << 16) | yl;	// command, flipped, tile, yl
	ewdata[1] = (yl << 16) | yh;								// ym, yh
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);				// xl, xl frac
	ewdata[3] = 0;												// dxldy, dxldy frac
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);				// xh, xh frac
	ewdata[5] = 0;												// dxhdy, dxhdy frac
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);				// xm, xm frac
	ewdata[7] = 0;												// dxmdy, dxmdy frac
	memset(&ewdata[8], 0, 16 * sizeof(UINT32));					// shade
	ewdata[24] = (s << 16) | t;									// s, t
	ewdata[25] = 0;												// w
	ewdata[26] = ((dsdx >> 5) << 16);							// dsdx, dtdx
	ewdata[27] = 0;												// dwdx
	ewdata[28] = 0;												// s frac, t frac
	ewdata[29] = 0;												// w frac
	ewdata[30] = ((dsdx & 0x1f) << 11) << 16;					// dsdx frac, dtdx frac
	ewdata[31] = 0;												// dwdx frac
	ewdata[32] = (dtdy >> 5) & 0xffff;//dsde, dtde
	ewdata[33] = 0;//dwde
	ewdata[34] = (dtdy >> 5) & 0xffff;//dsdy, dtdy
	ewdata[35] = 0;//dwdy
	ewdata[36] = (dtdy & 0x1f) << 11;//dsde frac, dtde frac
	ewdata[37] = 0;//dwde frac
	ewdata[38] = (dtdy & 0x1f) << 11;//dsdy frac, dtdy frac
	ewdata[39] = 0;//dwdy frac
	memset(&ewdata[40], 0, 4 * sizeof(UINT32));//depth

	DrawTriangle(true, true, false, true);
}

void n64_rdp::CmdTexRectFlip(UINT32 w1, UINT32 w2)
{
	//if(m_pending_mode_block) { wait("Block on pending mode-change"); m_pending_mode_block = false; }

	UINT32 *data = m_cmd_data + m_cmd_cur;

	UINT32 w3 = data[2];
	UINT32 w4 = data[3];

	UINT32 tilenum	= (w2 >> 24) & 0x7;
	UINT32 xl = (w1 >> 12) & 0xfff;
	UINT32 yl	= (w1 >>  0) & 0xfff;
	UINT32 xh	= (w2 >> 12) & 0xfff;
	UINT32 yh	= (w2 >>  0) & 0xfff;
	INT32 s = (w3 >> 16) & 0xffff;
	INT32 t = (w3 >>  0) & 0xffff;
	INT32 dsdx = (w4 >> 16) & 0xffff;
	INT32 dtdy = (w4 >>  0) & 0xffff;

	dsdx = SIGN16(dsdx);
	dtdy = SIGN16(dtdy);

	if (OtherModes.cycle_type == CYCLE_TYPE_FILL || OtherModes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	UINT32 xlint = (xl >> 2) & 0x3ff;
	UINT32 xhint = (xh >> 2) & 0x3ff;

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

	DrawTriangle(true, true, false, true);
}

void n64_rdp::CmdSyncLoad(UINT32 w1, UINT32 w2)
{
	//wait("SyncLoad");
}

void n64_rdp::CmdSyncPipe(UINT32 w1, UINT32 w2)
{
	//wait("SyncPipe");
}

void n64_rdp::CmdSyncTile(UINT32 w1, UINT32 w2)
{
	//wait("SyncTile");
}

void n64_rdp::CmdSyncFull(UINT32 w1, UINT32 w2)
{
	//wait("SyncFull");
	dp_full_sync(*m_machine);
}

void n64_rdp::CmdSetKeyGB(UINT32 w1, UINT32 w2)
{
	KeyScale.i.b = w2 & 0xff;
	KeyScale.i.g = (w2 >> 16) & 0xff;
}

void n64_rdp::CmdSetKeyR(UINT32 w1, UINT32 w2)
{
	KeyScale.i.r = w2 & 0xff;
}

void n64_rdp::CmdSetFillColor32(UINT32 w1, UINT32 w2)
{
	//wait("SetFillColor");
	FillColor = w2;
}

void n64_rdp::CmdSetConvert(UINT32 w1, UINT32 w2)
{
	if(!m_pipe_clean) { m_pipe_clean = true; wait("SetConvert"); }
	INT32 k0 = (w1 >> 13) & 0xff;
	INT32 k1 = (w1 >> 4) & 0xff;
	INT32 k2 = ((w1 & 7) << 5) | ((w2 >> 27) & 0x1f);
	INT32 k3 = (w2 >> 18) & 0xff;
	INT32 k4 = (w2 >> 9) & 0xff;
	INT32 k5 = w2 & 0xff;
	k0 = ((w1 >> 21) & 1) ? (-(0x100 - k0)) : k0;
	k1 = ((w1 >> 12) & 1) ?	(-(0x100 - k1)) : k1;
	k2 = (w1 & 0xf) ? (-(0x100 - k2)) : k2;
	k3 = ((w2 >> 26) & 1) ? (-(0x100 - k3)) : k3;
	k4 = ((w2 >> 17) & 1) ? (-(0x100 - k4)) : k4;
	k5 = ((w2 >> 8) & 1) ? (-(0x100 - k5)) : k5;
	SetYUVFactors(k0, k1, k2, k3, k4, k5);
}

void n64_rdp::CmdSetScissor(UINT32 w1, UINT32 w2)
{
	Scissor.m_xh = ((w1 >> 12) & 0xfff) >> 2;
	Scissor.m_yh = ((w1 >>  0) & 0xfff) >> 2;
	Scissor.m_xl = ((w2 >> 12) & 0xfff) >> 2;
	Scissor.m_yl = ((w2 >>  0) & 0xfff) >> 2;

	// TODO: handle f & o?
}

void n64_rdp::CmdSetPrimDepth(UINT32 w1, UINT32 w2)
{
	MiscState.PrimitiveZ = (UINT16)(w2 >> 16) & 0x7fff;
	MiscState.PrimitiveDZ = (UINT16)(w1);
}

void n64_rdp::CmdSetOtherModes(UINT32 w1, UINT32 w2)
{
	//wait("SetOtherModes");
	OtherModes.cycle_type		= (w1 >> 20) & 0x3; // 01
	OtherModes.persp_tex_en		= (w1 & 0x80000) ? 1 : 0; // 1
	OtherModes.detail_tex_en		= (w1 & 0x40000) ? 1 : 0; // 0
	OtherModes.sharpen_tex_en	= (w1 & 0x20000) ? 1 : 0; // 0
	OtherModes.tex_lod_en		= (w1 & 0x10000) ? 1 : 0; // 0
	OtherModes.en_tlut			= (w1 & 0x08000) ? 1 : 0; // 0
	OtherModes.tlut_type			= (w1 & 0x04000) ? 1 : 0; // 0
	OtherModes.sample_type		= (w1 & 0x02000) ? 1 : 0; // 1
	OtherModes.mid_texel			= (w1 & 0x01000) ? 1 : 0; // 0
	OtherModes.bi_lerp0			= (w1 & 0x00800) ? 1 : 0; // 1
	OtherModes.bi_lerp1			= (w1 & 0x00400) ? 1 : 0; // 1
	OtherModes.convert_one		= (w1 & 0x00200) ? 1 : 0; // 0
	OtherModes.key_en			= (w1 & 0x00100) ? 1 : 0; // 0
	OtherModes.rgb_dither_sel	= (w1 >> 6) & 0x3; // 00
	OtherModes.alpha_dither_sel	= (w1 >> 4) & 0x3; // 01
	OtherModes.blend_m1a_0		= (w2 >> 30) & 0x3; // 11
	OtherModes.blend_m1a_1		= (w2 >> 28) & 0x3; // 00
	OtherModes.blend_m1b_0		= (w2 >> 26) & 0x3; // 10
	OtherModes.blend_m1b_1		= (w2 >> 24) & 0x3; // 00
	OtherModes.blend_m2a_0		= (w2 >> 22) & 0x3; // 00
	OtherModes.blend_m2a_1		= (w2 >> 20) & 0x3; // 01
	OtherModes.blend_m2b_0		= (w2 >> 18) & 0x3; // 00
	OtherModes.blend_m2b_1		= (w2 >> 16) & 0x3; // 01
	OtherModes.force_blend		= (w2 >> 14) & 1; // 0
	OtherModes.alpha_cvg_select	= (w2 >> 13) & 1; // 1
	OtherModes.cvg_times_alpha	= (w2 >> 12) & 1; // 0
	OtherModes.z_mode			= (w2 >> 10) & 0x3; // 00
	OtherModes.cvg_dest			= (w2 >> 8) & 0x3; // 00
	OtherModes.color_on_cvg		= (w2 >> 7) & 1; // 0
	OtherModes.image_read_en	= (w2 >> 6) & 1; // 1
	OtherModes.z_update_en		= (w2 >> 5) & 1; // 1
	OtherModes.z_compare_en		= (w2 >> 4) & 1; // 1
	OtherModes.antialias_en		= (w2 >> 3) & 1; // 1
	OtherModes.z_source_sel		= (w2 >> 2) & 1; // 0
	OtherModes.dither_alpha_en	= (w2 >> 1) & 1; // 0
	OtherModes.alpha_compare_en	= (w2) & 1; // 0
}

void n64_rdp::CmdLoadTLUT(UINT32 w1, UINT32 w2)
{
	//wait("LoadTLUT");
	N64Tile* tile = m_tiles;

	int tilenum = (w2 >> 24) & 0x7;
	int sl = tile[tilenum].sl = ((w1 >> 12) & 0xfff);
	int tl = tile[tilenum].tl =  w1 & 0xfff;
	int sh = tile[tilenum].sh = ((w2 >> 12) & 0xfff);
	int th = tile[tilenum].th = w2 & 0xfff;

	if (tl != th)
	{
		fatalerror("Load tlut: tl=%d, th=%d\n",tl,th);
	}

	int count = (sh >> 2) - (sl >> 2) + 1;
	count <<= 2;

	switch (MiscState.TISize)
	{
		case PIXEL_SIZE_16BIT:
		{
			if (tile[tilenum].tmem < 256)
			{
				fatalerror("rdp_load_tlut: loading tlut into low half at %d qwords\n",tile[tilenum].tmem);
			}
			UINT32 srcstart = (MiscState.TIAddress + (tl >> 2) * (MiscState.TIWidth << 1) + (sl >> 1)) >> 1;
			UINT16 *dst = GetTMEM16();
			UINT32 dststart = tile[tilenum].tmem << 2;

			for (int i = 0; i < count; i += 4)
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
		default:	fatalerror("RDP: load_tlut: size = %d\n", MiscState.TISize);
	}
}

void n64_rdp::CmdSetTileSize(UINT32 w1, UINT32 w2)
{
	//wait("SetTileSize");

	const int tilenum = (w2 >> 24) & 0x7;

	m_tiles[tilenum].sl = (w1 >> 12) & 0xfff;
	m_tiles[tilenum].tl = (w1 >>  0) & 0xfff;
	m_tiles[tilenum].sh = (w2 >> 12) & 0xfff;
	m_tiles[tilenum].th = (w2 >>  0) & 0xfff;
}

void n64_rdp::CmdLoadBlock(UINT32 w1, UINT32 w2)
{
	//wait("LoadBlock");
	N64Tile* tile = m_tiles;

	int tilenum = (w2 >> 24) & 0x7;
	UINT16* tc = GetTMEM16();

	UINT16 sl, sh, tl;
	tile[tilenum].sl = sl = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl = tl = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh = sh = ((w2 >> 12) & 0xfff);
	UINT16 dxt = ((w2 >>  0) & 0xfff);

	if (sh < sl)
	{
		fatalerror("load_block: sh < sl\n");
	}

	INT32 width = (sh - sl) + 1;

	width = (width << MiscState.TISize) >> 1;
	if (width & 7)
	{
		width = (width & ~7) + 8;
	}
	width >>= 3;

	UINT32 tb = tile[tilenum].tmem << 2;

	int tiwinwords = MiscState.TIWidth;
	UINT32 slinwords = sl;

	tiwinwords = (tiwinwords << MiscState.TISize) >> 2;
	slinwords = (slinwords << MiscState.TISize) >> 2;

	int ptr = 0, srcptr = 0;
	UINT16 first, sec;
	UINT32 src = (MiscState.TIAddress >> 1) + (tl * tiwinwords) + slinwords;

	if (dxt != 0)
	{
		int j = 0;
		int t = 0;
		int oldt = 0;

		if (tile[tilenum].size != PIXEL_SIZE_32BIT && tile[tilenum].format != FORMAT_YUV)
		{
			for (int i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				ptr = tb + (i << 2);
				srcptr = src + (i << 2);

				tc[(ptr ^ t) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ t) & 0x7ff] = U_RREADIDX16(srcptr + 3);
				j += dxt;
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (int i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
				{
					i += tile[tilenum].line;
				}

				ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				srcptr = src + (i << 2);

				first = U_RREADIDX16(srcptr);
				sec = U_RREADIDX16(srcptr + 1);
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
			for (int i = 0; i < width; i ++)
			{
				oldt = t;
				t = ((j >> 11) & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				if (t != oldt)
					i += tile[tilenum].line;

				ptr = ((tb + (i << 1)) ^ t) & 0x3ff;
				srcptr = src + (i << 2);
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
			for (int i = 0; i < width; i ++)
			{
				ptr = tb + (i << 2);
				srcptr = src + (i << 2);
				tc[(ptr ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr);
				tc[((ptr + 1) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 1);
				tc[((ptr + 2) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 2);
				tc[((ptr + 3) ^ WORD_ADDR_XOR) & 0x7ff] = U_RREADIDX16(srcptr + 3);
			}
		}
		else if (tile[tilenum].format == FORMAT_YUV)
		{
			for (int i = 0; i < width; i ++)
			{
				ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				srcptr = src + (i << 2);
				first = U_RREADIDX16(srcptr);
				sec = U_RREADIDX16(srcptr + 1);
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
			for (int i = 0; i < width; i ++)
			{
				ptr = ((tb + (i << 1)) ^ WORD_ADDR_XOR) & 0x3ff;
				srcptr = src + (i << 2);
				tc[ptr] = U_RREADIDX16(srcptr);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 1);

				ptr = ((tb + (i << 1) + 1) ^ WORD_ADDR_XOR) & 0x3ff;
				tc[ptr] = U_RREADIDX16(srcptr + 2);
				tc[ptr | 0x400] = U_RREADIDX16(srcptr + 3);
			}
		}
		tile[tilenum].th = tl;
	}
}

void n64_rdp::CmdLoadTile(UINT32 w1, UINT32 w2)
{
	//wait("LoadTile");
	N64Tile* tile = m_tiles;
	int tilenum = (w2 >> 24) & 0x7;

	tile[tilenum].sl	= ((w1 >> 12) & 0xfff);
	tile[tilenum].tl	= ((w1 >>  0) & 0xfff);
	tile[tilenum].sh	= ((w2 >> 12) & 0xfff);
	tile[tilenum].th	= ((w2 >>  0) & 0xfff);

	UINT16 sl = tile[tilenum].sl >> 2;
	UINT16 tl = tile[tilenum].tl >> 2;
	UINT16 sh = tile[tilenum].sh >> 2;
	UINT16 th = tile[tilenum].th >> 2;

	INT32 width = (sh - sl) + 1;
	INT32 height = (th - tl) + 1;
/*
    int topad;
    if (MiscState.TISize < 3)
    {
        topad = (width * MiscState.TISize) & 0x7;
    }
    else
    {
        topad = (width << 2) & 0x7;
    }
    topad = 0; // ????
*/
	switch (MiscState.TISize)
	{
		case PIXEL_SIZE_8BIT:
		{
			UINT32 src = MiscState.TIAddress;
			UINT8 *tc = GetTMEM();
			int tb = tile[tilenum].tmem << 3;

			for (int j = 0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line << 3) * j);
				int s = ((j + tl) * MiscState.TIWidth) + sl;

				int xorval8 = ((j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
				for (int i = 0; i < width; i++)
				{
					tc[((tline + i) ^ xorval8) & 0xfff] = U_RREADADDR8(src + s + i);
				}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			UINT32 src = MiscState.TIAddress >> 1;
			UINT16 *tc = GetTMEM16();
			UINT16 yuvword;

			if (tile[tilenum].format != FORMAT_YUV)
			{
				for (int j = 0; j < height; j++)
				{
					int tb = tile[tilenum].tmem << 2;
					int tline = tb + ((tile[tilenum].line << 2) * j);
					int s = ((j + tl) * MiscState.TIWidth) + sl;
					int xorval16 = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;

					for (int i = 0; i < width; i++)
					{
						UINT32 taddr = (tline + i) ^ xorval16;
						tc[taddr & 0x7ff] = U_RREADIDX16(src + s + i);
					}
				}
			}
			else
			{
				for (int j = 0; j < height; j++)
				{
					int tb = tile[tilenum].tmem << 3;
					int tline = tb + ((tile[tilenum].line << 3) * j);
					int s = ((j + tl) * MiscState.TIWidth) + sl;
					int xorval8 = (j & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;

					for (int i = 0; i < width; i++)
					{
						UINT32 taddr = ((tline + i) ^ xorval8) & 0x7ff;
						yuvword = U_RREADIDX16(src + s + i);
						GetTMEM()[taddr] = yuvword >> 8;
						GetTMEM()[taddr | 0x800] = yuvword & 0xff;
					}
				}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			UINT32 src = MiscState.TIAddress >> 2;
			UINT16 *tc16 = GetTMEM16();
			int tb = (tile[tilenum].tmem << 2);

			for (int j = 0; j < height; j++)
			{
				int tline = tb + ((tile[tilenum].line << 2) * j);

				int s = ((j + tl) * MiscState.TIWidth) + sl;
				int xorval32cur = (j & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
				for (int i = 0; i < width; i++)
				{
					UINT32 c = U_RREADIDX32(src + s + i);
					UINT32 ptr = ((tline + i) ^ xorval32cur) & 0x3ff;
					tc16[ptr] = c >> 16;
					tc16[ptr | 0x400] = c & 0xffff;
				}
			}
			break;
		}

		default:	fatalerror("RDP: load_tile: size = %d\n", MiscState.TISize);
	}
}

void n64_rdp::CmdSetTile(UINT32 w1, UINT32 w2)
{
	//wait("SetTile");

	int tilenum = (w2 >> 24) & 0x7;
	N64Tile* tex_tile = &m_tiles[tilenum];

	tex_tile->format	= (w1 >> 21) & 0x7;
	tex_tile->size		= (w1 >> 19) & 0x3;
	tex_tile->line		= (w1 >>  9) & 0x1ff;
	tex_tile->tmem		= (w1 >>  0) & 0x1ff;
	tex_tile->palette	= (w2 >> 20) & 0xf;
	tex_tile->ct		= (w2 >> 19) & 0x1;
	tex_tile->mt		= (w2 >> 18) & 0x1;
	tex_tile->mask_t	= (w2 >> 14) & 0xf;
	tex_tile->shift_t	= (w2 >> 10) & 0xf;
	tex_tile->cs		= (w2 >>  9) & 0x1;
	tex_tile->ms		= (w2 >>  8) & 0x1;
	tex_tile->mask_s	= (w2 >>  4) & 0xf;
	tex_tile->shift_s	= (w2 >>  0) & 0xf;

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

void n64_rdp::CmdFillRect(UINT32 w1, UINT32 w2)
{
	//if(m_pending_mode_block) { wait("Block on pending mode-change"); m_pending_mode_block = false; }

	UINT32 xl = (w1 >> 12) & 0xfff;
	UINT32 yl = (w1 >>  0) & 0xfff;
	UINT32 xh = (w2 >> 12) & 0xfff;
	UINT32 yh = (w2 >>  0) & 0xfff;

	if (OtherModes.cycle_type == CYCLE_TYPE_FILL || OtherModes.cycle_type == CYCLE_TYPE_COPY)
	{
		yl |= 3;
	}

	UINT32 xlint = (xl >> 2) & 0x3ff;
	UINT32 xhint = (xh >> 2) & 0x3ff;

	UINT32* ewdata = m_temp_rect_data;
	ewdata[0] = (0x3680 << 16) | yl;//command, flipped, tile, yl
	ewdata[1] = (yl << 16) | yh;//ym, yh
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);//xl, xl frac
	ewdata[3] = 0;//dxldy, dxldy frac
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);//xh, xh frac
	ewdata[5] = 0;//dxhdy, dxhdy frac
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);//xm, xm frac
	ewdata[7] = 0;//dxmdy, dxmdy frac
	memset(&ewdata[8], 0, 36 * sizeof(UINT32));//shade, texture, depth

	DrawTriangle(false, false, false, true);
}

void n64_rdp::CmdSetFogColor(UINT32 w1, UINT32 w2)
{
	FogColor.c = w2;
}

void n64_rdp::CmdSetBlendColor(UINT32 w1, UINT32 w2)
{
	BlendColor.c = w2;
}

void n64_rdp::CmdSetPrimColor(UINT32 w1, UINT32 w2)
{
	MiscState.MinLevel = (w1 >> 8) & 0x1f;
	PrimLODFraction = w1 & 0xff;
	PrimColor.c = w2;
}

void n64_rdp::CmdSetEnvColor(UINT32 w1, UINT32 w2)
{
	EnvColor.c = w2;
}

void n64_rdp::CmdSetCombine(UINT32 w1, UINT32 w2)
{
	m_combine.sub_a_rgb0	= (w1 >> 20) & 0xf;
	m_combine.mul_rgb0		= (w1 >> 15) & 0x1f;
	m_combine.sub_a_a0		= (w1 >> 12) & 0x7;
	m_combine.mul_a0		= (w1 >>  9) & 0x7;
	m_combine.sub_a_rgb1	= (w1 >>  5) & 0xf;
	m_combine.mul_rgb1		= (w1 >>  0) & 0x1f;

	m_combine.sub_b_rgb0	= (w2 >> 28) & 0xf;
	m_combine.sub_b_rgb1	= (w2 >> 24) & 0xf;
	m_combine.sub_a_a1		= (w2 >> 21) & 0x7;
	m_combine.mul_a1		= (w2 >> 18) & 0x7;
	m_combine.add_rgb0		= (w2 >> 15) & 0x7;
	m_combine.sub_b_a0		= (w2 >> 12) & 0x7;
	m_combine.add_a0		= (w2 >>  9) & 0x7;
	m_combine.add_rgb1		= (w2 >>  6) & 0x7;
	m_combine.sub_b_a1		= (w2 >>  3) & 0x7;
	m_combine.add_a1		= (w2 >>  0) & 0x7;
}

void n64_rdp::CmdSetTextureImage(UINT32 w1, UINT32 w2)
{
	MiscState.TIFormat	= (w1 >> 21) & 0x7;
	MiscState.TISize	= (w1 >> 19) & 0x3;
	MiscState.TIWidth	= (w1 & 0x3ff) + 1;
	MiscState.TIAddress	= w2 & 0x01ffffff;
}

void n64_rdp::CmdSetMaskImage(UINT32 w1, UINT32 w2)
{
	//wait("SetMaskImage");

	MiscState.ZBAddress = w2 & 0x01ffffff;
}

void n64_rdp::CmdSetColorImage(UINT32 w1, UINT32 w2)
{
	//wait("SetColorImage");

	MiscState.FBFormat	= (w1 >> 21) & 0x7;
	MiscState.FBSize	= (w1 >> 19) & 0x3;
	MiscState.FBWidth		= (w1 & 0x3ff) + 1;
	MiscState.FBAddress	= w2 & 0x01ffffff;

	if (MiscState.FBFormat < 2 || MiscState.FBFormat > 32) // Jet Force Gemini sets the format to 4, Intensity.  Protection?
	{
		MiscState.FBFormat = 2;
	}
}

UINT32 n64_rdp::AddRightCvg(UINT32 x, UINT32 k)
{
//#undef FULL_SUBPIXELS
#define FULL_SUBPIXELS
	UINT32 coveredsubpixels=((x >> 14) & 3);
	if (!(x & 0xffff))
	{
		return 0;
	}
#ifdef FULL_SUBPIXELS
	if (!coveredsubpixels)
	{
		return 0;
	}
	if (!(k & 1))
	{
		return (coveredsubpixels<3) ? 1 : 2;
	}
	else
	{
		return (coveredsubpixels<2) ? 0 : 1;
	}
#endif
	if (!(k & 1))
	{
		return (coveredsubpixels<2) ? 1 : 2;
	}
	else
	{
		if (coveredsubpixels<1)
		{
			return 0;
		}
		else if (coveredsubpixels<3)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

UINT32 n64_rdp::AddLeftCvg(UINT32 x, UINT32 k)
{
	UINT32 coveredsubpixels = 3 - ((x >> 14) & 3);
	if (!(x & 0xffff))
	{
		return 2;
	}
#ifdef FULL_SUBPIXELS
	if (!coveredsubpixels)
	{
		return 0;
	}
	if (!(k & 1))
	{
		return (coveredsubpixels<2) ? 0 : 1;
	}
	else
	{
		return (coveredsubpixels<3) ? 1 : 2;
	}
#endif
	if (k & 1)
	{
		return (coveredsubpixels<2) ? 1 : 2;
	}
	else
	{
		if (coveredsubpixels < 1)
		{
			return 0;
		}
		else if (coveredsubpixels < 3)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

/*****************************************************************************/

void n64_rdp::CmdInvalid(UINT32 w1, UINT32 w2)
{
	fatalerror("n64_rdp::Invalid: %d, %08x %08x\n", (w1 >> 24) & 0x3f, w1, w2);
}

void n64_rdp::CmdNoOp(UINT32 w1, UINT32 w2)
{
	// Do nothing
}


void n64_rdp::ProcessList()
{
	INT32 length = m_end - m_current;

	if(length < 0)
	{
		m_current = m_end;
		return;
	}

	// load command data
	for(int i = 0; i < length; i += 4)
	{
		m_cmd_data[m_cmd_ptr++] = ReadData((m_current & 0x1fffffff) + i);
	}

	m_current = m_end;

	UINT32 cmd = (m_cmd_data[0] >> 24) & 0x3f;
	UINT32 cmd_length = (m_cmd_ptr + 1) * 4;

	SetStatusReg(GetStatusReg() &~ DP_STATUS_FREEZE);

	// check if more data is needed
	if (cmd_length < rdp_command_length[cmd])
	{
		return;
	}

	while (m_cmd_cur < m_cmd_ptr)
	{
		cmd = (m_cmd_data[m_cmd_cur] >> 24) & 0x3f;

		if (((m_cmd_ptr - m_cmd_cur) * 4) < rdp_command_length[cmd])
		{
			return;
			//fatalerror("rdp_process_list: not enough rdp command data: cur = %d, ptr = %d, expected = %d\n", m_cmd_cur, m_cmd_ptr, rdp_command_length[cmd]);
		}

		if (LOG_RDP_EXECUTION)
		{
			char string[4000];
			Dasm(string);

			fprintf(rdp_exec, "%08X: %08X %08X   %s\n", m_start+(m_cmd_cur * 4), m_cmd_data[m_cmd_cur+0], m_cmd_data[m_cmd_cur+1], string);
			fflush(rdp_exec);
		}

		// execute the command
		UINT32 w1 = m_cmd_data[m_cmd_cur+0];
		UINT32 w2 = m_cmd_data[m_cmd_cur+1];

		switch(cmd)
		{
			case 0x00:	CmdNoOp(w1, w2);			break;

			case 0x08:	CmdTriangle(w1, w2);		break;
			case 0x09:	CmdTriangleZ(w1, w2);		break;
			case 0x0a:	CmdTriangleT(w1, w2);		break;
			case 0x0b:	CmdTriangleTZ(w1, w2);		break;
			case 0x0c:	CmdTriangleS(w1, w2);		break;
			case 0x0d:	CmdTriangleSZ(w1, w2);		break;
			case 0x0e:	CmdTriangleST(w1, w2);		break;
			case 0x0f:	CmdTriangleSTZ(w1, w2);		break;

			case 0x24:	CmdTexRect(w1, w2);			break;
			case 0x25:	CmdTexRectFlip(w1, w2);		break;

			case 0x26:	CmdSyncLoad(w1, w2);		break;
			case 0x27:	CmdSyncPipe(w1, w2);		break;
			case 0x28:	CmdSyncTile(w1, w2);		break;
			case 0x29:	CmdSyncFull(w1, w2);		break;

			case 0x2a:	CmdSetKeyGB(w1, w2);		break;
			case 0x2b:	CmdSetKeyR(w1, w2);			break;

			case 0x2c:	CmdSetConvert(w1, w2);		break;
			case 0x3c:	CmdSetCombine(w1, w2);		break;
			case 0x2d:	CmdSetScissor(w1, w2);		break;
			case 0x2e:	CmdSetPrimDepth(w1, w2);	break;
			case 0x2f:	CmdSetOtherModes(w1, w2);	break;

			case 0x30:	CmdLoadTLUT(w1, w2);		break;
			case 0x33:	CmdLoadBlock(w1, w2);		break;
			case 0x34:	CmdLoadTile(w1, w2);		break;

			case 0x32:	CmdSetTileSize(w1, w2);		break;
			case 0x35:	CmdSetTile(w1, w2);			break;

			case 0x36:	CmdFillRect(w1, w2);		break;

			case 0x37:	CmdSetFillColor32(w1, w2);	break;
			case 0x38:	CmdSetFogColor(w1, w2);		break;
			case 0x39:	CmdSetBlendColor(w1, w2);	break;
			case 0x3a:	CmdSetPrimColor(w1, w2);	break;
			case 0x3b:	CmdSetEnvColor(w1, w2);		break;

			case 0x3d:	CmdSetTextureImage(w1, w2);	break;
			case 0x3e:	CmdSetMaskImage(w1, w2);	break;
			case 0x3f:	CmdSetColorImage(w1, w2);	break;
		}

		m_cmd_cur += rdp_command_length[cmd] / 4;
	};
	m_cmd_ptr = 0;
	m_cmd_cur = 0;

	m_start = m_current = m_end;
}

/*****************************************************************************/

n64_rdp::n64_rdp(n64_state &state) : poly_manager<UINT32, rdp_poly_state, 8, 32000>(state.machine())
{
	AuxBufPtr = 0;
	AuxBuf = NULL;
	m_pipe_clean = true;

	m_pending_mode_block = false;

	m_cmd_ptr = 0;
	m_cmd_cur = 0;

	m_start = 0;
	m_end = 0;
	m_current = 0;
	m_status = 0x88;

	for (int i = 0; i < 8; i++)
	{
		m_tiles[i].num = i;
	}

	OneColor.c = 0xffffffff;
	ZeroColor.c = 0x00000000;

	m_tmem = NULL;

	m_machine = NULL;

	//memset(m_hidden_bits, 3, 8388608);

	PrimLODFraction = 0;

	for (int i = 0; i < 256; i++)
	{
		m_gamma_table[i] = sqrt((float)(i << 6));
		m_gamma_table[i] <<= 1;
	}

	for (int i = 0; i < 0x4000; i++)
	{
		m_gamma_dither_table[i] = sqrt((float)i);
		m_gamma_dither_table[i] <<= 1;
	}

	z_build_com_table();

	for (int i = 0; i < 0x4000; i++)
	{
		UINT32 exponent = (i >> 11) & 7;
		UINT32 mantissa = i & 0x7ff;
		z_complete_dec_table[i] = ((mantissa << z_dec_table[exponent].shift) + z_dec_table[exponent].add) & 0x3fffff;
	}

	precalc_cvmask_derivatives();

	for(int i = 0; i < 0x200; i++)
	{
		switch((i >> 7) & 3)
		{
		case 0:
		case 1:
			m_special_9bit_clamptable[i] = i & 0xff;
			break;
		case 2:
			m_special_9bit_clamptable[i] = 0xff;
			break;
		case 3:
			m_special_9bit_clamptable[i] = 0;
			break;
		}
	}

	for(int i = 0; i < 32; i++)
	{
		ReplicatedRGBA[i] = (i << 3) | ((i >> 2) & 7);
	}

	_Write[0] = &n64_rdp::_Write16Bit_Cvg0_NoBlend;
	_Write[1] = &n64_rdp::_Write16Bit_Cvg0_Blend;
	_Write[2] = &n64_rdp::_Write16Bit_Cvg1;
	_Write[3] = &n64_rdp::_Write16Bit_Cvg1;
	_Write[4] = &n64_rdp::_Write16Bit_Cvg2;
	_Write[5] = &n64_rdp::_Write16Bit_Cvg2;
	_Write[6] = &n64_rdp::_Write16Bit_Cvg3;
	_Write[7] = &n64_rdp::_Write16Bit_Cvg3;
	_Write[8] = &n64_rdp::_Write32Bit_Cvg0_NoBlend;
	_Write[9] = &n64_rdp::_Write32Bit_Cvg0_Blend;
	_Write[10] = &n64_rdp::_Write32Bit_Cvg1;
	_Write[11] = &n64_rdp::_Write32Bit_Cvg1;
	_Write[12] = &n64_rdp::_Write32Bit_Cvg2;
	_Write[13] = &n64_rdp::_Write32Bit_Cvg2;
	_Write[14] = &n64_rdp::_Write32Bit_Cvg3;
	_Write[15] = &n64_rdp::_Write32Bit_Cvg3;

	_Read[0] = &n64_rdp::_Read16Bit_ImgRead0;
	_Read[1] = &n64_rdp::_Read16Bit_ImgRead1;
	_Read[2] = &n64_rdp::_Read32Bit_ImgRead0;
	_Read[3] = &n64_rdp::_Read32Bit_ImgRead1;

	_Copy[0] = &n64_rdp::_Copy16Bit;
	_Copy[1] = &n64_rdp::_Copy32Bit;

	_Fill[0] = &n64_rdp::_Fill16Bit;
	_Fill[1] = &n64_rdp::_Fill32Bit;
}

VIDEO_START(n64)
{
	n64_state *state = machine.driver_data<n64_state>();

	state->m_rdp = auto_alloc(machine, n64_rdp(*state));

	state->m_rdp->SetMachine(machine);
	state->m_rdp->InitInternalState();

	state->m_rdp->Blender.SetMachine(machine);
	state->m_rdp->Blender.SetProcessor(state->m_rdp);

	state->m_rdp->TexPipe.SetMachine(machine);

	state->m_rdp->AuxBuf = auto_alloc_array_clear(machine, UINT8, EXTENT_AUX_COUNT);

	if (LOG_RDP_EXECUTION)
	{
		rdp_exec = fopen("rdp_execute.txt", "wt");
	}
}

SCREEN_UPDATE_RGB32(n64)
{
	n64_state *state = screen.machine().driver_data<n64_state>();
	n64_periphs *n64 = screen.machine().device<n64_periphs>("rcp");
    state->m_rdp->visarea = screen.visible_area();

	//UINT16 *frame_buffer = (UINT16*)&rdram[(n64->vi_origin & 0xffffff) >> 2];
	//UINT8  *cvg_buffer = &state->m_rdp.HiddenBits[((n64->vi_origin & 0xffffff) >> 2) >> 1];
    //int vibuffering = ((n64->vi_control & 2) && fsaa && divot);

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

	state->m_rdp->wait();
	state->m_rdp->AuxBufPtr = 0;

    if (n64->vi_blank)
    {
		bitmap.fill(0, state->m_rdp->visarea);
        return 0;
    }

	state->m_rdp->VideoUpdate(n64, bitmap);

	return 0;
}
