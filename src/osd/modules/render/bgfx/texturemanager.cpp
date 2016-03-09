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

#include "texturemanager.h"
#include "texture.h"

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
	m_textures[name] = provider;
}

bgfx_texture* texture_manager::create_texture(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, void* data, uint32_t flags)
{
	bgfx_texture* texture = new bgfx_texture(name, format, width, height, flags, data);
	m_textures[name] = texture;
	return texture;
}

bgfx::TextureHandle texture_manager::handle(std::string name)
{
	std::map<std::string, bgfx_texture_handle_provider*>::iterator iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		return (iter->second)->texture();
	}

	return BGFX_INVALID_HANDLE;
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
