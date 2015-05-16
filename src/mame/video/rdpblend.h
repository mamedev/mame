// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
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
		typedef bool (N64BlenderT::*Blender1)(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		typedef bool (N64BlenderT::*Blender2)(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);

		N64BlenderT();

		Blender1            blend1[8];
		Blender2            blend2[8];

		void                SetMachine(running_machine& machine) { m_machine = &machine; }
		void                SetProcessor(n64_rdp* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

		INT32 min(const INT32 x, const INT32 min);

	private:
		running_machine*    m_machine;
		n64_rdp*            m_rdp;

		bool alpha_reject(rdp_span_aux *userdata, const rdp_poly_state& object);
		bool test_for_reject(rdp_span_aux *userdata, const rdp_poly_state& object);
		void blend_pipe(const int cycle, const int special, int* r_out, int* g_out, int* b_out, rdp_span_aux *userdata, const rdp_poly_state& object);
		void blend_with_partial_reject(INT32* r, INT32* g, INT32* b, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux *userdata, const rdp_poly_state& object);

		bool Blend1CycleNoBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleNoBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleNoBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleNoBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend1CycleBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux *userdata, const rdp_poly_state& object);

		bool Blend2CycleNoBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleNoBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleNoBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleNoBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleBlendNoACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleBlendNoACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleBlendACVGNoDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool Blend2CycleBlendACVGDither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux *userdata, const rdp_poly_state& object);

		INT32 dither_alpha(INT32 alpha, INT32 dither);
		INT32 dither_color(INT32 color, INT32 dither);

		UINT8				m_color_dither[256 * 8];
		UINT8				m_alpha_dither[256 * 8];
};

#endif // _VIDEO_RDPBLEND_H_
