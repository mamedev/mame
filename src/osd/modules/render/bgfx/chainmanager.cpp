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

#include "chainmanager.h"

#include <bx/readerwriter.h>
#include <bx/file.h>

#include "emucore.h"
#include "render.h"
#include "../frontend/mame/ui/slider.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdwindow.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "bgfxutil.h"

#include "chain.h"
#include "chainreader.h"
#include "slider.h"
#include "target.h"
#include "texture.h"

#include "sliderdirtynotifier.h"

#include "util/path.h"
#include "util/unicode.h"
#include "util/xmlfile.h"

#include "osdcore.h"
#include "osdfile.h"

#include <algorithm>
#include <locale>


using namespace rapidjson;

chain_manager::screen_prim::screen_prim(render_primitive *prim)
{
	m_prim = prim;
	m_screen_width = uint16_t(floorf(prim->get_full_quad_width() + 0.5f));
	m_screen_height = uint16_t(floorf(prim->get_full_quad_height() + 0.5f));
	m_quad_width = uint16_t(floorf(prim->get_quad_width() + 0.5f));
	m_quad_height = uint16_t(floorf(prim->get_quad_height() + 0.5f));
	m_tex_width = prim->texture.width;
	m_tex_height = prim->texture.height;
	m_rowpixels = prim->texture.rowpixels;
	m_palette_length = prim->texture.palette_length;
	m_flags = prim->flags;
}

chain_manager::chain_manager(
		running_machine& machine,
		const osd_options& options,
		texture_manager& textures,
		target_manager& targets,
		effect_manager& effects,
		uint32_t window_index,
		slider_dirty_notifier& slider_notifier,
		uint16_t user_prescale,
		uint16_t max_prescale_size)
	: m_machine(machine)
	, m_options(options)
	, m_textures(textures)
	, m_targets(targets)
	, m_effects(effects)
	, m_window_index(window_index)
	, m_user_prescale(user_prescale)
	, m_max_prescale_size(max_prescale_size)
	, m_slider_notifier(slider_notifier)
	, m_screen_count(0)
	, m_default_chain_index(-1)
{
	m_converters.clear();
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
	m_converters.push_back(m_effects.get_or_load_effect(m_options, "misc/texconv_palette16"));
	m_converters.push_back(m_effects.get_or_load_effect(m_options, "misc/texconv_rgb32"));
	m_converters.push_back(nullptr);
	m_converters.push_back(m_effects.get_or_load_effect(m_options, "misc/texconv_yuy16"));
	m_adjuster = m_effects.get_or_load_effect(m_options, "misc/bcg_adjust");
}

void chain_manager::get_default_chain_info(std::string &out_chain_name, int32_t &out_chain_index)
{
	if (m_default_chain_index == -1)
	{
		out_chain_index = CHAIN_NONE;
		out_chain_name = "";
		return;
	}

	out_chain_index = m_default_chain_index;
	out_chain_name = "default";
	return;
}

void chain_manager::refresh_available_chains()
{
	m_available_chains.clear();
	m_available_chains.emplace_back("none", "");

	find_available_chains(util::path_concat(m_options.bgfx_path(), "chains"), "");
	std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(std::locale());
	std::sort(
			m_available_chains.begin(),
			m_available_chains.end(),
			[&coll] (chain_desc const &x, chain_desc const &y) -> bool
			{
				if (x.m_name == "none")
					return y.m_name != "none";
				else if (y.m_name == "none")
					return false;
				else if (x.m_name == "default")
					return y.m_name != "default";
				else if (y.m_name == "default")
					return false;
				std::wstring const xstr = wstring_from_utf8(x.m_name);
				std::wstring const ystr = wstring_from_utf8(y.m_name);
				return coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size()) < 0;
			});

	if (m_default_chain_index == -1)
	{
		for (size_t i = 0; i < m_available_chains.size(); i++)
		{
			if (m_available_chains[i].m_name == "default")
			{
				m_default_chain_index = int32_t(i);
			}
		}
	}

	destroy_unloaded_chains();
}

