// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  frameparameter.h - Frame-based dynamic shader param
//
//============================================================

#ifndef MAME_RENDER_BGFX_FRAMEPARAMETER_H
#define MAME_RENDER_BGFX_FRAMEPARAMETER_H

#pragma once

#include "parameter.h"

#include <cstdint>
#include <string>

class bgfx_frame_parameter : public bgfx_parameter
{
public:
	bgfx_frame_parameter(std::string &&name, parameter_type type, uint32_t period);

	virtual float value() override;
	virtual void tick(double delta) override;

private:
	uint32_t    m_current_frame;
	uint32_t    m_period;
};

#endif // MAME_RENDER_BGFX_FRAMEPARAMETER_H
