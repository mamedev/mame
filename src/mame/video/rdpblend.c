/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"
#include "video/rdpbhelp.h"

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

	compare[0] = &N64BlenderT::AlphaCompareNone;
	compare[1] = &N64BlenderT::AlphaCompareNone;
	compare[2] = &N64BlenderT::AlphaCompareNoDither;
	compare[3] = &N64BlenderT::AlphaCompareDither;
}

bool N64BlenderT::Blend1CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);
	TEST_REJECT();
	WRITE_OUT_NB_ND(0);

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);
	TEST_REJECT();
	ASSIGN_OUT(0);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	WRITE_OUT_NB_ND(0);

	return true;
}

bool N64BlenderT::Blend1CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	ASSIGN_OUT(0);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend1CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	WRITE_OUT_NB_ND(1);

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	ASSIGN_OUT(1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	WRITE_OUT_NB_ND(1);

	return true;
}

bool N64BlenderT::Blend2CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	ASSIGN_OUT(1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1, 1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->PixelColor.i.a, adseed);
	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1, 1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1, 1);
	WRITE_OUT();

	return true;
}

bool N64BlenderT::Blend2CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	INT32 r, g, b;

	DITHER_A(userdata->ShadeColor.i.a, adseed);

	TEST_REJECT();
	BLEND_CYCLE(0, 0);
	WRITE_BLENDED_COLOR();
	BLEND_CYCLE(1, 1);
	DITHER_RGB(dith);
	WRITE_OUT();

	return true;
}

#define BLEND_PIPE(cycle, special, sum, shift)  \
	BLEND_FACTORS(cycle, special, sum);         \
	BLEND_MUL(cycle);                           \
	BLEND_ADD(cycle, special);                  \
	BLEND_SHIFT(shift);                         \
	BLEND_SCALE_CLAMP(sum);

void N64BlenderT::BlendEquationCycle0NoForceNoSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(0, 0, 1, 2);
}

void N64BlenderT::BlendEquationCycle0NoForceSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(0, 1, 1, 2);
}

void N64BlenderT::BlendEquationCycle0ForceNoSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(0, 0, 0, 5);
}

void N64BlenderT::BlendEquationCycle0ForceSpecial(int* r, int* g, int* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(0, 1, 0, 5);
}

void N64BlenderT::BlendEquationCycle1NoForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(1, 0, 1, 2);
}

void N64BlenderT::BlendEquationCycle1NoForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(1, 1, 1, 2);
}

void N64BlenderT::BlendEquationCycle1ForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(1, 0, 0, 5);
}

void N64BlenderT::BlendEquationCycle1ForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object)
{
	BLEND_PIPE(1, 1, 0, 5);
}

bool N64BlenderT::AlphaCompareNone(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object)
{
	return false;
}

bool N64BlenderT::AlphaCompareNoDither(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object)
{
	return alpha < userdata->BlendColor.i.a;
}

bool N64BlenderT::AlphaCompareDither(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object)
{
	return alpha < (rand() & 0xff);
}
