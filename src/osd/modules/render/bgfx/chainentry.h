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

#include <bgfx/bgfx.h>

#include <string>
#include <vector>

#include "inputpair.h"

class render_primitive;
class bgfx_effect;
class bgfx_target;
class bgfx_entry_uniform;
class texture_manager;

class bgfx_chain_entry
{
public:
	bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_input_pair>& inputs, std::vector<bgfx_entry_uniform*> uniforms, bgfx_target* output);
	~bgfx_chain_entry();

	void submit(render_primitive* prim, int view, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint64_t blend);

	// Getters
	std::string name() const { return m_name; }

private:
	void setup_view(int view, uint16_t screen_width, uint16_t screen_height);
	void put_screen_buffer(render_primitive* prim, bgfx::TransientVertexBuffer* buffer);

	std::string                     	m_name;
	bgfx_effect*                    	m_effect;
	std::vector<bgfx_input_pair>    	m_inputs;
	std::vector<bgfx_entry_uniform*>	m_uniforms;
	bgfx_target*                    	m_output;
};

#endif // __DRAWBGFX_CHAIN_ENTRY__
