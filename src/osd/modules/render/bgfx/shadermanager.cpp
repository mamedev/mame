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

#include "shadermanager.h"

#include "emucore.h"

#include "osdfile.h"
#include "modules/lib/osdobj_common.h"

#include <bx/file.h>
#include <bx/math.h>
#include <bx/readerwriter.h>


shader_manager::~shader_manager()
{
	for (std::pair<std::string, bgfx::ShaderHandle> shader : m_shaders)
	{
		bgfx::destroy(shader.second);
	}
	m_shaders.clear();
}

bgfx::ShaderHandle shader_manager::get_or_load_shader(const osd_options &options, const std::string &name)
{
	std::map<std::string, bgfx::ShaderHandle>::iterator iter = m_shaders.find(name);
	if (iter != m_shaders.end())
	{
		return iter->second;
	}

	bgfx::ShaderHandle handle = load_shader(options, name);
	if (handle.idx != bgfx::kInvalidHandle)
	{
		m_shaders[name] = handle;
	}

	return handle;
}

bgfx::ShaderHandle shader_manager::load_shader(const osd_options &options, const std::string &name)
{
	std::string shader_path = make_path_string(options, name);
	const bgfx::Memory* mem = load_mem(shader_path + name + ".bin");
	if (mem != nullptr)
	{
		return bgfx::createShader(mem);
	}

	return BGFX_INVALID_HANDLE;
}

bool shader_manager::is_shader_present(const osd_options &options, const std::string &name)
{
	std::string shader_path = make_path_string(options, name);
	std::string file_name = shader_path + name + ".bin";
	bx::FileReader reader;
	bx::ErrorAssert err;
	if (bx::open(&reader, file_name.c_str()))
	{
		uint32_t expected_size(bx::getSize(&reader));
		uint8_t *data = new uint8_t[expected_size];
		uint32_t read_size = (uint32_t)bx::read(&reader, data, expected_size, &err);
		delete [] data;
		bx::close(&reader);

		return expected_size == read_size;
	}

	return false;
}

std::string shader_manager::make_path_string(const osd_options &options, const std::string &name)
{
	std::string shader_path(options.bgfx_path());
	shader_path += PATH_SEPARATOR "shaders" PATH_SEPARATOR;

#if defined(SDLMAME_EMSCRIPTEN)
	// Hard-code renderer type to OpenGL ES for emscripten builds since the
	// bgfx::getRendererType() is called here before BGFX has been
	// initialized and therefore gives the wrong renderer type (Noop).
	shader_path += "essl" PATH_SEPARATOR;
	return shader_path;
#endif

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
	return shader_path;
}

const bgfx::Memory* shader_manager::load_mem(const std::string &name)
{
	bx::FileReader reader;
	if (bx::open(&reader, name.c_str()))
	{
		bx::ErrorAssert err;
		uint32_t size(bx::getSize(&reader));
		const bgfx::Memory* mem = bgfx::alloc(size + 1);
		bx::read(&reader, mem->data, size, &err);
		bx::close(&reader);

		mem->data[mem->size - 1] = '\0';
		return mem;
	}
	else
	{
		osd_printf_error("Unable to load shader %s\n", name);
	}
	return nullptr;
}
