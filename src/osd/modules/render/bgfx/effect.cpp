// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effect.cpp - BGFX shader material to be applied to a mesh
//
//============================================================

#include "effect.h"

#include "uniform.h"

#include "modules/osdmodule.h"
#include "osdcore.h"

#include <utility>


bgfx_effect::bgfx_effect(std::string &&name, uint64_t state, bgfx::ShaderHandle vertex_shader, bgfx::ShaderHandle fragment_shader, std::vector<std::unique_ptr<bgfx_uniform> > &uniforms)
	: m_name(std::move(name))
	, m_state(state)
{
	m_program_handle = bgfx::createProgram(vertex_shader, fragment_shader, false);

	for (auto &uniform : uniforms)
	{
		const auto existing = m_uniforms.find(uniform->name());
		if (existing != m_uniforms.end())
		{
			osd_printf_verbose("Uniform %s appears to be duplicated in one or more effects, please double-check the effect JSON files.\n", uniform->name());
			uniform.reset();
			continue;
		}
		uniform->create();
		m_uniforms.emplace(uniform->name(), std::move(uniform));
	}
}

bgfx_effect::~bgfx_effect()
{
	m_uniforms.clear();
	bgfx::destroy(m_program_handle);
}

void bgfx_effect::submit(int view, uint64_t blend)
{
	for (auto &[name, uniform] : m_uniforms)
	{
		uniform->upload();
	}

	const uint64_t final_state = (blend != ~0ULL) ? ((m_state & ~BGFX_STATE_BLEND_MASK) | blend) : m_state;

	bgfx::setState(final_state);
	bgfx::submit(view, m_program_handle);
}

bgfx_uniform* bgfx_effect::uniform(const std::string &name)
{
	const auto iter = m_uniforms.find(name);
	return (iter != m_uniforms.end()) ? iter->second.get() : nullptr;
}
