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

#include <bx/math.h>
#include <bx/readerwriter.h>
#include <bx/file.h>

#include "emu.h"

#include <bgfx/bgfx.h>

#include "shadermanager.h"

shader_manager::~shader_manager()
{
	for (std::pair<std::string, bgfx::ShaderHandle> shader : m_shaders)
	{
		bgfx::destroy(shader.second);
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
	std::string shader_path(m_options.bgfx_path());
	shader_path += PATH_SEPARATOR "shaders" PATH_SEPARATOR;
	switch (bgfx::getRendererType())
	{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D9:
			shader_path += "dx9";
			break;

		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			shader_path += "dx11";
			break;

		case bgfx::RendererType::Gnm:
			shader_path += "pssl";
			break;

		case bgfx::RendererType::Metal:
			shader_path += "metal";
			break;

		case bgfx::RendererType::OpenGL:
			shader_path += "glsl";
			break;

		case bgfx::RendererType::OpenGLES:
			shader_path += "essl";
			break;

		case bgfx::RendererType::Vulkan:
			shader_path += "spirv";
			break;

		default:
			fatalerror("Unknown BGFX renderer type %d", bgfx::getRendererType());
	}
	shader_path += PATH_SEPARATOR;
	osd_subst_env(shader_path, shader_path);

	const bgfx::Memory* mem = load_mem(shader_path + name + ".bin");
	if (mem != nullptr)
	{
		bgfx::ShaderHandle handle = bgfx::createShader(mem);

		m_shaders[name] = handle;

		return handle;
	}

	return BGFX_INVALID_HANDLE;
}

const bgfx::Memory* shader_manager::load_mem(std::string name)
{
	bx::FileReader reader;
	if (bx::open(&reader, name.c_str()))
	{
		uint32_t size(bx::getSize(&reader));
		const bgfx::Memory* mem = bgfx::alloc(size + 1);
		bx::read(&reader, mem->data, size);
		bx::close(&reader);

		mem->data[mem->size - 1] = '\0';
		return mem;
	}
	else
	{
		printf("Unable to load shader %s\n", name.c_str());
	}
	return nullptr;
}
