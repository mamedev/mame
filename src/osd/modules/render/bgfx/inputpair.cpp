// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  inputpair.cpp - BGFX sampler-and-texture pair
//
//  Keeps track of the texture which is bound to the sampler
//  which is bound to the specified stage index.
//
//============================================================

#include "emucore.h"

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
	std::string name = m_texture + std::to_string(screen);

	bgfx_texture_handle_provider* provider = textures.provider(name);
	bgfx_uniform *tex_size = effect->uniform("u_tex_size" + std::to_string(m_index));
	if (tex_size != nullptr)
	{
		float values[2] = { float(provider->width()), float(provider->height()) };
		tex_size->set(values, sizeof(float) * 2);
	}

	bgfx::setTexture(m_index, effect->uniform(m_sampler)->handle(), textures.handle(name));
}
