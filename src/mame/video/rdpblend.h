/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by MooglyGuy
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"

struct OtherModesT;
struct MiscStateT;
class n64_rdp;
struct rdp_span_aux;
class Color;
struct rdp_poly_state;

class N64BlenderT
{
	public:
		typedef bool (N64BlenderT::*Blender1)(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		typedef bool (N64BlenderT::*Blender2)(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		typedef void (N64BlenderT::*BlendEquation)(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		typedef bool (N64BlenderT::*AlphaCompare)(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object);

		N64BlenderT();

		Blender1            blend1[8];
		Blender2            blend2[8];

		void                SetMachine(running_machine& machine) { m_machine = &machine; }
		void                SetProcessor(n64_rdp* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	private:
		running_machine*    m_machine;
		n64_rdp*            m_rdp;

		bool                Blend1CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend1CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);

		bool                Blend2CycleNoBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleNoBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleNoBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleNoBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleBlendNoACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleBlendNoACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleBlendACVGNoDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                Blend2CycleBlendACVGDither(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, int acmode, rdp_span_aux *userdata, const rdp_poly_state& object);

		void                BlendEquationCycle0NoForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle0NoForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle0ForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle0ForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);

		void                BlendEquationCycle1NoForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle1NoForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle1ForceNoSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                BlendEquationCycle1ForceSpecial(INT32* r, INT32* g, INT32* b, rdp_span_aux *userdata, const rdp_poly_state& object);

		BlendEquation       cycle0[4];
		BlendEquation       cycle1[4];
		AlphaCompare        compare[4];

		bool                AlphaCompareNone(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                AlphaCompareNoDither(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object);
		bool                AlphaCompareDither(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object);

		void                DitherRGB(INT32* r, INT32* g, INT32* b, int dith);
		void                DitherA(UINT8* a, int dith);
};

#endif // _VIDEO_RDPBLEND_H_
