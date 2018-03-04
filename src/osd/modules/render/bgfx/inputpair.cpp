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

#include "emu.h"
#include "../frontend/mame/ui/slider.h"

#include "emucore.h"

#include "inputpair.h"
#include "texture.h"
#include "target.h"
#include "effect.h"
#include "render.h"
#include "uniform.h"
#include "chainmanager.h"
#include "slider.h"
#include "sliderdirtynotifier.h"

bgfx_input_pair::bgfx_input_pair(int index, std::string sampler, std::string texture, std::vector<std::string> available_textures, std::string selection, chain_manager& chains, uint32_t screen_index)
	: m_index(index)
	, m_sampler(sampler)
	, m_texture(texture)
	, m_available_textures(available_textures)
	, m_selection(selection)
	, m_chains(chains)
{
	if (m_available_textures.size() > 0)
	{
		m_current_texture = std::find(m_available_textures.begin(), m_available_textures.end(), m_texture) - m_available_textures.begin();

		create_selection_slider(screen_index);
	}
}

bgfx_input_pair::~bgfx_input_pair()
{
}

void bgfx_input_pair::bind(bgfx_effect *effect, const int32_t screen) const
{
	assert(effect->uniform(m_sampler) != nullptr);
	std::string name = m_texture + std::to_string(screen);

	bgfx_texture_handle_provider* provider = chains().textures().provider(name);
	bgfx_uniform *tex_size = effect->uniform("u_tex_size" + std::to_string(m_index));
	if (tex_size != nullptr)
	{
		float values[2] = { float(provider->width()), float(provider->height()) };
		tex_size->set(values, sizeof(float) * 2);
	}

	bgfx::setTexture(m_index, effect->uniform(m_sampler)->handle(), chains().textures().handle(name));
}

int32_t bgfx_input_pair::slider_changed(running_machine &machine, void *arg, int id, std::string *str, int32_t newval)
{
	return texture_changed(id, str, newval);
}

int32_t bgfx_input_pair::texture_changed(int32_t id, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
	{
		m_current_texture = newval;
		m_texture = m_available_textures[m_current_texture];

		chains().slider_notifier().set_sliders_dirty();
	}

	if (str != nullptr)
	{
		std::string file = m_texture;
		const size_t last_slash = m_texture.rfind('/');
		if (last_slash != std::string::npos)
		{
			file = m_texture.substr(last_slash + 1, m_texture.length() - (last_slash + 1));
		}

		std::string file_name;
		const size_t last_dot = file.rfind('.');
		if (last_dot != std::string::npos)
		{
			file_name = file.substr(0, last_dot);
		}

		*str = string_format("%s", file_name.c_str());
	}

	return m_current_texture;
}

void bgfx_input_pair::create_selection_slider(uint32_t screen_index)
{
	m_slider_state = make_unique_clear<slider_state>();

	m_slider_state->minval = 0;
	m_slider_state->defval = m_current_texture;
	m_slider_state->maxval = m_available_textures.size() - 1;
	m_slider_state->incval = 1;

	using namespace std::placeholders;
	m_slider_state->update = std::bind(&bgfx_input_pair::slider_changed, this, _1, _2, _3, _4, _5);
	m_slider_state->arg = this;
	m_slider_state->id = screen_index;
	m_slider_state->description = "Window " + std::to_string(chains().window_index()) + ", Screen " + std::to_string(screen_index) + " " + m_selection + ":";

	ui::menu_item item;
	item.text = m_slider_state->description;
	item.subtext = "";
	item.flags = 0;
	item.ref = m_slider_state.get();
	item.type = ui::menu_item_type::SLIDER;
	m_selection_slider = item;
}

bool bgfx_input_pair::needs_sliders()
{
	return chains().screen_count() > 0 && m_available_textures.size() > 1;
}

std::vector<ui::menu_item> bgfx_input_pair::get_slider_list()
{
	std::vector<ui::menu_item> sliders;

	if (!needs_sliders())
	{
		return sliders;
	}

	sliders.push_back(m_selection_slider);

	return sliders;
}
