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
#include "targetmanager.h"

class render_primitive;
class bgfx_effect;
class bgfx_target;
class bgfx_entry_uniform;
class bgfx_suppressor;
class texture_manager;
class target_manager;

class bgfx_chain_entry
{
public:
	bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_suppressor*> suppressors, std::vector<bgfx_input_pair> inputs, std::vector<bgfx_entry_uniform*> uniforms, target_manager& targets, std::string output);
	~bgfx_chain_entry();

	void submit(int view, render_primitive* prim, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint32_t rotation_type, bool swap_xy, uint64_t blend, int32_t screen);

	// Getters
	std::string name() const { return m_name; }
	bool skip();

private:
	void setup_auto_uniforms(render_primitive* prim, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint32_t rotation_type, bool swap_xy, int32_t screen);
	void setup_screensize_uniforms(texture_manager& textures, uint16_t screen_width, uint16_t screen_height, int32_t screen);
	void setup_sourcesize_uniform(render_primitive* prim) const;
	void setup_rotationtype_uniform(uint32_t rotation_type) const;
	void setup_swapxy_uniform(bool swap_xy) const;
	void setup_quaddims_uniform(render_primitive* prim) const;
	void setup_screenindex_uniform(int32_t screen) const;

	bool setup_view(int view, uint16_t screen_width, uint16_t screen_height, int32_t screen) const;
	void put_screen_buffer(render_primitive* prim, bgfx::TransientVertexBuffer* buffer) const;

	std::string                         m_name;
	bgfx_effect*                        m_effect;
	std::vector<bgfx_suppressor*>       m_suppressors;
	std::vector<bgfx_input_pair>        m_inputs;
	std::vector<bgfx_entry_uniform*>    m_uniforms;
	target_manager&                     m_targets;
	std::string                         m_output;
};

#endif // __DRAWBGFX_CHAIN_ENTRY__
