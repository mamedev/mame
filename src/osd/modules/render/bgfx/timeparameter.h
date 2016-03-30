// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  timeparameter.h - Time-based dynamic shader param
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TIME_PARAMETER__
#define __DRAWBGFX_TIME_PARAMETER__

#include <bgfx/bgfx.h>

#include <string>

#include "parameter.h"

class bgfx_time_parameter : public bgfx_parameter
{
public:
	bgfx_time_parameter(std::string name, parameter_type type, double limit);
	virtual ~bgfx_time_parameter() { }

	virtual float value() override;
	virtual void tick(double delta) override;

private:
	double m_current_time;
	double m_limit;
};

#endif // __DRAWBGFX_TIME_PARAMETER__