void chain_manager::destroy_unloaded_chains()
{
	// O(shaders*available_chains), but we don't care because asset reloading happens rarely
	for (int i = 0; i < m_chain_names.size(); i++)
	{
		const std::string &name = m_chain_names[i];
		if (name.length() > 0)
		{
			for (chain_desc desc : m_available_chains)
			{
				if (desc.m_name == name)
				{
					delete m_screen_chains[i];
					m_screen_chains[i] = nullptr;
					get_default_chain_info(m_chain_names[i], m_current_chain[i]);
					break;
				}
			}
		}
	}
}

void chain_manager::find_available_chains(std::string_view root, std::string_view path)
{
	osd::directory::ptr directory = osd::directory::open(path.empty() ? std::string(root) : util::path_concat(root, path));
	if (directory)
	{
		for (const osd::directory::entry *entry = directory->read(); entry; entry = directory->read())
		{
			if (entry->type == osd::directory::entry::entry_type::FILE)
			{
				const std::string_view name(entry->name);
				const std::string_view extension(".json");

				// Does the name has at least one character in addition to ".json"?
				if (name.length() > extension.length())
				{
					size_t start = name.length() - extension.length();
					const std::string_view test_segment = name.substr(start, extension.length());

					// Does it end in .json?
					if (test_segment == extension)
					{
						m_available_chains.emplace_back(std::string(name.substr(0, start)), std::string(path));
					}
				}
			}
			else if (entry->type == osd::directory::entry::entry_type::DIR)
			{
				const std::string_view name = entry->name;
				if ((name != ".") && (name != ".."))
				{
					if (path.empty())
						find_available_chains(root, name);
					else
						find_available_chains(root, util::path_concat(path, name));
				}
			}
		}
	}
}

std::unique_ptr<bgfx_chain> chain_manager::load_chain(std::string name, uint32_t screen_index)
{
	if (name.length() < 5 || (name.compare(name.length() - 5, 5, ".json") != 0))
	{
		name += ".json";
	}
	const std::string path = util::path_concat(m_options.bgfx_path(), "chains", name);

	bx::FileReader reader;
	if (!bx::open(&reader, path.c_str()))
	{
		osd_printf_warning("Unable to open chain file %s, falling back to no post processing\n", path);
		return nullptr;
	}

	const int32_t size(bx::getSize(&reader));

	bx::ErrorAssert err;
	std::unique_ptr<char []> data(new (std::nothrow) char [size + 1]);
	if (!data)
	{
		osd_printf_error("Out of memory reading chain file %s\n", path);
		bx::close(&reader);
		return nullptr;
	}

	bx::read(&reader, reinterpret_cast<void*>(data.get()), size, &err);
	bx::close(&reader);
	data[size] = 0;

	Document document;
	document.Parse<kParseCommentsFlag>(data.get());
	data.reset();

	if (document.HasParseError())
	{
		std::string error(GetParseError_En(document.GetParseError()));
		osd_printf_warning("Unable to parse chain %s. Errors returned:\n%s\n", path, error);
		return nullptr;
	}

	std::unique_ptr<bgfx_chain> chain = chain_reader::read_from_value(document, name + ": ", *this, screen_index, m_user_prescale, m_max_prescale_size);

	if (!chain)
	{
		osd_printf_warning("Unable to load chain %s, falling back to no post processing\n", path);
		return nullptr;
	}

	return chain;
}

