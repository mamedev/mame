// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainmanager.cpp - BGFX shader chain manager
//
//  Provides loading for BGFX shader effect chains, defined
//  by chain.h and read by chainreader.h
//
//============================================================

#include <bx/readerwriter.h>
#include <bx/file.h>

#include "emu.h"
#include "../frontend/mame/ui/slider.h"

#include "osdcore.h"
#include "modules/osdwindow.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "bgfxutil.h"

#include "chainmanager.h"
#include "chainreader.h"
#include "chain.h"

#include "texture.h"
#include "target.h"
#include "slider.h"

#include "sliderdirtynotifier.h"

using namespace rapidjson;

const uint32_t chain_manager::CHAIN_NONE = 0;

chain_manager::chain_manager(running_machine& machine, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t window_index, slider_dirty_notifier& slider_notifier)
	: m_machine(machine)
	, m_options(options)
	, m_textures(textures)
	, m_targets(targets)
	, m_effects(effects)
	, m_window_index(window_index)
	, m_slider_notifier(slider_notifier)
	, m_screen_count(0)
{
	refresh_available_chains();
	parse_chain_selections(options.bgfx_screen_chains());
	init_texture_converters();
}

chain_manager::~chain_manager()
{
	destroy_chains();
}

void chain_manager::init_texture_converters()
{
	m_converters.push_back(nullptr);
	m_converters.push_back(m_effects.effect("misc/texconv_palette16"));
	m_converters.push_back(m_effects.effect("misc/texconv_rgb32"));
	m_converters.push_back(nullptr);
	m_converters.push_back(m_effects.effect("misc/texconv_yuy16"));
}

void chain_manager::refresh_available_chains()
{
	m_available_chains.clear();
	m_available_chains.push_back(chain_desc("none", ""));

	std::string chains_path;
	osd_subst_env(chains_path, util::string_format("%s" PATH_SEPARATOR "chains", m_options.bgfx_path()));
	find_available_chains(chains_path, "");

	destroy_unloaded_chains();
}

void chain_manager::destroy_unloaded_chains()
{
	// O(shaders*available_chains), but we don't care because asset reloading happens rarely
	for (int i = 0; i < m_chain_names.size(); i++)
	{
		std::string name = m_chain_names[i];
		if (name.length() > 0)
		{
			for (chain_desc desc : m_available_chains)
			{
				if (desc.m_name == name)
				{
					delete m_screen_chains[i];
					m_screen_chains[i] = nullptr;
					m_chain_names[i] = "";
					m_current_chain[i] = CHAIN_NONE;
					break;
				}
			}
		}
	}
}

void chain_manager::find_available_chains(std::string root, std::string path)
{
	osd::directory::ptr directory = osd::directory::open(root + path);
	if (directory != nullptr)
	{
		for (const osd::directory::entry *entry = directory->read(); entry != nullptr; entry = directory->read())
		{
			if (entry->type == osd::directory::entry::entry_type::FILE)
			{
				std::string name(entry->name);
				std::string extension(".json");

				// Does the name has at least one character in addition to ".json"?
				if (name.length() > extension.length())
				{
					size_t start = name.length() - extension.length();
					std::string test_segment = name.substr(start, extension.length());

					// Does it end in .json?
					if (test_segment == extension)
					{
						m_available_chains.push_back(chain_desc(name.substr(0, start), path));
					}
				}
			}
			else if (entry->type == osd::directory::entry::entry_type::DIR)
			{
				std::string name = entry->name;
				if (!(name == "." || name == ".."))
				{
					std::string appended_path = path + "/" + name;
					if (path.length() == 0)
					{
						appended_path = name;
					}
					find_available_chains(root, path + "/" + name);
				}
			}
		}
	}
}

