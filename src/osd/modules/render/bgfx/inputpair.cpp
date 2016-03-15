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

#include "inputpair.h"
#include "texture.h"
#include "target.h"
#include "effect.h"
#include "uniform.h"
#include "texturemanager.h"

bgfx_input_pair::bgfx_input_pair(int index, std::string sampler, std::string texture)
	: m_index(index)
	, m_sampler(sampler)
	, m_texture(texture)
{
}

void bgfx_input_pair::bind(bgfx_effect *effect, texture_manager& textures) const
{
	bgfx::setTexture(m_index, effect->uniform(m_sampler)->handle(), textures.handle(m_texture));
}
