/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Functions for managing ASTC codec images.
 */
/*----------------------------------------------------------------------------*/

#include <math.h>

#include "astc_codec_internals.h"

#include "softfloat.h"
#include <stdint.h>
#include <stdio.h>

// conversion functions between the LNS representation and the FP16 representation.

float float_to_lns(float p)
{

	if (astc_isnan(p) || p <= 1.0f / 67108864.0f)
	{
		// underflow or NaN value, return 0.
		// We count underflow if the input value is smaller than 2^-26.
		return 0;
	}

	if (fabs(p) >= 65536.0f)
	{
		// overflow, return a +INF value
		return 65535;
	}

	int expo;
	float normfrac = frexp(p, &expo);
	float p1;
	if (expo < -13)
	{
		// input number is smaller than 2^-14. In this case, multiply by 2^25.
		p1 = p * 33554432.0f;
		expo = 0;
	}
	else
	{
		expo += 14;
		p1 = (normfrac - 0.5f) * 4096.0f;
	}

	if (p1 < 384.0f)
		p1 *= 4.0f / 3.0f;
	else if (p1 <= 1408.0f)
		p1 += 128.0f;
	else
		p1 = (p1 + 512.0f) * (4.0f / 5.0f);

	p1 += expo * 2048.0f;
	return p1 + 1.0f;
}



uint16_t lns_to_sf16(uint16_t p)
{

	uint16_t mc = p & 0x7FF;
	uint16_t ec = p >> 11;
	uint16_t mt;
	if (mc < 512)
		mt = 3 * mc;
	else if (mc < 1536)
		mt = 4 * mc - 512;
	else
		mt = 5 * mc - 2048;

	uint16_t res = (ec << 10) | (mt >> 3);
	if (res >= 0x7BFF)
		res = 0x7BFF;
	return res;
}


// conversion function from 16-bit LDR value to FP16.
// note: for LDR interpolation, it is impossible to get a denormal result;
// this simplifies the conversion.
// FALSE; we can receive a very small UNORM16 through the constant-block.
uint16_t unorm16_to_sf16(uint16_t p)
{
	if (p == 0xFFFF)
		return 0x3C00;			// value of 1.0 .
	if (p < 4)
		return p << 8;

	int lz = clz32(p) - 16;
	p <<= (lz + 1);
	p >>= 6;
	p |= (14 - lz) << 10;
	return p;
}





void imageblock_initialize_deriv_from_work_and_orig(imageblock * pb, int pixelcount)
{
	int i;

	const float *fptr = pb->orig_data;
	const float *wptr = pb->work_data;
	float *dptr = pb->deriv_data;

	for (i = 0; i < pixelcount; i++)
	{

		// compute derivatives for RGB first
		if (pb->rgb_lns[i])
		{
			float r = MAX(fptr[0], 6e-5f);
			float g = MAX(fptr[1], 6e-5f);
			float b = MAX(fptr[2], 6e-5f);

			float rderiv = (float_to_lns(r * 1.05f) - float_to_lns(r)) / (r * 0.05f);
			float gderiv = (float_to_lns(g * 1.05f) - float_to_lns(g)) / (g * 0.05f);
			float bderiv = (float_to_lns(b * 1.05f) - float_to_lns(b)) / (b * 0.05f);

			// the derivative may not actually take values smaller than 1/32 or larger than 2^25;
			// if it does, we clamp it.
			if (rderiv < (1.0f / 32.0f))
				rderiv = (1.0f / 32.0f);
			else if (rderiv > 33554432.0f)
				rderiv = 33554432.0f;

			if (gderiv < (1.0f / 32.0f))
				gderiv = (1.0f / 32.0f);
			else if (gderiv > 33554432.0f)
				gderiv = 33554432.0f;

			if (bderiv < (1.0f / 32.0f))
				bderiv = (1.0f / 32.0f);
			else if (bderiv > 33554432.0f)
				bderiv = 33554432.0f;

			dptr[0] = rderiv;
			dptr[1] = gderiv;
			dptr[2] = bderiv;
		}
		else
		{
			dptr[0] = 65535.0f;
			dptr[1] = 65535.0f;
			dptr[2] = 65535.0f;
		}


		// then compute derivatives for Alpha
		if (pb->alpha_lns[i])
		{
			float a = MAX(fptr[3], 6e-5f);
			float aderiv = (float_to_lns(a * 1.05f) - float_to_lns(a)) / (a * 0.05f);
			// the derivative may not actually take values smaller than 1/32 or larger than 2^25;
			// if it does, we clamp it.
			if (aderiv < (1.0f / 32.0f))
				aderiv = (1.0f / 32.0f);
			else if (aderiv > 33554432.0f)
				aderiv = 33554432.0f;

			dptr[3] = aderiv;
		}
		else
		{
			dptr[3] = 65535.0f;
		}

		fptr += 4;
		wptr += 4;
		dptr += 4;
	}
}




