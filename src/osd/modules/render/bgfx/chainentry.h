// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentry.h - BGFX shader post-processing node
//
//  Represents a single entry in a list of post-processing
//  passes to be applied to a screen quad or buffer.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_ENTRY__
#define __DRAWBGFX_CHAIN_ENTRY__

#include <string>
#include <vector>

#include "inputpair.h"

class render_primitive;
class bgfx_effect;
class bgfx_texture;
class bgfx_target;

class bgfx_chain_entry
{
public:
	bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_input_pair>& inputs, bgfx_target* output);
	~bgfx_chain_entry();

	void submit(render_primitive* prim, int view);

	// Getters
	std::string name() const { return m_name; }

private:
	std::string                     m_name;
	bgfx_effect*                    m_effect;
	std::vector<bgfx_input_pair>    m_inputs;
	bgfx_target*                    m_output;
};

#endif // __DRAWBGFX_CHAIN_ENTRY__
