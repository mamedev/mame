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

#include <vector>
#include <map>
#include <string>

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"

class running_machine;
class osd_window;
class slider_dirty_notifier;
class render_primitive;

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

    uint32_t handle_screen_chains(uint32_t view, render_primitive *starting_prim, osd_window& window);
    int32_t chain_changed(int32_t index, std::string *str, int32_t newval);

    // Getters
	running_machine& machine() { return m_machine; }
	osd_options& options() { return m_options; }
	texture_manager& textures() { return m_textures; }
	target_manager& targets() { return m_targets; }
	effect_manager& effects() { return m_effects; }
	slider_dirty_notifier& slider_notifier() { return m_slider_notifier; }
	uint32_t window_index() { return m_window_index; }
	uint32_t screen_count() { return m_screen_count; }
    bgfx_chain* screen_chain(uint32_t screen);
    bgfx_chain* load_chain(std::string name, uint32_t screen_index);
    bool has_applicable_chain(uint32_t screen);
    std::vector<ui_menu_item> get_slider_list();
    std::vector<std::vector<float>> slider_settings();

    // Setters
    void restore_slider_settings(int32_t id, std::vector<std::vector<float>>& settings);

private:
    void load_chains();
    void destroy_chains();
    void reload_chains();

	void refresh_available_chains();
	void destroy_unloaded_chains();
    void find_available_chains(std::string root, std::string path);
    void parse_chain_selections(std::string chain_str);
    std::vector<std::string> split_option_string(std::string chain_str) const;

	void update_screen_count(uint32_t screen_count);
    void create_selection_slider(uint32_t screen_index);
    bool needs_sliders();

    std::vector<render_primitive*> count_screens(render_primitive* prim);
    void process_screen_quad(uint32_t view, uint32_t screen, render_primitive* prim, osd_window &window);

    running_machine&            m_machine;
	osd_options&                m_options;
	texture_manager&            m_textures;
	target_manager&             m_targets;
	effect_manager&             m_effects;
    uint32_t                    m_window_index;
    slider_dirty_notifier&      m_slider_notifier;
    uint32_t					m_screen_count;
    std::vector<chain_desc>		m_available_chains;
    std::vector<bgfx_chain*>    m_screen_chains;
    std::vector<std::string>	m_chain_names;
    std::vector<ui_menu_item>   m_selection_sliders;
    std::vector<int32_t>        m_current_chain;

    static const uint32_t       CHAIN_NONE;
};

#endif // __DRAWBGFX_CHAIN_MANAGER__
