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
#include <map>

#include "uniform.h"

class bgfx_slider
{
public:
	enum slider_type
	{
		SLIDER_BOOL,
		SLIDER_FLOAT,
		SLIDER_INT,
		SLIDER_COLOR,
		SLIDER_VEC2
	};

	bgfx_slider(slider_type type, std::string name, std::string description, void *defaults, void *min, void *max);
	~bgfx_slider();

	static size_t size(slider_type type);
	static size_t storage_size(slider_type type);
protected:

	slider_type m_type;
	std::string m_name;
	std::string m_description;
	char*   m_data;
	char*   m_min;
	char*   m_max;
};

#endif // __DRAWBGFX_SLIDER__
