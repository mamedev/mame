// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainmanager.h - BGFX shader chain manager
//
//  Maintains a string-to-entry lookup of BGFX shader
//  effect chains, defined by chain.h and read by chainreader.h
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_MANAGER__
#define __DRAWBGFX_CHAIN_MANAGER__

#include <map>
#include <string>

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"

class running_machine;

class bgfx_chain;

class chain_manager {
public:
	chain_manager(texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t width, uint32_t height)
		: m_textures(textures)
		, m_targets(targets)
		, m_effects(effects)
		, m_width(width)
		, m_height(height)
	{
	}
	~chain_manager();

	// Getters
	bgfx_chain* chain(std::string name, running_machine& machine);

private:
	bgfx_chain* load_chain(std::string name, running_machine& machine);

	texture_manager&					m_textures;
	target_manager&						m_targets;
	effect_manager&                     m_effects;
	uint32_t							m_width;
	uint32_t							m_height;
	std::map<std::string, bgfx_chain*>	m_chains;
};

#endif // __DRAWBGFX_CHAIN_MANAGER__
