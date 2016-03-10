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

class bgfx_chain
{
public:
	bgfx_chain(std::string name, std::string author, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries, std::string output);
	~bgfx_chain();

	void process(render_primitive* prim, int view, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint64_t blend = 0L);

    // Getters
    std::vector<bgfx_slider*>& sliders() { return m_sliders; }
    std::string output() const { return m_output; }
    uint32_t applicable_passes();

private:
	std::string                         m_name;
	std::string                         m_author;
    std::vector<bgfx_slider*>           m_sliders;
    std::vector<bgfx_parameter*>        m_params;
	std::vector<bgfx_chain_entry*>      m_entries;
    std::map<std::string, bgfx_slider*> m_slider_map;
    std::string                         m_output;
};

#endif // __DRAWBGFX_CHAIN__
