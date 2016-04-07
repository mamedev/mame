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

class render_primitive;
class bgfx_slider;
class bgfx_parameter;
class texture_manager;
class osd_window;

class bgfx_chain
{
public:
	bgfx_chain(std::string name, std::string author, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries);
	~bgfx_chain();

	void process(render_primitive* prim, int view, int screen, texture_manager& textures, osd_window &window, uint64_t blend = 0L);

	// Getters
	std::vector<bgfx_slider*>& sliders() { return m_sliders; }
	uint32_t applicable_passes();

private:
	std::string                         m_name;
	std::string                         m_author;
	std::vector<bgfx_slider*>           m_sliders;
	std::vector<bgfx_parameter*>        m_params;
	std::vector<bgfx_chain_entry*>      m_entries;
	std::map<std::string, bgfx_slider*> m_slider_map;
	int64_t                             m_current_time;
};

#endif // __DRAWBGFX_CHAIN__
