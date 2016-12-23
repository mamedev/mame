// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniform.h - Shader uniform abstraction for BGFX layer
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_UNIFORM__
#define __DRAWBGFX_UNIFORM__

#include <bgfx/bgfx.h>

#include <string>

class bgfx_uniform
{
public:
	bgfx_uniform(std::string name, bgfx::UniformType::Enum type);
	virtual ~bgfx_uniform();

	virtual void upload();

	// Getters
	std::string name() { return m_name; }
	bgfx::UniformType::Enum type() const { return m_type; }
	bgfx::UniformHandle handle() const { return m_handle; }

	// Setters
	bgfx_uniform* set(float* value);
	bgfx_uniform* set_int(int value);
	bgfx_uniform* set_mat3(float* value);
	bgfx_uniform* set_mat4(float* value);
	bgfx_uniform* set(void* data, size_t size);

	static size_t get_size_for_type(bgfx::UniformType::Enum type);

protected:
	bgfx::UniformHandle     m_handle;
	std::string             m_name;
	bgfx::UniformType::Enum m_type;
	uint8_t*                m_data;
	size_t                  m_data_size;
};

#endif // __DRAWBGFX_UNIFORM__
