// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effect.h - BGFX shader material to be applied to a mesh
//
//============================================================

#ifndef MAME_RENDER_BGFX_EFFECT_H
#define MAME_RENDER_BGFX_EFFECT_H

#pragma once

#include <bgfx/bgfx.h>

#include <map>
#include <memory>
#include <string>
#include <vector>


class bgfx_uniform;

class bgfx_effect
{
public:
	bgfx_effect(std::string &&name, uint64_t state, bgfx::ShaderHandle vertex_shader, bgfx::ShaderHandle fragment_shader, std::vector<std::unique_ptr<bgfx_uniform> > &uniforms);
	~bgfx_effect();

	void submit(int view, uint64_t blend = ~0ULL);
	bgfx_uniform *uniform(const std::string &name);
	bool is_valid() const { return m_program_handle.idx != bgfx::kInvalidHandle; }

private:
	std::string                          m_name;
	uint64_t                             m_state;
	bgfx::ProgramHandle                  m_program_handle;
	std::map<std::string, std::unique_ptr<bgfx_uniform> > m_uniforms;
};

#endif // MAME_RENDER_BGFX_EFFECT_H
