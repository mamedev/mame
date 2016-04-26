// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  inputpair.h - BGFX sampler-and-texture pair
//
//  Keeps track of the texture which is bound to the sampler
//  which is bound to the specified stage index.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_INPUT_PAIR__
#define __DRAWBGFX_INPUT_PAIR__

#include <string>

#include "ui/uimain.h"

class bgfx_effect;
class chain_manager;

class bgfx_input_pair
{
public:
	bgfx_input_pair(int index, std::string sampler, std::string texture, std::vector<std::string> available_textures, std::string selection, chain_manager& chains, uint32_t screen_index);

	void bind(bgfx_effect *effect, const int32_t screen) const;
	int32_t texture_changed(int32_t index, std::string *str, int32_t newval);

	// Getters
	chain_manager& chains() const { return m_chains; }
	std::string sampler() const { return m_sampler; }
	std::string texture() const { return m_texture; }
	std::vector<ui_menu_item> get_slider_list();

private:
	void create_selection_slider(uint32_t screen_index);
	bool needs_sliders();

	int                       m_index;
	std::string               m_sampler;
	std::string               m_texture;
	std::vector<std::string>  m_available_textures;
	std::string               m_selection;
	chain_manager&            m_chains;
	int32_t                   m_current_texture;
	ui_menu_item              m_selection_slider;
};

#endif // __DRAWBGFX_INPUT_PAIR__
