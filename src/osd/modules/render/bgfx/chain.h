// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chain.h - BGFX screen-space post-effect chain
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN__
#define __DRAWBGFX_CHAIN__

#include <string>
#include <vector>
#include <map>

#include "chainentry.h"

class bgfx_slider;
class bgfx_parameter;
class texture_manager;
class target_manager;
class bgfx_target;
class osd_window;

class bgfx_chain
{
public:
	bgfx_chain(std::string name, std::string author, bool transform, target_manager& targets, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries, std::vector<bgfx_target*> target_list, uint32_t screen_index);
	~bgfx_chain();

	void process(chain_manager::screen_prim &prim, int view, int screen, texture_manager& textures, osd_window &window, uint64_t blend = 0L);
	void repopulate_targets();

	// Getters
	std::vector<bgfx_slider*>& sliders() { return m_sliders; }
	std::vector<bgfx_chain_entry*>& entries() { return m_entries; }
	uint32_t applicable_passes();
	bool transform() const { return m_transform; }
	bool has_converter() const { return m_has_converter; }

	void prepend_converter(bgfx_effect *effect, chain_manager &chains);

private:
	std::string                         m_name;
	std::string                         m_author;
	bool                                m_transform;
	target_manager&                     m_targets;
	std::vector<bgfx_slider*>           m_sliders;
	std::vector<bgfx_parameter*>        m_params;
	std::vector<bgfx_chain_entry*>      m_entries;
	std::vector<bgfx_target*>           m_target_list;
	std::vector<std::string>            m_target_names;
	std::map<std::string, bgfx_target*> m_target_map;
	int64_t                             m_current_time;
	uint32_t                            m_screen_index;
	bool                                m_has_converter;
};

#endif // __DRAWBGFX_CHAIN__
