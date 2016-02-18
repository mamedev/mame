#pragma once

#ifndef __DRAWBGFX_EFFECT__
#define __DRAWBGFX_EFFECT__

#include <bgfx/bgfx.h>

#include <string>
#include <vector>
#include <map>

#include "uniform.h"

class bgfx_effect
{
public:
    bgfx_effect(uint64_t state, bgfx::ShaderHandle vertexShader, bgfx::ShaderHandle fragmentShader, std::vector<bgfx_uniform*> uniforms);
    ~bgfx_effect();

    void submit(int view);

    // Getters
    bgfx_uniform* uniform(std::string name);
    bgfx::ProgramHandle get_program() const { return m_program_handle; }

private:
    uint64_t								m_state;
    bgfx::ProgramHandle						m_program_handle;
    std::map<std::string, bgfx_uniform*>	m_uniforms;
};

#endif // __DRAWBGFX_EFFECT__