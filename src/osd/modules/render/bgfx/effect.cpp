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

bgfx_effect::bgfx_effect(uint64_t state, bgfx::ShaderHandle vertex_shader, bgfx::ShaderHandle fragment_shader, std::vector<bgfx_uniform*> uniforms)
	: m_state(state)
{
	m_program_handle = bgfx::createProgram(vertex_shader, fragment_shader, false);

	for (int i = 0; i < uniforms.size(); i++)
	{
		if (m_uniforms[uniforms[i]->name()] != nullptr)
		{
			osd_printf_verbose("Uniform %s appears to be duplicated in one or more effects, please double-check the effect JSON files.\n", uniforms[i]->name().c_str());
			delete uniforms[i];
			continue;
		}
		m_uniforms[uniforms[i]->name()] = uniforms[i];
	}
}

bgfx_effect::~bgfx_effect()
{
	for (std::pair<std::string, bgfx_uniform*> uniform : m_uniforms)
	{
		delete uniform.second;
	}
	m_uniforms.clear();
	bgfx::destroyProgram(m_program_handle);
}

void bgfx_effect::submit(int view, uint64_t blend)
{
	for (std::pair<std::string, bgfx_uniform*> uniform_pair : m_uniforms)
	{
		(uniform_pair.second)->upload();
	}
	bgfx::setState(m_state | blend);
	bgfx::submit(view, m_program_handle);
}

bgfx_uniform* bgfx_effect::uniform(std::string name)
{
	std::map<std::string, bgfx_uniform*>::iterator iter = m_uniforms.find(name);

	if (iter != m_uniforms.end())
	{
		return iter->second;
	}

	return nullptr;
}
