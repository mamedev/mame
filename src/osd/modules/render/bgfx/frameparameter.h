// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  frameparameter.h - Frame-based dynamic shader param
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_FRAME_PARAMETER__
#define __DRAWBGFX_FRAME_PARAMETER__

#include <bgfx/bgfx.h>

#include <string>

#include "parameter.h"

class bgfx_frame_parameter : public bgfx_parameter
{
public:
	bgfx_frame_parameter(std::string name, parameter_type type, uint32_t period);
	virtual ~bgfx_frame_parameter() { }

	virtual float value() override;
	virtual void tick(double delta) override;

private:
	uint32_t    m_current_frame;
	uint32_t    m_period;
};

#endif // __DRAWBGFX_FRAME_PARAMETER__
