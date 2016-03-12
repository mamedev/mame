// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chain.cpp - BGFX screen-space post-effect chain
//
//============================================================

#include "slider.h"
#include "parameter.h"
#include "chainentry.h"

#include "chain.h"

bgfx_chain::bgfx_chain(std::string name, std::string author, std::vector<bgfx_slider*> sliders, std::vector<bgfx_parameter*> params, std::vector<bgfx_chain_entry*> entries)
	: m_name(name)
	, m_author(author)
	, m_sliders(sliders)
	, m_params(params)
	, m_entries(entries)
{
}

bgfx_chain::~bgfx_chain()
{
	for (bgfx_slider* slider : m_sliders)
	{
		delete slider;
	}
	for (bgfx_parameter* param : m_params)
	{
		delete param;
	}
	for (bgfx_chain_entry* entry : m_entries)
	{
		delete entry;
	}
}

void bgfx_chain::submit(render_primitive* prim, int view)
{
	//for (bgfx_chain_entry* entry : m_entries)
	//{
	//}
}