bgfx_chain* chain_manager::load_chain(std::string name, uint32_t screen_index)
{
	if (name.length() < 5 || (name.compare(name.length() - 5, 5, ".json") != 0))
	{
		name = name + ".json";
	}
	std::string path;
	osd_subst_env(path, util::string_format("%s" PATH_SEPARATOR "chains" PATH_SEPARATOR, m_options.bgfx_path()));
	path += name;

	bx::FileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		printf("Unable to open chain file %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	int32_t size(bx::getSize(&reader));

	char* data = new char[size + 1];
	bx::read(&reader, reinterpret_cast<void*>(data), size);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<kParseCommentsFlag>(data);

	delete [] data;

	if (document.HasParseError())
	{
		std::string error(GetParseError_En(document.GetParseError()));
		printf("Unable to parse chain %s. Errors returned:\n", path.c_str());
		printf("%s\n", error.c_str());
		return nullptr;
	}

	bgfx_chain* chain = chain_reader::read_from_value(document, name + ": ", *this, screen_index);

	if (chain == nullptr)
	{
		printf("Unable to load chain %s, falling back to no post processing\n", path.c_str());
		return nullptr;
	}

	return chain;
}

void chain_manager::parse_chain_selections(std::string chain_str)
{
	std::vector<std::string> chain_names = split_option_string(chain_str);

	if (chain_names.empty())
		chain_names.push_back("default");

	while (m_current_chain.size() != chain_names.size())
	{
		m_screen_chains.push_back(nullptr);
		m_chain_names.push_back("");
		m_current_chain.push_back(CHAIN_NONE);
	}

	for (size_t index = 0; index < chain_names.size(); index++)
	{
		size_t chain_index = 0;
		for (chain_index = 0; chain_index < m_available_chains.size(); chain_index++)
		{
			if (m_available_chains[chain_index].m_name == chain_names[index])
			{
				break;
			}
		}

		if (chain_index < m_available_chains.size())
		{
			m_current_chain[index] = chain_index;
			m_chain_names[index] = m_available_chains[chain_index].m_name;
		}
		else
		{
			m_current_chain[index] = CHAIN_NONE;
			m_chain_names[index] = "";
		}
	}
}

std::vector<std::string> chain_manager::split_option_string(std::string chain_str) const
{
	std::vector<std::string> chain_names;

	uint32_t length = chain_str.length();
	uint32_t win = 0;
	uint32_t last_start = 0;
	for (uint32_t i = 0; i < length + 1; i++)
	{
		if (i == length || chain_str[i] == ',' || chain_str[i] == ':')
		{
			if (win == m_window_index)
			{
				chain_names.push_back(chain_str.substr(last_start, i - last_start));
			}
			last_start = i + 1;
			if (chain_str[i] == ':')
			{
				win++;
			}
		}
	}

	return chain_names;
}

void chain_manager::load_chains()
{
	for (size_t chain = 0; chain < m_current_chain.size() && chain < m_screen_chains.size(); chain++)
	{
		if (m_current_chain[chain] != CHAIN_NONE)
		{
			chain_desc& desc = m_available_chains[m_current_chain[chain]];
			m_chain_names[chain] = desc.m_name;
			m_screen_chains[chain] = load_chain(desc.m_path + "/" + desc.m_name, uint32_t(chain));
		}
	}
}

void chain_manager::destroy_chains()
{
	for (size_t index = 0; index < m_screen_chains.size(); index++)
	{
		if (m_screen_chains[index] != nullptr)
		{
			delete m_screen_chains[index];
			m_screen_chains[index] = nullptr;
		}
	}
}

void chain_manager::reload_chains()
{
	destroy_chains();
	load_chains();
}

bgfx_chain* chain_manager::screen_chain(uint32_t screen)
{
	if (screen >= m_screen_chains.size())
	{
		return m_screen_chains[m_screen_chains.size() - 1];
	}
	else
	{
		return m_screen_chains[screen];
	}
}

void chain_manager::process_screen_quad(uint32_t view, uint32_t screen, screen_prim &prim, osd_window& window)
{
	const bool any_targets_rebuilt = m_targets.update_target_sizes(screen, prim.m_tex_width, prim.m_tex_height, TARGET_STYLE_GUEST);
	if (any_targets_rebuilt)
	{
		for (bgfx_chain* chain : m_screen_chains)
		{
			if (chain != nullptr)
			{
				chain->repopulate_targets();
			}
		}
	}

	bgfx_chain* chain = screen_chain(screen);
	chain->process(prim, view, screen, m_textures, window, bgfx_util::get_blend_state(PRIMFLAG_GET_BLENDMODE(prim.m_flags)));
	view += chain->applicable_passes();
}

uint32_t chain_manager::count_screens(render_primitive* prim)
{
	uint32_t screen_count = 0;
	while (prim != nullptr)
	{
		if (PRIMFLAG_GET_SCREENTEX(prim->flags))
		{
			if (screen_count < m_screen_prims.size())
			{
				m_screen_prims[screen_count] = prim;
			}
			else
			{
				m_screen_prims.push_back(prim);
			}
			screen_count++;
		}
		prim = prim->next();
	}

	if (screen_count > 0)
	{
		update_screen_count(screen_count);
		m_targets.update_screen_count(screen_count);
	}

	if (screen_count < m_screen_prims.size())
	{
		m_screen_prims.resize(screen_count);
	}

	return screen_count;
}

void chain_manager::update_screen_count(uint32_t screen_count)
{
	if (screen_count != m_screen_count)
	{
		m_slider_notifier.set_sliders_dirty();
		m_screen_count = screen_count;

		// Ensure we have one screen chain entry per screen
		while (m_screen_chains.size() < m_screen_count)
		{
			m_screen_chains.push_back(nullptr);
			m_chain_names.push_back("");
			m_current_chain.push_back(CHAIN_NONE);
		}

		// Ensure we have a screen chain selection slider per screen
		while (m_selection_sliders.size() < m_screen_count)
		{
			create_selection_slider(m_selection_sliders.size());
		}

		load_chains();
	}
}

int32_t chain_manager::slider_changed(running_machine &machine, void *arg, int id, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
	{
		m_current_chain[id] = newval;

		std::vector<std::vector<float>> settings = slider_settings();
		reload_chains();
		restore_slider_settings(id, settings);

		m_slider_notifier.set_sliders_dirty();
	}

	if (str != nullptr)
	{
		*str = string_format("%s", m_available_chains[m_current_chain[id]].m_name.c_str());
	}

	return m_current_chain[id];
}

void chain_manager::create_selection_slider(uint32_t screen_index)
{
	if (screen_index < m_selection_sliders.size())
	{
		return;
	}

	std::unique_ptr<slider_state> state = make_unique_clear<slider_state>();

	state->minval = 0;
	state->defval = m_current_chain[screen_index];
	state->maxval = m_available_chains.size() - 1;
	state->incval = 1;

	using namespace std::placeholders;
	state->update = std::bind(&chain_manager::slider_changed, this, _1, _2, _3, _4, _5);
	state->arg = this;
	state->id = screen_index;
	state->description = "Window " + std::to_string(m_window_index) + ", Screen " + std::to_string(screen_index) + " Effect:";

	ui::menu_item item;
	item.text = state->description;
	item.subtext = "";
	item.flags = 0;
	item.ref = state.get();
	item.type = ui::menu_item_type::SLIDER;
	m_selection_sliders.push_back(item);
	m_core_sliders.push_back(std::move(state));
}

uint32_t chain_manager::update_screen_textures(uint32_t view, render_primitive *starting_prim, osd_window& window)
{
	if (!count_screens(starting_prim))
		return 0;

	for (int screen = 0; screen < m_screen_prims.size(); screen++)
	{
		screen_prim &prim = m_screen_prims[screen];
		uint16_t tex_width(prim.m_tex_width);
		uint16_t tex_height(prim.m_tex_height);

		bgfx_texture* texture = screen < m_screen_textures.size() ? m_screen_textures[screen] : nullptr;
		bgfx_texture* palette = screen < m_screen_palettes.size() ? m_screen_palettes[screen] : nullptr;

		const uint32_t src_format = (prim.m_flags & PRIMFLAG_TEXFORMAT_MASK) >> PRIMFLAG_TEXFORMAT_SHIFT;
		const bool needs_conversion = m_converters[src_format] != nullptr;
		std::string screen_index = std::to_string(screen);
		std::string source_name = "source" + screen_index;
		std::string screen_name = "screen" + screen_index;
		std::string palette_name = "palette" + screen_index;
		std::string full_name = needs_conversion ? source_name : screen_name;
		if (texture && (texture->width() != tex_width || texture->height() != tex_height))
		{
			m_textures.add_provider(full_name, nullptr);
			delete texture;
			texture = nullptr;

			if (palette)
			{
				m_textures.add_provider(palette_name, nullptr);
				delete palette;
				palette = nullptr;
			}
		}

		bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::RGBA8;
		uint16_t pitch = tex_width;
		const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, prim.m_flags & PRIMFLAG_TEXFORMAT_MASK,
			tex_width, tex_height, prim.m_rowpixels, prim.m_prim->texture.palette, prim.m_prim->texture.base, &pitch);

		if (texture == nullptr)
		{
			bgfx_texture *texture = new bgfx_texture(full_name, dst_format, tex_width, tex_height, mem, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, pitch);
			m_textures.add_provider(full_name, texture);

			if (prim.m_prim->texture.palette)
			{
				uint16_t palette_width = (uint16_t)std::min(prim.m_palette_length, 256U);
				uint16_t palette_height = (uint16_t)std::max((prim.m_palette_length + 255) / 256, 1U);
				m_palette_temp.resize(palette_width * palette_height * 4);
				memcpy(&m_palette_temp[0], prim.m_prim->texture.palette, prim.m_palette_length * 4);
				const bgfx::Memory *palmem = bgfx::copy(&m_palette_temp[0], palette_width * palette_height * 4);
				palette = new bgfx_texture(palette_name, bgfx::TextureFormat::BGRA8, palette_width, palette_height, palmem, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, palette_width * 4);
				m_textures.add_provider(palette_name, palette);
			}

			if (screen >= m_screen_textures.size())
			{
				m_screen_textures.push_back(texture);
				if (palette)
				{
					m_screen_palettes.push_back(palette);
				}
			}
			else
			{
				m_screen_textures[screen] = texture;
				if (palette)
				{
					m_screen_palettes[screen] = palette;
				}
			}
		}
		else
		{
			texture->update(mem, pitch);

			if (prim.m_prim->texture.palette)
			{
				m_palette_temp.resize(palette->width() * palette->height() * 4);
				memcpy(&m_palette_temp[0], prim.m_prim->texture.palette, prim.m_palette_length * 4);
				const bgfx::Memory *palmem = bgfx::copy(&m_palette_temp[0], palette->width() * palette->height() * 4);
				palette->update(palmem);
			}
		}

		bgfx_chain* chain = screen_chain(screen);
		if (chain && needs_conversion && !chain->has_converter())
		{
			chain->prepend_converter(m_converters[src_format], *this);
		}
	}

	return m_screen_prims.size();
}

