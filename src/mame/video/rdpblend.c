#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

N64BlenderT::N64BlenderT()
{
	blend1[0] = &N64BlenderT::Blend1CycleNoBlendNoACVGNoDither;
	blend1[1] = &N64BlenderT::Blend1CycleNoBlendNoACVGDither;
	blend1[2] = &N64BlenderT::Blend1CycleNoBlendACVGNoDither;
	blend1[3] = &N64BlenderT::Blend1CycleNoBlendACVGDither;
	blend1[4] = &N64BlenderT::Blend1CycleBlendNoACVGNoDither;
	blend1[5] = &N64BlenderT::Blend1CycleBlendNoACVGDither;
	blend1[6] = &N64BlenderT::Blend1CycleBlendACVGNoDither;
	blend1[7] = &N64BlenderT::Blend1CycleBlendACVGDither;

	blend2[0] = &N64BlenderT::Blend2CycleNoBlendNoACVGNoDither;
	blend2[1] = &N64BlenderT::Blend2CycleNoBlendNoACVGDither;
	blend2[2] = &N64BlenderT::Blend2CycleNoBlendACVGNoDither;
	blend2[3] = &N64BlenderT::Blend2CycleNoBlendACVGDither;
	blend2[4] = &N64BlenderT::Blend2CycleBlendNoACVGNoDither;
	blend2[5] = &N64BlenderT::Blend2CycleBlendNoACVGDither;
	blend2[6] = &N64BlenderT::Blend2CycleBlendACVGNoDither;
	blend2[7] = &N64BlenderT::Blend2CycleBlendACVGDither;

	cycle0[0] = &N64BlenderT::BlendEquationCycle0NoForceNoSpecial;
	cycle0[1] = &N64BlenderT::BlendEquationCycle0NoForceSpecial;
	cycle0[2] = &N64BlenderT::BlendEquationCycle0ForceNoSpecial;
	cycle0[3] = &N64BlenderT::BlendEquationCycle0ForceSpecial;

	cycle1[0] = &N64BlenderT::BlendEquationCycle1NoForceNoSpecial;
	cycle1[1] = &N64BlenderT::BlendEquationCycle1NoForceSpecial;
	cycle1[2] = &N64BlenderT::BlendEquationCycle1ForceNoSpecial;
	cycle1[3] = &N64BlenderT::BlendEquationCycle1ForceSpecial;
}

#define ALPHA_COMPARE()	\
	if (!AlphaCompare(userdata->PixelColor.i.a, userdata, object))	\
	{																\
		return false;												\
	}

#define CVG_COMPARE() \
	if (object.OtherModes.antialias_en ? (!userdata->CurrentPixCvg) : (!userdata->CurrentCvgBit))	\
	{																\
		return false;												\
	}

#define TEST_REJECT() \
	ALPHA_COMPARE() \
	CVG_COMPARE()

#define WRITE_OUT_NB_ND(cycle) \
	*fr = *userdata->ColorInputs.blender1a_r[cycle];	\
	*fg = *userdata->ColorInputs.blender1a_g[cycle];	\
	*fb = *userdata->ColorInputs.blender1a_b[cycle];

#define WRITE_OUT() \
	*fr = r;	\
	*fg = g;	\
	*fb = b;

#define WRITE_BLENDED_COLOR() \
	userdata->BlendedPixelColor.i.r = r;	\
	userdata->BlendedPixelColor.i.g = g;	\
	userdata->BlendedPixelColor.i.b = b;	\
	userdata->BlendedPixelColor.i.a = userdata->PixelColor.i.a;

