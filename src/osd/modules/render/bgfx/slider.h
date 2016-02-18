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
    bgfx_slider(slider_type type, std::string name, std::string description, void *defaults, void *min, void *max);
    ~bgfx_slider();

	enum slider_type
	{
		SLIDER_BOOL,
		SLIDER_FLOAT,
		SLIDER_INT,
		SLIDER_COLOR,
		SLIDER_VEC2
	};

    // Getters
    bgfx_uniform* uniform(std::string name);
    bgfx::ProgramHandle get_program() const { return m_program_handle; }

protected:
	std::string m_name;
	void *	m_data;
	void *	m_min;
	void *	m_max;
};

#endif // __DRAWBGFX_SLIDER__