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
#include "../frontend/mame/ui/slider.h"

class bgfx_slider : public slider_changed_notifier
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

	bgfx_slider(running_machine& machine, std::string name, float min, float def, float max, float step, slider_type type, screen_type screen, std::string format, std::string description, std::vector<std::string>& strings);
	virtual ~bgfx_slider();

	int32_t update(std::string *str, int32_t newval);

	// Getters
	std::string name() const { return m_name; }
	slider_type type() const { return m_type; }
	float value() const { return m_value; }
	float uniform_value() const { return float(m_value); }
	slider_state* core_slider() const { return m_slider_state; }
	size_t size() const { return get_size_for_type(m_type); }
	static size_t get_size_for_type(slider_type type);

	// Setters
	void import(float val);

protected:
	virtual INT32 slider_changed(running_machine &machine, void *arg, int /*id*/, std::string *str, INT32 newval) override;
	slider_state* create_core_slider(running_machine &machine);
	int32_t as_int() const { return int32_t(floor(m_value / m_step + 0.5f)); }

	std::string     m_name;
	float           m_min;
	float           m_default;
	float           m_max;
	float           m_step;
	slider_type     m_type;
	screen_type     m_screen_type;
	std::string     m_format;
	std::string     m_description;
	std::vector<std::string> m_strings;
	float           m_value;
	slider_state*   m_slider_state;
	running_machine&m_machine;
};

#endif // __DRAWBGFX_SLIDER__