uint32_t chain_manager::process_screen_chains(uint32_t view, osd_window& window)
{
	// Process each screen as necessary
	uint32_t used_views = 0;
	uint32_t screen_index = 0;
	for (screen_prim &prim : m_screen_prims)
	{
		if (m_current_chain[screen_index] == CHAIN_NONE || screen_chain(screen_index) == nullptr)
		{
			screen_index++;
			continue;
		}

		uint16_t screen_width = prim.m_screen_width;
		uint16_t screen_height = prim.m_screen_height;
		if (window.swap_xy())
		{
			std::swap(screen_width, screen_height);
		}

		const bool any_targets_rebuilt = m_targets.update_target_sizes(screen_index, screen_width, screen_height, TARGET_STYLE_NATIVE);
		if (any_targets_rebuilt)
		{
			for (bgfx_chain* chain : m_screen_chains)
			{
				if (chain != nullptr)
				{
					chain->repopulate_targets();
				}
			}
		}

		process_screen_quad(view + used_views, screen_index, prim, window);
		used_views += screen_chain(screen_index)->applicable_passes();

		screen_index++;
	}

	bgfx::setViewFrameBuffer(view + used_views, BGFX_INVALID_HANDLE);

	return used_views;
}

bool chain_manager::has_applicable_chain(uint32_t screen)
{
	return screen < m_screen_count && m_current_chain[screen] != CHAIN_NONE && m_screen_chains[screen] != nullptr;
}

