// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramuniform.h - BGFX shader chain parametric uniform
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_PARAM_UNIFORM__
#define __DRAWBGFX_PARAM_UNIFORM__

#include "entryuniform.h"

class bgfx_parameter;

class bgfx_param_uniform : public bgfx_entry_uniform
{
public:
	bgfx_param_uniform(bgfx_uniform* uniform, bgfx_parameter* param);

	virtual void bind() override;

private:
	bgfx_parameter* m_param;
};

#endif // __DRAWBGFX_PARAM_UNIFORM__
