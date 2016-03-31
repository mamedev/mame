// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  slider.h - BGFX shader parameter slider
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SLIDER__
#define __DRAWBGFX_SLIDER__

#include <bgfx/bgfx.h>

#include <string>
#include <vector>

#include "emu.h"
#include "ui/ui.h"

class bgfx_slider
{
public:
	enum slider_type
	{
		SLIDER_INT_ENUM,
		SLIDER_FLOAT,
		SLIDER_INT,
		SLIDER_COLOR,
		SLIDER_VEC2
	};

	enum screen_type
	{
		SLIDER_SCREEN_TYPE_NONE = 0,
		SLIDER_SCREEN_TYPE_RASTER = 1,
		SLIDER_SCREEN_TYPE_VECTOR = 2,
		SLIDER_SCREEN_TYPE_VECTOR_OR_RASTER = SLIDER_SCREEN_TYPE_VECTOR | SLIDER_SCREEN_TYPE_RASTER,
		SLIDER_SCREEN_TYPE_LCD = 4,
		SLIDER_SCREEN_TYPE_LCD_OR_RASTER = SLIDER_SCREEN_TYPE_LCD | SLIDER_SCREEN_TYPE_RASTER,
		SLIDER_SCREEN_TYPE_LCD_OR_VECTOR = SLIDER_SCREEN_TYPE_LCD | SLIDER_SCREEN_TYPE_VECTOR,
		SLIDER_SCREEN_TYPE_ANY = SLIDER_SCREEN_TYPE_RASTER | SLIDER_SCREEN_TYPE_VECTOR | SLIDER_SCREEN_TYPE_LCD
	};

	bgfx_slider(running_machine& machine, std::string name, int32_t min, int32_t def, int32_t max, int32_t step, slider_type type, screen_type screen, float scale, std::string format, std::string description, std::vector<std::string>& strings);
	~bgfx_slider();

	int32_t update(std::string *str, int32_t newval);

	// Getters
	std::string name() const { return m_name; }
	slider_type type() const { return m_type; }
	float value() const { return m_value; }
	float uniform_value() const { return float(m_value); }
	slider_state* core_slider() const { return m_slider_state; }
	size_t size() const { return get_size_for_type(m_type); }

	static size_t get_size_for_type(slider_type type);

protected:
	slider_state* create_core_slider(running_machine &machine);
	int32_t as_int() const { return int32_t(floor(m_value + 0.5f)); }

	std::string     m_name;
	int32_t         m_min;
	int32_t         m_default;
	int32_t         m_max;
	int32_t         m_step;
	slider_type     m_type;
	screen_type     m_screen_type;
	float           m_scale;
	std::string     m_format;
	std::string     m_description;
	std::vector<std::string> m_strings;
	float           m_value;
	slider_state*   m_slider_state;
};

#endif // __DRAWBGFX_SLIDER__
