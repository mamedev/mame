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
#include <string>

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"

class running_machine;
class osd_window;

class bgfx_chain;

class chain_manager {
public:
	chain_manager(running_machine& machine, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t window_index);
	~chain_manager();

    uint32_t handle_screen_chains(uint32_t view, render_primitive *starting_prim, osd_window& window);
    
    // Getters
    bgfx_chain* screen_chain(uint32_t screen);
    bgfx_chain* load_chain(std::string name, running_machine& machine, uint32_t window_index, uint32_t screen_index);
    bool has_applicable_pass(uint32_t screen);
    slider_state* get_slider_list();

private:
    void load_screen_chains(std::string chain_str);
    std::vector<std::vector<std::string>> split_option_string(std::string chain_str) const;
    void load_chains(std::vector<std::vector<std::string>>& chains);
    
    std::vector<render_primitive*> count_screens(render_primitive* prim);
    void process_screen_quad(uint32_t view, uint32_t screen, render_primitive* prim, osd_window &window);

    running_machine&                        m_machine;
	osd_options&                            m_options;
	texture_manager&                        m_textures;
	target_manager&                         m_targets;
	effect_manager&                         m_effects;
    uint32_t                                m_window_index;
    std::vector<std::vector<bgfx_chain*>>   m_screen_chains;
};

#endif // __DRAWBGFX_CHAIN_MANAGER__
