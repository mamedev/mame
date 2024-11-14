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

#ifndef MAME_RENDER_BGFX_EFFECTMANAGER_H
#define MAME_RENDER_BGFX_EFFECTMANAGER_H

#pragma once

#include <map>
#include <memory>
#include <string>


class bgfx_effect;
class osd_options;
class shader_manager;

class effect_manager
{
public:
	effect_manager(shader_manager& shaders);
	~effect_manager();

	// Getters
	bgfx_effect* get_or_load_effect(const osd_options &options, const std::string &name);
	static bool validate_effect(const osd_options &options, const std::string &name);

private:
	bgfx_effect* load_effect(const osd_options &options, const std::string &name);

	shader_manager &m_shaders;
	std::map<std::string, std::unique_ptr<bgfx_effect> > m_effects;
};

#endif // MAME_RENDER_BGFX_EFFECTMANAGER_H
