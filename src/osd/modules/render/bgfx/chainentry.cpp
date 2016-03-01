// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentry.cpp - BGFX shader post-processing node
//
//  Represents a single entry in a list of post-processing
//  passes to be applied to a screen quad or buffer.
//
//============================================================

#include "emu.h"

#include <bgfx/bgfx.h>

#include "chainentry.h"

#include "effect.h"
#include "texture.h"
#include "target.h"
#include "entryuniform.h"
#include "texturemanager.h"

bgfx_chain_entry::bgfx_chain_entry(std::string name, bgfx_effect* effect, std::vector<bgfx_input_pair>& inputs, std::vector<bgfx_entry_uniform*> uniforms, bgfx_target* output)
	: m_name(name)
	, m_effect(effect)
	, m_output(output)
{
	for (bgfx_input_pair input : inputs)
	{
		m_inputs.push_back(input);
	}
	for (bgfx_entry_uniform* uniform : uniforms)
	{
		m_uniforms.push_back(uniform);
	}
}

bgfx_chain_entry::~bgfx_chain_entry()
{
	for (bgfx_entry_uniform* uniform : m_uniforms)
	{
		delete uniform;
	}
	m_uniforms.clear();
}

void bgfx_chain_entry::submit(render_primitive* prim, int view, texture_manager& textures, uint16_t screen_width, uint16_t screen_height, uint64_t blend)
{
	for (bgfx_input_pair input : m_inputs)
	{
		input.bind(m_effect, textures);
	}
    if (m_output != nullptr)
    {
		printf("Setting view to %s, %dx%d\n", m_output->name().c_str(), m_outptu->width, m_output->height());
        bgfx::setViewFrameBuffer(view, m_output->target());
        bgfx::setViewRect(view, 0, 0, m_output->width(), m_output->height());
    }
    else
    {
		printf("Setting view to backbuffer, %dx%d\n", screen_width, screen_height);
        bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
        bgfx::setViewRect(view, 0, 0, screen_width, screen_height);
    }
    for (bgfx_entry_uniform* uniform : m_uniforms)
    {
		uniform->bind();
	}
	m_effect->submit(view, blend);
}