// helper function to initialize the work-data from the orig-data
void imageblock_initialize_work_from_orig(imageblock * pb, int pixelcount)
{
	int i;
	float *fptr = pb->orig_data;
	float *wptr = pb->work_data;

	for (i = 0; i < pixelcount; i++)
	{
		if (pb->rgb_lns[i])
		{
			wptr[0] = float_to_lns(fptr[0]);
			wptr[1] = float_to_lns(fptr[1]);
			wptr[2] = float_to_lns(fptr[2]);
		}
		else
		{
			wptr[0] = fptr[0] * 65535.0f;
			wptr[1] = fptr[1] * 65535.0f;
			wptr[2] = fptr[2] * 65535.0f;
		}

		if (pb->alpha_lns[i])
		{
			wptr[3] = float_to_lns(fptr[3]);
		}
		else
		{
			wptr[3] = fptr[3] * 65535.0f;
		}
		fptr += 4;
		wptr += 4;
	}

	imageblock_initialize_deriv_from_work_and_orig(pb, pixelcount);
}




// helper function to initialize the orig-data from the work-data
void imageblock_initialize_orig_from_work(imageblock * pb, int pixelcount)
{
	int i;
	float *fptr = pb->orig_data;
	float *wptr = pb->work_data;

	for (i = 0; i < pixelcount; i++)
	{
		if (pb->rgb_lns[i])
		{
			fptr[0] = sf16_to_float(lns_to_sf16((uint16_t) wptr[0]));
			fptr[1] = sf16_to_float(lns_to_sf16((uint16_t) wptr[1]));
			fptr[2] = sf16_to_float(lns_to_sf16((uint16_t) wptr[2]));
		}
		else
		{
			fptr[0] = sf16_to_float(unorm16_to_sf16((uint16_t) wptr[0]));
			fptr[1] = sf16_to_float(unorm16_to_sf16((uint16_t) wptr[1]));
			fptr[2] = sf16_to_float(unorm16_to_sf16((uint16_t) wptr[2]));
		}

		if (pb->alpha_lns[i])
		{
			fptr[3] = sf16_to_float(lns_to_sf16((uint16_t) wptr[3]));
		}
		else
		{
			fptr[3] = sf16_to_float(unorm16_to_sf16((uint16_t) wptr[3]));
		}

		fptr += 4;
		wptr += 4;
	}

	imageblock_initialize_deriv_from_work_and_orig(pb, pixelcount);
}


/*
   For an imageblock, update its flags.

   The updating is done based on work_data, not orig_data.
*/
void update_imageblock_flags(imageblock * pb, int xdim, int ydim, int zdim)
{
	int i;
	float red_min = 1e38f, red_max = -1e38f;
	float green_min = 1e38f, green_max = -1e38f;
	float blue_min = 1e38f, blue_max = -1e38f;
	float alpha_min = 1e38f, alpha_max = -1e38f;

	int texels_per_block = xdim * ydim * zdim;

	int grayscale = 1;

	for (i = 0; i < texels_per_block; i++)
	{
		float red = pb->work_data[4 * i];
		float green = pb->work_data[4 * i + 1];
		float blue = pb->work_data[4 * i + 2];
		float alpha = pb->work_data[4 * i + 3];
		if (red < red_min)
			red_min = red;
		if (red > red_max)
			red_max = red;
		if (green < green_min)
			green_min = green;
		if (green > green_max)
			green_max = green;
		if (blue < blue_min)
			blue_min = blue;
		if (blue > blue_max)
			blue_max = blue;
		if (alpha < alpha_min)
			alpha_min = alpha;
		if (alpha > alpha_max)
			alpha_max = alpha;

		if (grayscale == 1 && (red != green || red != blue))
			grayscale = 0;
	}

	pb->red_min = red_min;
	pb->red_max = red_max;
	pb->green_min = green_min;
	pb->green_max = green_max;
	pb->blue_min = blue_min;
	pb->blue_max = blue_max;
	pb->alpha_min = alpha_min;
	pb->alpha_max = alpha_max;
	pb->grayscale = grayscale;
}

