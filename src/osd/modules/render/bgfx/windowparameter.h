// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  windowparameter.h - Static window-index shader param
//
//============================================================

#ifndef MAME_RENDER_BGFX_WINDOWPARAMETER_H
#define MAME_RENDER_BGFX_WINDOWPARAMETER_H

#pragma once

#include "parameter.h"

#include <string>
#include <utility>

class bgfx_window_parameter : public bgfx_parameter
{
public:
	bgfx_window_parameter(std::string &&name, parameter_type type, uint32_t index) : bgfx_parameter(std::move(name), type), m_index(index) { }

	virtual float value() override { return float(m_index); }
	virtual void tick(double delta) override { }

private:
	uint32_t    m_index;
};

#endif // MAME_RENDER_BGFX_WINDOWPARAMETER_H
