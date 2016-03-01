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
	for (std::pair<std::string, bgfx_texture*> texture : m_textures)
	{
		if (texture.second != nullptr && !(texture.second)->is_target())
		{
			delete texture.second;
		}
	}
	m_textures.clear();
}

void texture_manager::add_texture(std::string name, bgfx_texture* texture)
{
	m_textures[name] = texture;
}

bgfx_texture* texture_manager::create_texture(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, void* data, uint32_t flags)
{
	bgfx_texture* texture = new bgfx_texture(name, format, width, height, flags, data);
	m_textures[name] = texture;
	return texture;
}

bgfx_texture* texture_manager::texture(std::string name)
{
	std::map<std::string, bgfx_texture*>::iterator iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		return iter->second;
	}

	return nullptr;
}