#define BLEND_CYCLE(cyc) \
	if (partialreject && userdata->PixelColor.i.a >= 0xff)	\
	{														\
		ASSIGN_OUT(cyc);									\
	}														\
	else													\
	{														\
		userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[cyc];	\
		((this)->*(cycle##cyc[((object.OtherModes.force_blend & 1) << 1) | (special_bsel##cyc & 1)]))(&r, &g, &b, userdata, object);	\
	}

#define BLEND_FACTORS(cycle) \
	UINT8 blend1a = *userdata->ColorInputs.blender1b_a[cycle] >> 3;	\
	UINT8 blend2a = *userdata->ColorInputs.blender2b_a[cycle] >> 3;

#define BLEND_FACTORS_SUM(cycle) \
	UINT8 blend1a = *userdata->ColorInputs.blender1b_a[cycle] >> 3;	\
	UINT8 blend2a = *userdata->ColorInputs.blender2b_a[cycle] >> 3;	\
	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

#define BLEND_FACTORS_SPECIAL(cycle) \
	UINT8 blend1a = (*userdata->ColorInputs.blender1b_a[cycle] >> (3 + userdata->ShiftA)) & 0x1c;	\
	UINT8 blend2a = (*userdata->ColorInputs.blender2b_a[cycle] >> (3 + userdata->ShiftB)) & 0x1c;

#define BLEND_FACTORS_SPECIAL_SUM(cycle) \
	UINT8 blend1a = (*userdata->ColorInputs.blender1b_a[cycle] >> (3 + userdata->ShiftA)) & 0x1c;	\
	UINT8 blend2a = (*userdata->ColorInputs.blender2b_a[cycle] >> (3 + userdata->ShiftB)) & 0x1c;	\
	UINT32 sum = ((blend1a >> 2) + (blend2a >> 2) + 1) & 0xf;

#define BLEND_MUL(cycle) \
	*r = (((int)(*userdata->ColorInputs.blender1a_r[cycle]) * (int)(blend1a))) +	\
		(((int)(*userdata->ColorInputs.blender2a_r[cycle]) * (int)(blend2a)));		\
	*g = (((int)(*userdata->ColorInputs.blender1a_g[cycle]) * (int)(blend1a))) +	\
		(((int)(*userdata->ColorInputs.blender2a_g[cycle]) * (int)(blend2a)));		\
	*b = (((int)(*userdata->ColorInputs.blender1a_b[cycle]) * (int)(blend1a))) +	\
		(((int)(*userdata->ColorInputs.blender2a_b[cycle]) * (int)(blend2a)));

#define BLEND_ADD_SPECIAL(cycle) \
	*r += (((int)*userdata->ColorInputs.blender2a_r[cycle]) << 2);	\
	*g += (((int)*userdata->ColorInputs.blender2a_g[cycle]) << 2);	\
	*b += (((int)*userdata->ColorInputs.blender2a_b[cycle]) << 2);

#define BLEND_ADD(cycle) \
	*r += (int)*userdata->ColorInputs.blender2a_r[cycle];	\
	*g += (int)*userdata->ColorInputs.blender2a_g[cycle];	\
	*b += (int)*userdata->ColorInputs.blender2a_b[cycle];

#define BLEND_SHIFT(shift)	\
	*r >>= shift;			\
	*g >>= shift;			\
	*b >>= shift;

#define BLEND_CLAMP()		\
	if (*r > 255) *r = 255;	\
	if (*g > 255) *g = 255;	\
	if (*b > 255) *b = 255;

#define BLEND_SCALE()			\
	if (sum)					\
	{							\
		*r /= sum;				\
		*g /= sum;				\
		*b /= sum;				\
	}							\
	else						\
	{							\
		*r = *g = *b = 0xff;	\
	}


#define ASSIGN_OUT(cycle) \
	r = *userdata->ColorInputs.blender1a_r[cycle];	\
	g = *userdata->ColorInputs.blender1a_g[cycle];	\
	b = *userdata->ColorInputs.blender1a_b[cycle];

bool N64BlenderT::Blend1CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	WRITE_OUT_NB_ND(0);

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	ASSIGN_OUT(0);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	WRITE_OUT_NB_ND(0);

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	ASSIGN_OUT(0);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	WRITE_OUT_NB_ND(1);

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	ASSIGN_OUT(1);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	WRITE_OUT_NB_ND(1);

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	ASSIGN_OUT(1);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->PixelColor.i.a, adseed);
	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel0, int special_bsel1, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DitherA(&userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	userdata->InvPixelColor.i.a = 0xff - *userdata->ColorInputs.blender1b_a[0];
	((this)->*(cycle0[((object.OtherModes.force_blend & 1) << 1) | (special_bsel0 & 1)]))(&r, &g, &b, userdata, object);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1);
	DitherRGB(&r, &g, &b, dith);
	WRITE_OUT();

	return true;
}

void N64BlenderT::BlendEquationCycle0NoForceNoSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SUM(0);
	BLEND_MUL(0);
	BLEND_ADD(0);
	BLEND_SHIFT(2);
	BLEND_SCALE();
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle0NoForceSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SPECIAL_SUM(0);
	BLEND_MUL(0);
	BLEND_ADD_SPECIAL(0);
	BLEND_SHIFT(2);
	BLEND_SCALE();
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle0ForceNoSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS(0);
	BLEND_MUL(0);
	BLEND_ADD(0);
	BLEND_SHIFT(5);
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle0ForceSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SPECIAL(0);
	BLEND_MUL(0);
	BLEND_ADD_SPECIAL(0);
	BLEND_SHIFT(5);
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle1NoForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SUM(1);
	BLEND_MUL(1);
	BLEND_ADD(1);
	BLEND_SHIFT(2);
	BLEND_SCALE();
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle1NoForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SPECIAL_SUM(1);
	BLEND_MUL(1);
	BLEND_ADD_SPECIAL(1);
	BLEND_SHIFT(2);
	BLEND_SCALE();
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle1ForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS(1);
	BLEND_MUL(1);
	BLEND_ADD(1);
	BLEND_SHIFT(5);
	BLEND_CLAMP();
}

void N64BlenderT::BlendEquationCycle1ForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_FACTORS_SPECIAL(1);
	BLEND_MUL(1);
	BLEND_ADD_SPECIAL(1);
	BLEND_SHIFT(5);
	BLEND_CLAMP();
}

bool N64BlenderT::AlphaCompare(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 threshold;
	if (object.OtherModes.alpha_compare_en)
	{
		threshold = (object.OtherModes.dither_alpha_en) ? m_rdp->GetRandom() : userdata->BlendColor.i.a;
		if (alpha < threshold)
		{
			return false;
		}
		return true;
	}
	return true;
}

void N64BlenderT::DitherA(UINT8 *a, int dith)
{
	INT32 new_a = *a + dith;
	if(new_a & 0x100)
	{
		new_a = 0xff;
	}
	*a = (UINT8)new_a;
}

void N64BlenderT::DitherRGB(INT32 *r, INT32 *g, INT32 *b, int dith)
{
	if ((*r & 7) > dith)
	{
		*r = (*r & 0xf8) + 8;
		if (*r > 247)
		{
			*r = 255;
		}
	}
	if ((*g & 7) > dith)
	{
		*g = (*g & 0xf8) + 8;
		if (*g > 247)
		{
			*g = 255;
		}
	}
	if ((*b & 7) > dith)
	{
		*b = (*b & 0xf8) + 8;
		if (*b > 247)
		{
			*b = 255;
		}
	}
}