bool chain_manager::needs_sliders()
{
	return m_screen_count > 0 && m_available_chains.size() > 1;
}

void chain_manager::restore_slider_settings(int32_t id, std::vector<std::vector<float>>& settings)
{
	if (!needs_sliders())
	{
		return;
	}

	for (size_t index = 0; index < m_screen_chains.size() && index < m_screen_count; index++)
	{
		if (index == id)
		{
			continue;
		}

		bgfx_chain* chain = m_screen_chains[index];
		if (chain == nullptr)
		{
			continue;
		}

		std::vector<bgfx_slider*> chain_sliders = chain->sliders();
		for (size_t slider = 0; slider < chain_sliders.size(); slider++)
		{
			chain_sliders[slider]->import(settings[index][slider]);
		}
	}
}

std::vector<std::vector<float>> chain_manager::slider_settings()
{
	std::vector<std::vector<float>> curr;

	if (!needs_sliders())
	{
		return curr;
	}

	for (size_t index = 0; index < m_screen_chains.size() && index < m_screen_count; index++)
	{
		curr.push_back(std::vector<float>());

		bgfx_chain* chain = m_screen_chains[index];
		if (chain == nullptr)
		{
			continue;
		}

		std::vector<bgfx_slider*> chain_sliders = chain->sliders();
		for (bgfx_slider* slider : chain_sliders)
		{
			curr[index].push_back(slider->value());
		}
	}

	return curr;
}