void chain_manager::parse_chain_selections(std::string_view chain_str)
{
	std::vector<std::string_view> chain_names = split_option_string(chain_str);

	if (chain_names.empty())
		chain_names.push_back("default");

	while (m_current_chain.size() < chain_names.size())
	{
		m_screen_chains.emplace_back(nullptr);
		m_chain_names.emplace_back();
		m_current_chain.push_back(CHAIN_NONE);
	}

	for (size_t index = 0; index < chain_names.size(); index++)
	{
		size_t chain_index = 0;
		for (chain_index = 0; chain_index < m_available_chains.size(); chain_index++)
		{
			if (m_available_chains[chain_index].m_name == chain_names[index])
				break;
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

std::vector<std::string_view> chain_manager::split_option_string(std::string_view chain_str) const
{
	std::vector<std::string_view> chain_names;

	const uint32_t length = chain_str.length();
	uint32_t win = 0;
	uint32_t last_start = 0;
	for (uint32_t i = 0; i <= length; i++)
	{
		if (i == length || (chain_str[i] == ',') || (chain_str[i] == ':'))
		{
			if ((win == 0) || (win == m_window_index))
			{
				// treat an empty string as equivalent to "default"
				if (i > last_start)
					chain_names.push_back(chain_str.substr(last_start, i - last_start));
				else
					chain_names.push_back("default");
			}

			last_start = i + 1;
			if ((i < length) && (chain_str[i] == ':'))
			{
				// no point walking the rest of the string if this was our window
				if (win == m_window_index)
					break;

				// don't use first for all if more than one window is specified
				chain_names.clear();
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
			m_screen_chains[chain] = load_chain(util::path_concat(desc.m_path, desc.m_name), uint32_t(chain)).release();
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
	const bool any_targets_rebuilt = m_targets.update_target_sizes(screen, prim.m_tex_width, prim.m_tex_height, TARGET_STYLE_GUEST, m_user_prescale, m_max_prescale_size);
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
	chain->process(prim, view, screen, m_textures, window);
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
		m_targets.update_screen_count(screen_count, m_user_prescale, m_max_prescale_size);
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

			int32_t chain_index = CHAIN_NONE;
			std::string chain_name;
			get_default_chain_info(chain_name, chain_index);
			m_chain_names.emplace_back(std::move(chain_name));
			m_current_chain.push_back(chain_index);
		}

		// Ensure we have a screen chain selection slider per screen
		while (m_selection_sliders.size() < m_screen_count)
		{
			create_selection_slider(m_selection_sliders.size());
		}

		load_chains();
	}
}

void chain_manager::set_current_chain(uint32_t screen, int32_t chain_index)
{
	if (chain_index < m_available_chains.size() && screen < m_current_chain.size() && screen < m_chain_names.size())
	{
		m_current_chain[screen] = chain_index;
		m_chain_names[screen] = m_available_chains[chain_index].m_name;
	}
}

int32_t chain_manager::slider_changed(int id, std::string *str, int32_t newval)
{
	if (newval != SLIDER_NOCHANGE)
	{
		set_current_chain(id, newval);

		std::vector<std::vector<float>> settings = slider_settings();
		reload_chains();
		restore_slider_settings(id, settings);

		m_slider_notifier.set_sliders_dirty();
	}

	if (str != nullptr)
	{
		*str = m_available_chains[m_current_chain[id]].m_name;
	}

	return m_current_chain[id];
}

void chain_manager::create_selection_slider(uint32_t screen_index)
{
	if (screen_index < m_selection_sliders.size())
	{
		return;
	}

	int32_t minval = 0;
	int32_t defval = m_current_chain[screen_index];
	int32_t maxval = m_available_chains.size() - 1;
	int32_t incval = 1;

	using namespace std::placeholders;
	auto state = std::make_unique<slider_state>(
			util::string_format("Window %1$u, Screen %2$u Effect", m_window_index, screen_index),
			minval, defval, maxval, incval,
			std::bind(&chain_manager::slider_changed, this, screen_index, _1, _2));

	ui::menu_item item(ui::menu_item_type::SLIDER, state.get());
	item.set_text(state->description);
	m_selection_sliders.emplace_back(item);
	m_core_sliders.emplace_back(std::move(state));
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
		const bool needs_adjust = prim.m_prim->texture.palette != nullptr && src_format != TEXFORMAT_PALETTE16;
		const std::string screen_index = std::to_string(screen);
		const std::string source_name = "source" + screen_index;
		const std::string screen_name = "screen" + screen_index;
		const std::string palette_name = "palette" + screen_index;
		const std::string &full_name = (needs_conversion || needs_adjust) ? source_name : screen_name;
		if (texture && (texture->width() != tex_width || texture->height() != tex_height))
		{
			m_textures.remove_provider(full_name);
			m_textures.remove_provider(palette_name);
			texture = nullptr;
			palette = nullptr;
		}

		bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::BGRA8;
		uint16_t pitch = prim.m_rowpixels;
		int width_div_factor = 1;
		int width_mul_factor = 1;
		const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, prim.m_flags & PRIMFLAG_TEXFORMAT_MASK,
			prim.m_rowpixels, prim.m_prim->texture.width_margin, tex_height, prim.m_prim->texture.palette, prim.m_prim->texture.base, pitch, width_div_factor, width_mul_factor);

		if (!texture)
		{
			uint32_t flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
			if (!PRIMFLAG_GET_TEXWRAP(prim.m_flags))
				flags |= BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
			auto newtex = std::make_unique<bgfx_texture>(full_name, dst_format, tex_width, prim.m_prim->texture.width_margin, tex_height, mem, flags, pitch, prim.m_rowpixels, width_div_factor, width_mul_factor);
			texture = newtex.get();
			m_textures.add_provider(full_name, std::move(newtex));

			if (prim.m_prim->texture.palette)
			{
				uint16_t palette_width = uint16_t(std::min(prim.m_palette_length, 256U));
				uint16_t palette_height = uint16_t(std::max((prim.m_palette_length + 255) / 256, 1U));
				m_palette_temp.resize(palette_width * palette_height * 4);
				memcpy(&m_palette_temp[0], prim.m_prim->texture.palette, prim.m_palette_length * 4);
				const bgfx::Memory *palmem = bgfx::copy(&m_palette_temp[0], palette_width * palette_height * 4);
				auto newpal = std::make_unique<bgfx_texture>(palette_name, bgfx::TextureFormat::BGRA8, palette_width, 0, palette_height, palmem, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, palette_width * 4);
				palette = newpal.get();
				m_textures.add_provider(palette_name, std::move(newpal));
			}

			while (screen >= m_screen_textures.size())
			{
				m_screen_textures.emplace_back(nullptr);
			}
			m_screen_textures[screen] = texture;

			while (screen >= m_screen_palettes.size())
			{
				m_screen_palettes.emplace_back(nullptr);
			}
			if (palette)
			{
				m_screen_palettes[screen] = palette;
			}
		}
		else
		{
			texture->update(mem, pitch, prim.m_prim->texture.width_margin);

			if (prim.m_prim->texture.palette)
			{
				uint16_t palette_width = uint16_t(std::min(prim.m_palette_length, 256U));
				uint16_t palette_height = uint16_t(std::max((prim.m_palette_length + 255) / 256, 1U));
				const uint32_t palette_size = palette_width * palette_height * 4;
				m_palette_temp.resize(palette_size);
				memcpy(&m_palette_temp[0], prim.m_prim->texture.palette, prim.m_palette_length * 4);
				const bgfx::Memory *palmem = bgfx::copy(&m_palette_temp[0], palette_size);

				if (palette)
				{
					palette->update(palmem);
				}
				else
				{
					auto newpal = std::make_unique<bgfx_texture>(palette_name, bgfx::TextureFormat::BGRA8, palette_width, 0, palette_height, palmem, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, palette_width * 4);
					palette = newpal.get();
					m_textures.add_provider(palette_name, std::move(newpal));
					while (screen >= m_screen_palettes.size())
					{
						m_screen_palettes.emplace_back(nullptr);
					}
					m_screen_palettes[screen] = palette;
				}
			}
		}

		const bool has_tint = (prim.m_prim->color.a != 1.0f) || (prim.m_prim->color.r != 1.0f) || (prim.m_prim->color.g != 1.0f) || (prim.m_prim->color.b != 1.0f);
		bgfx_chain* chain = screen_chain(screen);
		if (chain && needs_adjust && !chain->has_adjuster())
		{
			const bool apply_tint = !needs_conversion && has_tint;
			chain->insert_effect(chain->has_converter() ? 1 : 0, m_adjuster, apply_tint, "XXadjust", needs_conversion ? "screen" : "source", *this);
			chain->set_has_adjuster(true);
		}
		if (chain && needs_conversion && !chain->has_converter())
		{
			chain->insert_effect(0, m_converters[src_format], has_tint, "XXconvert", "source", *this);
			chain->set_has_converter(true);
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

		const bool any_targets_rebuilt = m_targets.update_target_sizes(screen_index, screen_width, screen_height, TARGET_STYLE_NATIVE, m_user_prescale, m_max_prescale_size);
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
	return (screen < m_screen_count) && (m_current_chain[screen] != CHAIN_NONE) && m_screen_chains[screen];
}

bool chain_manager::needs_sliders()
{
	return (m_screen_count > 0) && (m_available_chains.size() > 1);
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

		const std::vector<bgfx_slider*> &chain_sliders = chain->sliders();
		for (size_t slider = 0; slider < chain_sliders.size(); slider++)
		{
			chain_sliders[slider]->import(settings[index][slider]);
		}
	}
}

void chain_manager::load_config(util::xml::data_node const &windownode)
{
	// treat source INI files or more specific as higher priority than CFG
	// FIXME: leaky abstraction - this depends on a front-end implementation detail
	bool const persist = windownode.get_attribute_int("persist", 1) != 0;
	bool const default_chains = (OPTION_PRIORITY_NORMAL + 5) > m_options.get_entry(OSDOPTION_BGFX_SCREEN_CHAINS)->priority();
	bool const explicit_chains = !persist && !default_chains && *m_options.bgfx_screen_chains();

	// if chains weren't explicitly specified, restore the chains from the config file
	if (explicit_chains)
	{
		osd_printf_verbose(
				"BGFX: Ignoring chain selection from window %d configuration due to explicitly specified chains\n",
				m_window_index);
	}
	else
	{
		bool changed = false;
		util::xml::data_node const *screennode = windownode.get_child("screen");
		while (screennode)
		{
			auto const index = screennode->get_attribute_int("index", -1);
			if ((0 <= index) && (m_screen_count > index))
			{
				char const *const chainname = screennode->get_attribute_string("chain", nullptr);
				if (chainname)
				{
					auto const found = std::find_if(
							m_available_chains.begin(),
							m_available_chains.end(),
							[&chainname] (auto const &avail) { return avail.m_name == chainname; });
					if (m_available_chains.end() != found)
					{
						auto const chainnum = found - m_available_chains.begin();
						if (chainnum != m_current_chain[index])
						{
							m_current_chain[index] = chainnum;
							changed = true;
						}
					}
				}
			}

			screennode = screennode->get_next_sibling("screen");
		}

		if (changed)
			reload_chains();
	}

	// now apply slider settings for screens with chains matching config
	util::xml::data_node const *screennode = windownode.get_child("screen");
	while (screennode)
	{
		auto const index = screennode->get_attribute_int("index", -1);
		if ((0 <= index) && (m_screen_count > index) && (m_screen_chains.size() > index))
		{
			bgfx_chain *const chain = m_screen_chains[index];
			char const *const chainname = screennode->get_attribute_string("chain", nullptr);
			if (chain && chainname && (m_available_chains[m_current_chain[index]].m_name == chainname))
			{
				auto const &sliders = chain->sliders();

				util::xml::data_node const *slidernode = screennode->get_child("slider");
				while (slidernode)
				{
					char const *const slidername = slidernode->get_attribute_string("name", nullptr);
					if (slidername)
					{
						auto const found = std::find_if(
								sliders.begin(),
								sliders.end(),
								[&slidername] (auto const &slider) { return slider->name() == slidername; });
						if (sliders.end() != found)
						{
							bgfx_slider &slider = **found;
							switch (slider.type())
							{
							case bgfx_slider::SLIDER_INT_ENUM:
							case bgfx_slider::SLIDER_INT:
								{
									slider_state const &core = *slider.core_slider();
									int32_t const val = slidernode->get_attribute_int("value", core.defval);
									slider.update(nullptr, std::clamp(val, core.minval, core.maxval));
								}
								break;
							default:
								{
									float const val = slidernode->get_attribute_float("value", slider.default_value());
									slider.import(std::clamp(val, slider.min_value(), slider.max_value()));
								}
							}
						}
					}

					slidernode = slidernode->get_next_sibling("slider");
				}
			}
		}
		screennode = screennode->get_next_sibling("screen");
	}
}

void chain_manager::save_config(util::xml::data_node &parentnode)
{
	if (!needs_sliders())
		return;

	util::xml::data_node *const windownode = parentnode.add_child("window", nullptr);
	windownode->set_attribute_int("index", m_window_index);

	for (size_t index = 0; index < m_screen_chains.size() && index < m_screen_count; index++)
	{
		bgfx_chain *const chain = m_screen_chains[index];
		if (!chain)
			continue;

		util::xml::data_node *const screennode = windownode->add_child("screen", nullptr);
		screennode->set_attribute_int("index", index);
		screennode->set_attribute("chain", m_available_chains[m_current_chain[index]].m_name.c_str());

		for (bgfx_slider *slider : chain->sliders())
		{
			auto const val = slider->update(nullptr, SLIDER_NOCHANGE);
			if (val == slider->core_slider()->defval)
				continue;

			util::xml::data_node *const slidernode = screennode->add_child("slider", nullptr);
			slidernode->set_attribute("name", slider->name().c_str());
			switch (slider->type())
			{
			case bgfx_slider::SLIDER_INT_ENUM:
			case bgfx_slider::SLIDER_INT:
				slidernode->set_attribute_int("value", val);
				break;
			default:
				slidernode->set_attribute_float("value", slider->value());
			}
		}
	}

	if (!windownode->get_first_child())
		windownode->delete_node();
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

		const std::vector<bgfx_slider*> &chain_sliders = chain->sliders();
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

		const std::vector<bgfx_chain_entry*> &chain_entries = chain->entries();
		for (bgfx_chain_entry* entry : chain_entries)
		{
			const std::vector<bgfx_input_pair*> &entry_inputs = entry->inputs();
			for (bgfx_input_pair* input : entry_inputs)
			{
				std::vector<ui::menu_item> input_sliders = input->get_slider_list();
				for (ui::menu_item &slider : input_sliders)
				{
					sliders.emplace_back(slider);
				}
			}
		}

		const std::vector<bgfx_slider*> &chain_sliders = chain->sliders();
		for (bgfx_slider* slider : chain_sliders)
		{
			slider_state *const core_slider = slider->core_slider();

			ui::menu_item item(ui::menu_item_type::SLIDER, core_slider);
			item.set_text(core_slider->description);
			m_selection_sliders.emplace_back(item);

			sliders.emplace_back(std::move(item));
		}

		if (chain_sliders.size() > 0)
		{
			ui::menu_item item(ui::menu_item_type::SEPARATOR);
			item.set_text(MENU_SEPARATOR_ITEM);

			sliders.emplace_back(std::move(item));
		}
	}

	return sliders;
}
