// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by Ryan Holtz
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"
#include "video/n64.h"

class n64_blender_t
{
	public:
		typedef bool (n64_blender_t::*blender1)(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		typedef bool (n64_blender_t::*blender2)(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);

		n64_blender_t();

		blender1            blend1[8];
		blender2            blend2[8];

		void                set_machine(running_machine& machine) { m_machine = &machine; }
		void                set_processor(n64_rdp* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }

	private:
		running_machine*    m_machine;
		n64_rdp*            m_rdp;

		INT32 min(const INT32 x, const INT32 min);
		bool alpha_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		bool test_for_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_pipe(const int cycle, const int special, color_t& out, rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_with_partial_reject(color_t& out, INT32 cycle, INT32 partialreject, INT32 select, rdp_span_aux* userdata, const rdp_poly_state& object);

		bool cycle1_noblend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_noblend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle1_blend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);

		bool cycle2_noblend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_noblend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_noacvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_noacvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_acvg_nodither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);
		bool cycle2_blend_acvg_dither(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);

		INT32 dither_alpha(INT32 alpha, INT32 dither);
		INT32 dither_color(INT32 color, INT32 dither);

		UINT8               m_color_dither[256 * 8];
		UINT8               m_alpha_dither[256 * 8];
};

#endif // _VIDEO_RDPBLEND_H_
