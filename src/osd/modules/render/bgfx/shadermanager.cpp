// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  shadermanager.cpp - BGFX shader manager
//
//  Maintains a mapping between strings and BGFX shaders,
//  either vertex or pixel/fragment.
//
//============================================================

#include "emu.h"

#include <bgfx/bgfxplatform.h>
#include <bgfx/bgfx.h>
#include <bx/fpumath.h>
#include <bx/readerwriter.h>
#include <bx/crtimpl.h>

#include "shadermanager.h"

shader_manager::~shader_manager()
{
	for (std::pair<std::string, bgfx::ShaderHandle> shader : m_shaders)
	{
		bgfx::destroyShader(shader.second);
	}
	m_shaders.clear();
}

bgfx::ShaderHandle shader_manager::shader(std::string name)
{
	std::map<std::string, bgfx::ShaderHandle>::iterator iter = m_shaders.find(name);
	if (iter != m_shaders.end())
	{
		return iter->second;
	}

	return load_shader(name);
}

bgfx::ShaderHandle shader_manager::load_shader(std::string name)
{
	std::string shader_path = "shaders/dx9/";
	switch (bgfx::getRendererType())
	{
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			shader_path = "shaders/dx11/";
			break;

		case bgfx::RendererType::OpenGL:
			shader_path = "shaders/glsl/";
			break;

		case bgfx::RendererType::Metal:
			shader_path = "shaders/metal/";
			break;

		case bgfx::RendererType::OpenGLES:
			shader_path = "shaders/gles/";
			break;

		default:
			break;
	}

	bgfx::ShaderHandle handle = bgfx::createShader(load_mem(shader_path + name + ".bin"));

	m_shaders[name] = handle;

	return handle;
}

const bgfx::Memory* shader_manager::load_mem(std::string name)
{
	bx::CrtFileReader reader;
	bx::open(&reader, name.c_str());

	uint32_t size = (uint32_t)bx::getSize(&reader);
	const bgfx::Memory* mem = bgfx::alloc(size + 1);
	bx::read(&reader, mem->data, size);
	bx::close(&reader);

	mem->data[mem->size - 1] = '\0';
	return mem;
}
