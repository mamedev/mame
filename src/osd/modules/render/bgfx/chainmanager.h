// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainmanager.h - BGFX shader chain manager
//
//  Provides loading for BGFX shader effect chains, defined
//  by chain.h and read by chainreader.h
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_MANAGER__
#define __DRAWBGFX_CHAIN_MANAGER__

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"

#include <map>
#include <string>
#include <vector>

class running_machine;
class osd_window;
struct slider_state;
class slider_dirty_notifier;
class render_primitive;

namespace ui { class menu_item; }

class bgfx_chain;
class bgfx_slider;

class chain_desc
{
public:
	chain_desc(std::string name, std::string path)
		: m_name(name)
		, m_path(path)
	{
	}

	const std::string m_name;
	const std::string m_path;
};

class chain_manager
{
public:
	chain_manager(running_machine& machine, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t window_index, slider_dirty_notifier& slider_notifier);
	~chain_manager();

	uint32_t update_screen_textures(uint32_t view, render_primitive *starting_prim, osd_window& window);
	uint32_t process_screen_chains(uint32_t view, osd_window& window);

	// Getters
	running_machine& machine() const { return m_machine; }
	osd_options& options() const { return m_options; }
	texture_manager& textures() const { return m_textures; }
	target_manager& targets() const { return m_targets; }
	effect_manager& effects() const { return m_effects; }
	slider_dirty_notifier& slider_notifier() const { return m_slider_notifier; }
	uint32_t window_index() const { return m_window_index; }
	uint32_t screen_count() const { return m_screen_count; }
	bgfx_chain* screen_chain(uint32_t screen);
	bgfx_chain* load_chain(std::string name, uint32_t screen_index);
	bool has_applicable_chain(uint32_t screen);
	std::vector<ui::menu_item> get_slider_list();
	std::vector<std::vector<float>> slider_settings();

	// Setters
	void restore_slider_settings(int32_t id, std::vector<std::vector<float>>& settings);

	class screen_prim
	{
	public:
		screen_prim() : m_prim(nullptr), m_screen_width(0), m_screen_height(0), m_quad_width(0), m_quad_height(0)
			, m_tex_width(0), m_tex_height(0), m_rowpixels(0), m_palette_length(0), m_flags(0)
		{
		}

		screen_prim(render_primitive *prim);

		render_primitive *m_prim;
		uint16_t m_screen_width;
		uint16_t m_screen_height;
		uint16_t m_quad_width;
		uint16_t m_quad_height;
		float m_tex_width;
		float m_tex_height;
		int m_rowpixels;
		uint32_t m_palette_length;
		uint32_t m_flags;
	};

private:
	void load_chains();
	void destroy_chains();
	void reload_chains();

	void init_texture_converters();

	void refresh_available_chains();
	void destroy_unloaded_chains();
	void find_available_chains(std::string root, std::string path);
	void parse_chain_selections(std::string chain_str);
	std::vector<std::string> split_option_string(std::string chain_str) const;

	void update_screen_count(uint32_t screen_count);

	int32_t slider_changed(int id, std::string *str, int32_t newval);
	void create_selection_slider(uint32_t screen_index);
	bool needs_sliders();

	uint32_t count_screens(render_primitive* prim);
	void process_screen_quad(uint32_t view, uint32_t screen, screen_prim &prim, osd_window& window);

	running_machine&            m_machine;
	osd_options&                m_options;
	texture_manager&            m_textures;
	target_manager&             m_targets;
	effect_manager&             m_effects;
	uint32_t                    m_window_index;
	slider_dirty_notifier&      m_slider_notifier;
	uint32_t                    m_screen_count;
	std::vector<chain_desc>     m_available_chains;
	std::vector<bgfx_chain*>    m_screen_chains;
	std::vector<std::string>    m_chain_names;
	std::vector<ui::menu_item>  m_selection_sliders;
	std::vector<std::unique_ptr<slider_state>> m_core_sliders;
	std::vector<int32_t>        m_current_chain;
	std::vector<bgfx_texture*>  m_screen_textures;
	std::vector<bgfx_texture*>  m_screen_palettes;
	std::vector<bgfx_effect*>   m_converters;
	bgfx_effect *               m_adjuster;
	std::vector<screen_prim>    m_screen_prims;
	std::vector<uint8_t>        m_palette_temp;

	static const uint32_t       CHAIN_NONE;
};

#endif // __DRAWBGFX_CHAIN_MANAGER__
