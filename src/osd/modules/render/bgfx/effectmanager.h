#pragma once

#ifndef __DRAWBGFX_EFFECT_MANAGER__
#define __DRAWBGFX_EFFECT_MANAGER__

#include <map>
#include <string>

#include <bgfx/bgfx.h>

#include "shadermanager.h"

class bgfx_effect;

class effect_manager {
public:
    effect_manager(shader_manager& shaders) : m_shaders(shaders) { }
    ~effect_manager();

    // Getters
    bgfx_effect* effect(std::string name);

private:
    bgfx_effect* load_effect(std::string name);

	shader_manager&						m_shaders;
    std::map<std::string, bgfx_effect*> m_effects;
};

#endif // __DRAWBGFX_EFFECT_MANAGER__