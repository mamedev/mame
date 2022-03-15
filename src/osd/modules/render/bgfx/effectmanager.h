// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effectmanager.h - BGFX shader effect manager
//
//  Maintains a string-to-entry lookup of BGFX shader
//  effects, defined by effect.h and read by effectreader.h
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_EFFECT_MANAGER__
#define __DRAWBGFX_EFFECT_MANAGER__

#include <map>
#include <string>

#include <bgfx/bgfx.h>

#include "shadermanager.h"

class bgfx_effect;

class effect_manager {
public:
	effect_manager(osd_options& options, shader_manager& shaders) : m_options(options), m_shaders(shaders) { }
	~effect_manager();

	// Getters
	bgfx_effect* effect(std::string name);

private:
	bgfx_effect* load_effect(std::string name);

	osd_options&                        m_options;
	shader_manager&                     m_shaders;
	std::map<std::string, bgfx_effect*> m_effects;
};

#endif // __DRAWBGFX_EFFECT_MANAGER__
