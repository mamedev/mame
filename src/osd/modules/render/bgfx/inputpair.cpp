// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  inputpair.h - BGFX sampler-and-texture pair
//
//  Keeps track of the texture which is bound to the sampler
//  which is bound to the specified stage index.
//
//============================================================

#include "emu.h"

#include "inputpair.h"
#include "texture.h"
#include "target.h"
#include "effect.h"
#include "uniform.h"
#include "texturemanager.h"
#include "targetmanager.h"

bgfx_input_pair::bgfx_input_pair(int index, std::string sampler, std::string texture)
	: m_index(index)
	, m_sampler(sampler)
	, m_texture(texture)
{
}

void bgfx_input_pair::bind(bgfx_effect *effect, target_manager& targets, texture_manager& textures, const int32_t screen) const
{
    assert(effect->uniform(m_sampler) != nullptr);
    std::string texture = m_texture;
    if (targets.target(m_texture + std::to_string(screen)) != nullptr)
    {
        texture = m_texture + std::to_string(screen);
    }
	bgfx::setTexture(m_index, effect->uniform(m_sampler)->handle(), textures.handle(texture));
}
