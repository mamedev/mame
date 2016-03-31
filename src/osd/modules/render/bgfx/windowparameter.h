// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  windowparameter.h - Static window-index shader param
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_WINDOW_PARAMETER__
#define __DRAWBGFX_WINDOW_PARAMETER__

#include <bgfx/bgfx.h>

#include <string>

#include "parameter.h"

class bgfx_window_parameter : public bgfx_parameter
{
public:
	bgfx_window_parameter(std::string name, parameter_type type, uint32_t index) : bgfx_parameter(name, type), m_index(index) { }
	virtual ~bgfx_window_parameter() { }

	virtual float value() override { return float(m_index); }
	virtual void tick(double delta) override { };

private:
	uint32_t    m_index;
};

#endif // __DRAWBGFX_WINDOW_PARAMETER__
