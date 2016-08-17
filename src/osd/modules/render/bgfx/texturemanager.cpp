// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texturemanager.cpp - BGFX texture manager
//
//  Maintains a string-to-entry mapping for any registered
//  textures.
//
//============================================================

#include <bgfx/bgfx.h>

#include "emu.h"

#include "texturemanager.h"
#include "texture.h"
#include "rendutil.h"
#include "modules/render/copyutil.h"

texture_manager::~texture_manager()
{
	for (std::pair<std::string, bgfx_texture_handle_provider*> texture : m_textures)
	{
		if (texture.second != nullptr && !(texture.second)->is_target())
		{
			delete texture.second;
		}
	}
	m_textures.clear();
}

void texture_manager::add_provider(std::string name, bgfx_texture_handle_provider* provider)
{
	std::map<std::string, bgfx_texture_handle_provider*>::iterator iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		iter->second = provider;
	}
	else
	{
		m_textures[name] = provider;
	}
}

bgfx_texture* texture_manager::create_texture(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, void* data, uint32_t flags)
{
	bgfx_texture* texture = new bgfx_texture(name, format, width, height, flags, data);
	m_textures[name] = texture;
	return texture;
}

bgfx_texture* texture_manager::create_png_texture(std::string path, std::string file_name, std::string texture_name, uint32_t flags, uint32_t screen)
{
	bitmap_argb32 bitmap;
	emu_file file(path.c_str(), OPEN_FLAG_READ);
	render_load_png(bitmap, file, nullptr, file_name.c_str());

	if (bitmap.width() == 0 || bitmap.height() == 0)
	{
		printf("Unable to load PNG '%s' from path '%s'\n", path.c_str(), file_name.c_str());
		return nullptr;
	}

	uint8_t *data = new uint8_t[bitmap.width() * bitmap.height() * 4];
	UINT32 *data32 = reinterpret_cast<UINT32 *>(data);

	const uint32_t width = bitmap.width();
	const uint32_t height = bitmap.height();
	const uint32_t rowpixels = bitmap.rowpixels();
	UINT32* base = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(0));
	for (int y = 0; y < height; y++)
	{
		copy_util::copyline_argb32(data32 + y * width, base + y * rowpixels, width, nullptr);
	}

	if (screen >= 0)
	{
		texture_name += std::to_string(screen);
	}
	bgfx_texture* texture = create_texture(texture_name, bgfx::TextureFormat::RGBA8, width, height, data, flags);

	delete [] data;

	return texture;
}

bgfx::TextureHandle texture_manager::handle(std::string name)
{
	bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
	std::map<std::string, bgfx_texture_handle_provider*>::iterator iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		handle = (iter->second)->texture();
	}

	assert(handle.idx != bgfx::invalidHandle);
	return handle;
}

bgfx_texture_handle_provider* texture_manager::provider(std::string name)
{
	std::map<std::string, bgfx_texture_handle_provider*>::iterator iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		return iter->second;
	}

	return nullptr;
}

void texture_manager::remove_provider(std::string name, bool delete_provider)
{
	if (provider(name) != nullptr)
	{
		if (delete_provider)
		{
			delete m_textures[name];
		}
		m_textures[name] = nullptr;
	}
}
