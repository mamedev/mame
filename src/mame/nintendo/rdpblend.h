// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    SGI/Nintendo Reality Display Processor Blend Unit (BL)
    -------------------

    by Ryan Holtz
    based on initial C code by Ville Linde
    contains additional improvements from angrylion, Ziggy, Gonetz and Orkin


******************************************************************************/

#ifndef MAME_NINTENDO_RDPBLEND_H
#define MAME_NINTENDO_RDPBLEND_H

#pragma once

#include "n64_v.h"

class n64_blender_t
{
	public:
		typedef bool (n64_blender_t::*blender1)(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, rdp_span_aux* userdata, const rdp_poly_state& object);
		typedef bool (n64_blender_t::*blender2)(color_t& blended_pixel, int dith, int adseed, int partialreject, int sel0, int sel1, rdp_span_aux* userdata, const rdp_poly_state& object);

		n64_blender_t(running_machine &machine, n64_rdp &rdp);

		blender1            blend1[8];
		blender2            blend2[8];

		running_machine &machine() const { return m_machine; }

	private:
		running_machine&    m_machine;

		int32_t min(const int32_t x, const int32_t min);
		bool alpha_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		bool test_for_reject(rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_pipe(const int cycle, const int special, color_t& out, rdp_span_aux* userdata, const rdp_poly_state& object);
		void blend_with_partial_reject(color_t& out, int32_t cycle, int32_t partialreject, int32_t select, rdp_span_aux* userdata, const rdp_poly_state& object);

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

		int32_t dither_alpha(int32_t alpha, int32_t dither);
		int32_t dither_color(int32_t color, int32_t dither);

		uint8_t               m_color_dither[256 * 8];
		uint8_t               m_alpha_dither[256 * 8];
};

#endif // MAME_NINTENDO_RDPBLEND_H
