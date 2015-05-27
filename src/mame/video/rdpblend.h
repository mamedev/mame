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

struct other_modes_t;
struct misc_state_t;
class n64_rdp;
struct rdp_span_aux;
class color_t;
struct rdp_poly_state;

class n64_blender_t
{
	public:
		typedef bool (n64_blender_t::*blender1)(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		typedef bool (n64_blender_t::*blender2)(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);

		n64_blender_t();

		blender1            blend1[8];
		blender2            blend2[8];

		void                set_machine(running_machine& machine) { m_machine = &machine; }
		void                set_processor(n64_rdp* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	private:
		running_machine*    m_machine;
		n64_rdp*            m_rdp;

		INT32 min(const INT32 x, const INT32 min);
		bool alpha_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		bool test_for_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_pipe(const int cycle, const int special, int* r_out, int* g_out, int* b_out, rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_with_partial_reject(INT32* r, INT32* g, INT32* b, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux* userdata, const rdp_poly_state& object);

		bool cycle1_noblend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);

		bool cycle2_noblend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_noacvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_noacvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_acvg_nodither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_acvg_dither(INT32* fr, INT32* fg, INT32* fb, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);

		INT32 dither_alpha(INT32 alpha, INT32 dither);
		INT32 dither_color(INT32 color, INT32 dither);

		UINT8               m_color_dither[256 * 8];
		UINT8               m_alpha_dither[256 * 8];
};

#endif // _VIDEO_RDPBLEND_H_