std::vector<ui::menu_item> chain_manager::get_slider_list()
{
	std::vector<ui::menu_item> sliders;

	if (!needs_sliders())
	{
		return sliders;
	}

	for (size_t index = 0; index < m_screen_chains.size() && index < m_screen_count; index++)
	{
		bgfx_chain* chain = m_screen_chains[index];
		sliders.push_back(m_selection_sliders[index]);

		if (chain == nullptr)
		{
			continue;
		}

		std::vector<bgfx_chain_entry*> chain_entries = chain->entries();
		for (bgfx_chain_entry* entry : chain_entries)
		{
			std::vector<bgfx_input_pair*> entry_inputs = entry->inputs();
			for (bgfx_input_pair* input : entry_inputs)
			{
				std::vector<ui::menu_item> input_sliders = input->get_slider_list();
				for (ui::menu_item slider : input_sliders)
				{
					sliders.push_back(slider);
				}
			}
		}

		std::vector<bgfx_slider*> chain_sliders = chain->sliders();
		for (bgfx_slider* slider : chain_sliders)
		{
			slider_state* core_slider = slider->core_slider();

			ui::menu_item item;
			item.text = core_slider->description;
			item.subtext = "";
			item.flags = 0;
			item.ref = core_slider;
			item.type = ui::menu_item_type::SLIDER;
			m_selection_sliders.push_back(item);

			sliders.push_back(item);
		}

		if (chain_sliders.size() > 0)
		{
			ui::menu_item item;
			item.text = MENU_SEPARATOR_ITEM;
			item.subtext = "";
			item.flags = 0;
			item.ref = nullptr;
			item.type = ui::menu_item_type::SEPARATOR;

			sliders.push_back(item);
		}
	}

	return sliders;
}
