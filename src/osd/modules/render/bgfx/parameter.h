// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  parameter.h - BGFX shader parameter
//
//  A value that represents some form of parametric
//  operation, which can be fed to the input of a BGFX
//  shader uniform.
//
//============================================================

#ifndef MAME_RENDER_BGFX_PARAMETER_H
#define MAME_RENDER_BGFX_PARAMETER_H

#pragma once

#include <string>
#include <utility>

class bgfx_parameter
{
public:
	enum parameter_type
	{
		PARAM_FRAME,
		PARAM_WINDOW,
		PARAM_TIME
	};

	bgfx_parameter(std::string &&name, parameter_type type) : m_name(std::move(name)), m_type(type) { }
	virtual ~bgfx_parameter() = default;

	virtual void tick(double delta) = 0;

	// Getters
	virtual float value() = 0;
	const std::string &name() const { return m_name; }

protected:
	std::string m_name;
	parameter_type m_type;
};

#endif // MAME_RENDER_BGFX_PARAMETER_H
