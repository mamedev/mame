/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL) helper macros
    -------------------

    by MooglyGuy
    based on code by Ville Linde, MooglyGuy, angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef _VIDEO_RDPBHELP_H_
#define _VIDEO_RDPBHELP_H_

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

#define DITHER_A(val, dith)     \
	if ((val + dith) >= 0x100)  \
	{                           \
		val = 0xff;             \
	}                           \
	else                        \
	{                           \
		val += dith;            \
	}

#define DITHER_CHAN(chan, dith)     \
	if ((chan & 7) > dith)          \
	{                               \
		chan = (chan & 0xf8) + 8;   \
		if (chan > 247)             \
		{                           \
			chan = 255;             \
		}                           \
	}

#define DITHER_RGB(dith)    \
	DITHER_CHAN(r, dith)    \
	DITHER_CHAN(g, dith)    \
	DITHER_CHAN(b, dith)

#define ALPHA_COMPARE() \
	if (((this)->*(compare[acmode]))(userdata->PixelColor.i.a, userdata, object))   \
	{                                                               \
		return false;                                               \
	}

#define CVG_COMPARE() \
	if (object.OtherModes.antialias_en ? (!userdata->CurrentPixCvg) : (!userdata->CurrentCvgBit))   \
	{                                                               \
		return false;                                               \
	}

#define TEST_REJECT() \
	ALPHA_COMPARE(); \
	CVG_COMPARE();

#define WRITE_OUT_NB_ND(cycle) \
	*fr = *userdata->ColorInputs.blender1a_r[cycle];    \
	*fg = *userdata->ColorInputs.blender1a_g[cycle];    \
	*fb = *userdata->ColorInputs.blender1a_b[cycle];

#define WRITE_OUT() \
	*fr = r;    \
	*fg = g;    \
	*fb = b;

#define WRITE_BLENDED_COLOR() \
	userdata->BlendedPixelColor.i.r = r;    \
	userdata->BlendedPixelColor.i.g = g;    \
	userdata->BlendedPixelColor.i.b = b;    \
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

#define BLEND_CYCLE0(cyc) \
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[cyc];   \
	((this)->*(cycle##cyc[sel##cyc]))(&r, &g, &b, userdata, object);
#define BLEND_CYCLE1(cyc) \
	if (partialreject && userdata->PixelColor.i.a >= 0xff)  \
	{                                                       \
		ASSIGN_OUT(cyc);                                    \
	}                                                       \
	else                                                    \
	{                                                       \
		userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[cyc];   \
		((this)->*(cycle##cyc[sel##cyc]))(&r, &g, &b, userdata, object);    \
	}

#define BLEND_CYCLE(cyc, check_reject) \
	BLEND_CYCLE##check_reject(cyc)

#define BLEND_FACTORS0(cycle) \
	UINT8 blend1a = *userdata->ColorInputs.blender1b_a[cycle] >> 3; \
	UINT8 blend2a = *userdata->ColorInputs.blender2b_a[cycle] >> 3;

#define BLEND_FACTORS1(cycle) \
	UINT8 blend1a = (*userdata->ColorInputs.blender1b_a[cycle] >> (3 + userdata->ShiftA)) & 0x1c;   \
	UINT8 blend2a = (*userdata->ColorInputs.blender2b_a[cycle] >> (3 + userdata->ShiftB)) & 0x1c;

#define BLEND_SUM0()    ;

#define BLEND_SUM1() \
	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

#define BLEND_FACTORS(cycle, special, sum)  \
	BLEND_FACTORS##special(cycle);          \
	BLEND_SUM##sum();

#define BLEND_MUL(cycle) \
	*r = (((int)(*userdata->ColorInputs.blender1a_r[cycle]) * (int)(blend1a))) +    \
		(((int)(*userdata->ColorInputs.blender2a_r[cycle]) * (int)(blend2a)));      \
	*g = (((int)(*userdata->ColorInputs.blender1a_g[cycle]) * (int)(blend1a))) +    \
		(((int)(*userdata->ColorInputs.blender2a_g[cycle]) * (int)(blend2a)));      \
	*b = (((int)(*userdata->ColorInputs.blender1a_b[cycle]) * (int)(blend1a))) +    \
		(((int)(*userdata->ColorInputs.blender2a_b[cycle]) * (int)(blend2a)));

#define BLEND_ADD1(cycle) \
	*r += (((int)*userdata->ColorInputs.blender2a_r[cycle]) << 2);  \
	*g += (((int)*userdata->ColorInputs.blender2a_g[cycle]) << 2);  \
	*b += (((int)*userdata->ColorInputs.blender2a_b[cycle]) << 2);

#define BLEND_ADD0(cycle) \
	*r += (int)*userdata->ColorInputs.blender2a_r[cycle];   \
	*g += (int)*userdata->ColorInputs.blender2a_g[cycle];   \
	*b += (int)*userdata->ColorInputs.blender2a_b[cycle];

#define BLEND_ADD(cycle, special)   \
	BLEND_ADD##special(cycle);

#define BLEND_SHIFT(shift)  \
	*r >>= shift;           \
	*g >>= shift;           \
	*b >>= shift;

#define BLEND_CLAMP()       \
	if (*r > 255) *r = 255; \
	if (*g > 255) *g = 255; \
	if (*b > 255) *b = 255;

#define BLEND_SCALE()           \
	if (sum)                    \
	{                           \
		*r /= sum;              \
		*g /= sum;              \
		*b /= sum;              \
	}                           \
	else                        \
	{                           \
		*r = *g = *b = 0xff;    \
	}

#define BLEND_SCALE_CLAMP0()    \
	BLEND_CLAMP();

#define BLEND_SCALE_CLAMP1()    \
	BLEND_SCALE();              \
	BLEND_CLAMP();

#define BLEND_SCALE_CLAMP(sum)  \
	BLEND_SCALE_CLAMP##sum();

#define ASSIGN_OUT(cycle) \
	r = *userdata->ColorInputs.blender1a_r[cycle];  \
	g = *userdata->ColorInputs.blender1a_g[cycle];  \
	b = *userdata->ColorInputs.blender1a_b[cycle];

#endif // _VIDEO_RDPBHELP_H_
