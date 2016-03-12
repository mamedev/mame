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

class bgfx_slider;
class bgfx_parameter;
class bgfx_chain_entry;
class render_primitive;

class bgfx_chain
{
public:
	bgfx_chain(std::string name, std::string author, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries);
	~bgfx_chain();

	void submit(render_primitive* prim, int view);

private:
	std::string                     m_name;
	std::string                     m_author;
	std::vector<bgfx_slider*>       m_sliders;
	std::vector<bgfx_parameter*>    m_params;
	std::vector<bgfx_chain_entry*>  m_entries;
};

#endif // __DRAWBGFX_CHAIN__
